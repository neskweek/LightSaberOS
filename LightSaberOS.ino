/*
 * LightSaberOS V1.3
 *
 * released on: 10 mar 2016
 * author: 		Sebastien CAPOU (neskweek@gmail.com) and Andras Kun (kun.andras@yahoo.de)
 * Source : 	https://github.com/neskweek/LightSaberOS
 * Description:	Operating System for Arduino based LightSaber
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-sa/4.0/.
 */
/***************************************************************************************************
 * DFPLAYER variables
 */
//#define OLD_DPFPLAYER_LIB
#ifdef OLD_DPFPLAYER_LIB
  #include <SoftwareSerial.h> // interestingly the DFPlayer lib refuses
  #include "DFPlayer_Mini_Mp3.h"
  //SoftwareSerial mp3player(DFPLAYER_TX, DFPLAYER_RX); // TX, RX
  SoftwareSerial mp3player(7, 8); // TX, RX
#else
  #include <DFPlayer.h>
  DFPlayer dfplayer;
#endif

#include <Arduino.h>
#include <I2Cdev.h>
#include <MPU6050_6Axis_MotionApps20.h>
#include <EEPROMex.h>
#include <OneButton.h>
#include <LinkedList.h>


#include "Buttons.h"
#include "Config.h"
#include "ConfigMenu.h"
#include "Light.h"
#include "SoundFont.h"

#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
#include <Wire.h>
#endif
#if defined NEOPIXEL
#include <WS2812.h>
#endif



SoundFont soundFont;
unsigned long sndSuppress = millis();
unsigned long sndSuppress2 = millis();
bool hum_playing=false; // variable to store whether hum is being played
#ifdef JUKEBOX
bool jukebox_play=false; // indicate whether a song is being played in JukeBox mode
uint8_t jb_track;  // sound file track number in the directory designated for music playback
#endif  




/***************************************************************************************************
 * Saber Finite State Machine Custom Type and State Variable
 */
enum SaberStateEnum {S_STANDBY, S_SABERON, S_CONFIG, S_SLEEP, S_JUKEBOX};
SaberStateEnum SaberState;
SaberStateEnum PrevSaberState;

enum ActionModeSubStatesEnum {AS_HUM, AS_IGNITION, AS_RETRACTION, AS_BLADELOCKUP, AS_BLASTERDEFLECTMOTION, AS_BLASTERDEFLECTPRESS, AS_CLASH, AS_SWING, AS_SPIN, AS_FORCE};
ActionModeSubStatesEnum ActionModeSubStates;

/***************************************************************************************************
 * Motion detection Variables
 */
MPU6050 mpu;
// MPU control/status vars
volatile bool mpuInterrupt = false; // indicates whether MPU interrupt pin has gone high
bool dmpReady = false;  // set true if DMP init was successful
uint8_t mpuIntStatus;   // holds actual interrupt status byte from MPU
uint8_t devStatus; // return status after each device operation (0 = success, !0 = error)
uint16_t packetSize;    // expected DMP packet size (default is 42 bytes)
uint8_t fifoBuffer[64]; // FIFO storage buffer
uint16_t mpuFifoCount;     // count of all bytes currently in FIFO
// orientation/motion vars
Quaternion curRotation;           // [w, x, y, z]         quaternion container
Quaternion prevRotation;           // [w, x, y, z]         quaternion container
static Quaternion prevOrientation;  // [w, x, y, z]         quaternion container
static Quaternion curOrientation; // [w, x, y, z]         quaternion container
VectorInt16 curAccel;
VectorInt16 prevAccel;
VectorInt16 curDeltAccel;
VectorInt16 prevDeltAccel;

/***************************************************************************************************
 * LED String variables
 */
#if defined LEDSTRINGS
uint8_t ledPins[] = { LEDSTRING1, LEDSTRING2, LEDSTRING3, LEDSTRING4,
LEDSTRING5, LEDSTRING6 };
uint8_t blasterPin;
#endif
#if defined LUXEON
uint8_t ledPins[] = {LED_RED, LED_GREEN, LED_BLUE};
uint8_t currentColor[4]; //0:Red 1:Green 2:Blue 3:ColorID
#endif
#if defined NEOPIXEL
uint8_t ledPins[] = {STRING1, STRING2, STRING3};
WS2812 pixels(NUMPIXELS);
cRGB color;
volatile bool isFlickering = false;
cRGB currentColor;
uint8_t blasterPixel;
#endif
# if defined ACCENT_LED
unsigned long lastAccent = millis();
#if defined SOFT_ACCENT
unsigned long lastAccentTick = micros();

struct softPWM {
	uint8_t dutyCycle; // in percent
	bool revertCycle;
	uint8_t state;
	uint16_t tick;
} pwmPin = { 100, false, LOW, 0 };
#endif
#endif
uint8_t blaster = 0;
//bool blasterBlocks = false;
uint8_t clash = 0;
//bool lockup = false;
uint8_t blink = 0;
uint8_t randomBlink = 0;
/***************************************************************************************************
 * Buttons variables
 */
OneButton mainButton(MAIN_BUTTON, true);
OneButton lockupButton(LOCKUP_BUTTON, true);
// replaced by Saber State Machine Variables
//bool actionMode = false; // Play with your saber
//bool configMode = false; // Configure your saber
//static bool ignition = false;
//static bool browsing = false;

/***************************************************************************************************
 * ConfigMode Variables
 */
int8_t modification = 0;
int16_t value = 0;
uint8_t menu = 0;
bool enterMenu = false;
bool changeMenu = false;
bool play = false;
unsigned int configAdress = 0;
volatile uint8_t portbhistory = 0xFF;     // default is high because the pull-up
#if defined LEDSTRINGS
struct StoreStruct {
	// This is for mere detection if they are our settings
	char version[5];
	// The settings
	uint8_t volume;     // 0 to 31
	uint8_t soundFont; // as many Sound font you have defined in Soundfont.h Max:253
} storage;
#endif
#if defined LUXEON
struct StoreStruct {
	// This is for mere detection if they are our settings
	char version[5];
	// The settings
	uint8_t volume;// 0 to 31
	uint8_t soundFont;// as many as Sound font you have defined in Soundfont.h Max:253
  struct Profile {
    uint8_t mainColor;  //colorID
    uint8_t clashColor;//colorID
  }sndProfile[SOUNDFONT_QUANTITY + 2];
}storage;
#endif

#if defined NEOPIXEL
struct StoreStruct {
	// This is for mere detection if they are our settings
	char version[5];
	// The settings
	uint8_t volume;// 0 to 31
	uint8_t soundFont;// as many as Sound font you have defined in Soundfont.h Max:253
  struct Profile {
    uint8_t mainColor;  //colorID
    uint8_t clashColor;//colorID
    uint8_t blasterboltColor;
  }sndProfile[SOUNDFONT_QUANTITY + 2];
}storage;

#endif

/***************************************************************************************************
 * Function Prototypes
 * The following prototypes are not correctly generated by Arduino IDE 1.6.5-r5 or previous
 */
inline void printQuaternion(Quaternion quaternion, long multiplier);
inline void printAcceleration(VectorInt16 aaWorld);

// ====================================================================================
// ===        	       	   			SETUP ROUTINE  	 	                			===
// ====================================================================================
void setup() {

#ifdef LS_SERIAL
	// join I2C bus (I2Cdev library doesn't do this automatically)
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
	Wire.begin();
	TWBR = 24; // 400kHz I2C clock (200kHz if CPU is 8MHz). Comment this line if having compilation difficulties with TWBR.
#elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
			Fastwire::setup(400, true);
#endif
	// Serial line for debug
	Serial.begin(115200);
#endif


	/***** LOAD CONFIG *****/
	// Get config from EEPROM if there is one
	// or initialise value with default ones set in StoreStruct
	EEPROM.setMemPool(MEMORYBASE, EEPROMSizeATmega328); //Set memorypool base to 32, assume Arduino Uno board
	configAdress = EEPROM.getAddress(sizeof(StoreStruct)); // Size of config object

	if (!loadConfig()) {
		for (uint8_t i = 0; i <= 2; i++)
			storage.version[i] = CONFIG_VERSION[i];
		storage.soundFont = SOUNDFONT;
		storage.volume = VOL;
#if defined LEDSTRINGS
#endif
#if defined LUXEON
    for (uint8_t i=2; i<SOUNDFONT_QUANTITY+2;i++){
      storage.sndProfile[i].mainColor=1;
      storage.sndProfile[i].clashColor=1;
    }
#endif
#if defined NEOPIXEL
    for (uint8_t i=2; i<SOUNDFONT_QUANTITY+2;i++){
      storage.sndProfile[i].mainColor=1;
      storage.sndProfile[i].clashColor=1;
      storage.sndProfile[i].blasterboltColor=1;
    }
#endif

#if defined LS_INFO
		Serial.println(F("DEFAULT VALUE"));
#endif
	}
#if defined LS_INFO
	else {
		Serial.println(F("EEPROM LOADED"));
	}
#endif
  soundFont.setID(storage.soundFont);

	/***** LOAD CONFIG *****/

	/***** MP6050 MOTION DETECTOR INITIALISATION  *****/

	// initialize device
#if defined LS_INFO
	Serial.println(F("Initializing I2C devices..."));
#endif
	mpu.initialize();

	// verify connection
#if defined LS_INFO
	Serial.println(F("Testing device connections..."));
	Serial.println(
			mpu.testConnection() ?
					F("MPU6050 connection successful") :
					F("MPU6050 connection failed"));

	// load and configure the DMP
	Serial.println(F("Initializing DMP..."));
#endif
	//devStatus = mpu.dmpInitialize();  // this command alone eats up 18% of program storage space!!!
  devStatus = mpu.dmpInitialize_light();  // this command alone eats up 18% of program storage space!!!

	/*
	 * Those offsets are specific to each MPU6050 device.
	 * they are found via calibration process.
	 * See this script http://www.i2cdevlib.com/forums/index.php?app=core&module=attach&section=attach&attach_id=27
	 */
#ifdef MPUCALOFFSETEEPROM
        // retreive MPU6050 calibrated offset values from EEPROM
        EEPROM.setMemPool(MEMORYBASEMPUCALIBOFFSET, EEPROMSizeATmega328);
        int addressInt=MEMORYBASEMPUCALIBOFFSET;
       mpu.setXAccelOffset(EEPROM.readInt(addressInt));
        #ifdef LS_INFO
          int16_t output;
          output = EEPROM.readInt(addressInt);
          Serial.print("address: ");Serial.println(addressInt);Serial.print("output: ");Serial.println(output);Serial.println("");
        #endif
         addressInt = addressInt + 2; //EEPROM.getAddress(sizeof(int));        
       mpu.setYAccelOffset(EEPROM.readInt(addressInt));
        #ifdef LS_INFO
          output = EEPROM.readInt(addressInt);
          Serial.print("address: ");Serial.println(addressInt);Serial.print("output: ");Serial.println(output);Serial.println("");
        #endif
         addressInt = addressInt + 2; //EEPROM.getAddress(sizeof(int));
       mpu.setZAccelOffset(EEPROM.readInt(addressInt));
        #ifdef LS_INFO
          output = EEPROM.readInt(addressInt);
          Serial.print("address: ");Serial.println(addressInt);Serial.print("output: ");Serial.println(output);Serial.println("");
        #endif
         addressInt = addressInt + 2; //EEPROM.getAddress(sizeof(int));
       mpu.setXGyroOffset(EEPROM.readInt(addressInt));
        #ifdef LS_INFO
          output = EEPROM.readInt(addressInt);
          Serial.print("address: ");Serial.println(addressInt);Serial.print("output: ");Serial.println(output);Serial.println("");
        #endif
        addressInt = addressInt + 2; //EEPROM.getAddress(sizeof(int));
        mpu.setYGyroOffset(EEPROM.readInt(addressInt));
        #ifdef LS_INFO
          output = EEPROM.readInt(addressInt);
          Serial.print("address: ");Serial.println(addressInt);Serial.print("output: ");Serial.println(output);Serial.println("");
        #endif
         addressInt = addressInt + 2; //EEPROM.getAddress(sizeof(int));
       mpu.setZGyroOffset(EEPROM.readInt(addressInt));
        #ifdef LS_INFO
          output = EEPROM.readInt(addressInt);
          Serial.print("address: ");Serial.println(addressInt);Serial.print("output: ");Serial.println(output);Serial.println("");
        #endif
 #else // assign calibrated offset values here:
 /* UNIT1 */
	mpu.setXAccelOffset(46);
	mpu.setYAccelOffset(-4942);
	mpu.setZAccelOffset(4721);
	mpu.setXGyroOffset(23);
	mpu.setYGyroOffset(-11);
	mpu.setZGyroOffset(44);

	/* DIYino*/
//	mpu.setXAccelOffset(-84);
//	mpu.setYAccelOffset(788);
//	mpu.setZAccelOffset(1137);
//	mpu.setXGyroOffset(7);
//	mpu.setYGyroOffset(6);
//	mpu.setZGyroOffset(7);
#endif


  
	// make sure it worked (returns 0 if so)
	if (devStatus == 0) {
		// turn on the DMP, now that it's ready
#if defined LS_INFO
		Serial.println(F("Enabling DMP..."));
#endif
		mpu.setDMPEnabled(true);

		// enable Arduino interrupt detection
#if defined LS_INFO
		Serial.println(
				F(
						"Enabling interrupt detection (Arduino external interrupt 0)..."));
#endif
//		attachInterrupt(0, dmpDataReady, RISING);
		mpuIntStatus = mpu.getIntStatus();

		// set our DMP Ready flag so the main loop() function knows it's okay to use it
#if defined LS_INFO
		Serial.println(F("DMP ready! Waiting for first interrupt..."));
#endif
		dmpReady = true;

		// get expected DMP packet size for later comparison
		packetSize = mpu.dmpGetFIFOPacketSize();
	} else {
		// ERROR!
		// 1 = initial memory load failed
		// 2 = DMP configuration updates failed
		// (if it's going to break, usually the code will be 1)
#if defined LS_INFO
		Serial.print(F("DMP Initialization failed (code "));
		Serial.print(devStatus);
		Serial.println(F(")"));
#endif
	}

	// configure the motion interrupt for clash recognition
	// INT_PIN_CFG register
	// in the working code of MPU6050_DMP all bits of the INT_PIN_CFG are false (0)

//	mpu.setInterruptMode(false); // INT_PIN_CFG register INT_LEVEL (0-active high, 1-active low)
//	mpu.setInterruptDrive(false); // INT_PIN_CFG register INT_OPEN (0-push/pull, 1-open drain)
//	mpu.setInterruptLatch(false); // INT_PIN_CFG register LATCH_INT_EN (0 - emits 50us pulse upon trigger, 1-pin is held until int is cleared)
//	mpu.setInterruptLatchClear(false); // INT_PIN_CFG register INT_RD_CLEAR (0-clear int only on reading int status reg, 1-any read clears int)
//	mpu.setFSyncInterruptLevel(false);
//	mpu.setFSyncInterruptEnabled(false);
//	mpu.setI2CBypassEnabled(false);
//	// Enable/disable interrupt sources - enable only motion interrupt
//	mpu.setIntFreefallEnabled(false);
//	mpu.setIntMotionEnabled(true);
//	mpu.setIntZeroMotionEnabled(false);
//	mpu.setIntFIFOBufferOverflowEnabled(false);
//	mpu.setIntI2CMasterEnabled(false);
//	mpu.setIntDataReadyEnabled(false);

	mpu.setDLPFMode(3);
	mpu.setDHPFMode(0);
	//mpu.setFullScaleAccelRange(3);
	mpu.setIntMotionEnabled(true); // INT_ENABLE register enable interrupt source  motion detection
	mpu.setIntZeroMotionEnabled(false);
	mpu.setIntFIFOBufferOverflowEnabled(false);
	mpu.setIntI2CMasterEnabled(false);
	mpu.setIntDataReadyEnabled(false);
	mpu.setMotionDetectionThreshold(10); // 1mg/LSB
	mpu.setMotionDetectionDuration(2); // number of consecutive samples above threshold to trigger int
	mpuIntStatus = mpu.getIntStatus();

	/***** MP6050 MOTION DETECTOR INITIALISATION  *****/

	/***** LED SEGMENT INITIALISATION  *****/

	// initialize ledstrings segments
	DDRD |= B01101000;
	DDRB |= B00101110;

	//We shut off all pins that could wearing leds,just to be sure
	PORTD &= B10010111;
	PORTB &= B11010001;

#if defined LUXEON
	//initialise start color
	getColor(currentColor, storage.mainColor);
#endif

#if defined NEOPIXEL
	pixels.setOutput(DATA_PIN); // This initializes the NeoPixel library.
	lightOff();
	getColor(storage.sndProfile[storage.soundFont].mainColor);
  neopixels_stripeKillKey_Enable();
#endif

#if defined FoCSTRING
	pinMode(FoCSTRING, OUTPUT);
	FoCOff(FoCSTRING);
#endif

#if defined ACCENT_LED
	pinMode(ACCENT_LED, OUTPUT);
#endif

	//Randomize randomness (no really that's what it does)
	randomSeed(analogRead(2));

	/***** LED SEGMENT INITIALISATION  *****/

	/***** BUTTONS INITIALISATION  *****/
 
	// link the Main button functions.
	mainButton.setClickTicks(CLICK);
	mainButton.setPressTicks(PRESS_CONFIG);
	mainButton.attachClick(mainClick);
	//mainButton.attachDoubleClick(mainDoubleClick);
	mainButton.attachLongPressStart(mainLongPressStart);
	mainButton.attachLongPressStop(mainLongPressStop);
	mainButton.attachDuringLongPress(mainLongPress);

	// link the Lockup button functions.
  // AK 6.9.2016: if the following 2 lines are there, double click does not work
	lockupButton.setClickTicks(CLICK);
	lockupButton.setPressTicks(PRESS_CONFIG);
	lockupButton.attachClick(lockupClick);
	lockupButton.attachDoubleClick(lockupDoubleClick);
	lockupButton.attachLongPressStart(lockupLongPressStart);
	lockupButton.attachLongPressStop(lockupLongPressStop);
	lockupButton.attachDuringLongPress(lockupLongPress);
	/***** BUTTONS INITIALISATION  *****/

	/***** DF PLAYER INITIALISATION  *****/
  InitDFPlayer();

  delay(200);
  pinMode(SPK1, INPUT);
  pinMode(SPK2, INPUT);
  SinglePlay_Sound(11);
  delay(20);


  /****** INIT SABER STATE VARIABLE *****/
  SaberState = S_STANDBY;
  PrevSaberState=S_SLEEP;
  ActionModeSubStates=AS_HUM;
}

// ====================================================================================
// ===               	   			LOOP ROUTINE  	 	                			===
// ====================================================================================
void loop() {

	// if MPU6050 DMP programming failed, don't try to do anything : EPIC FAIL !
	if (!dmpReady) {
		return;
	}

	mainButton.tick();
	lockupButton.tick();

	/*//////////////////////////////////////////////////////////////////////////////////////////////////////////
	 * ACTION MODE HANDLER
	 */ /////////////////////////////////////////////////////////////////////////////////////////////////////////
	if (SaberState == S_SABERON) {
		/*
		 // In case we want to time the loop
		 Serial.print(F("Action Mode"));
		 Serial.print(F(" time="));
		 Serial.println(millis());
		 */
    if (ActionModeSubStates != AS_HUM) { // needed for hum relauch only in case it's not already being played
      hum_playing=false;
    }
    else { // AS_HUM
      if ((millis() - sndSuppress > HUM_RELAUNCH and not hum_playing)) {
        HumRelaunch();
      }
    }

		if (ActionModeSubStates == AS_IGNITION) {
			/*
			 *  This is the very first loop after Action Mode has been turned on
			 */
			attachInterrupt(0, dmpDataReady, RISING);
			// Reduce lockup trigger time for faster lockup response
			lockupButton.setPressTicks(PRESS_ACTION);
#if defined NEOPIXEL
      neopixels_stripeKillKey_Disable();
#endif
#if defined LS_INFO
			Serial.println(F("START ACTION"));
#endif
			//Play powerons wavs
			SinglePlay_Sound(soundFont.getPowerOn());
			// Light up the ledstrings
#if defined LEDSTRINGS
      lightIgnition(ledPins, soundFont.getPowerOnTime(),
          soundFont.getPowerOnEffect());
#endif
#if defined LUXEON
			lightIgnition(ledPins, currentColor, soundFont.getPowerOnTime());
#endif
#if defined NEOPIXEL
			for (uint8_t i = 0; i <= 5; i++) {
				digitalWrite(ledPins[i], HIGH);
			}
			lightIgnition(currentColor, soundFont.getPowerOnTime(), 0);

#endif
			sndSuppress = millis();
			sndSuppress2 = millis();

			// Get the initial position of the motion detector
			motionEngine();
     ActionModeSubStates=AS_HUM;
		//ignition = true;

#if defined ACCENT_LED
			// turns accent LED On
			analogWrite(ACCENT_LED, HIGH);
#endif
		}

		// ************************* blade movement detection ************************************
		//Let's get our values !
		motionEngine();

		/*
		 * CLASH DETECTION :
		 * A clash is a violent deceleration when 2 blades hit each other
		 * For a realistic clash detection it's imperative to detect
		 * such a deceleration instantenously, which is only feasible
		 * using the motion interrupt feature of the MPU6050.
		 */
		if (mpuIntStatus > 60 and mpuIntStatus < 70 and ActionModeSubStates != AS_BLADELOCKUP) {

#if defined LS_CLASH_DEBUG
			Serial.print(F("CLASH\tmpuIntStatus="));
			Serial.println(mpuIntStatus);
#endif
			if (millis() - sndSuppress >= CLASH_SUPRESS) {
				//blink = 0;
				//clash = CLASH_FLASH_TIME;
				SinglePlay_Sound(soundFont.getClash());
				sndSuppress = millis();
				sndSuppress2 = millis();
        /*
         * THIS IS A CLASH  !
         */
        ActionModeSubStates=AS_CLASH;
  #if defined LUXEON
        getColor(currentColor, storage.sndProfile[storage.soundFont].clashColor);
        lightOn(ledPins, currentColor);
  #endif
  #if defined LEDSTRINGS
      for (uint8_t i = 0; i <= 5; i++) {
        analogWrite(ledPins[i], 255);
      }
  #endif
  #if defined NEOPIXEL
    #ifdef FIREBLADE  // simply flash white
        getColor(14);
        lightOn(currentColor);
    #else
        getColor(storage.sndProfile[storage.soundFont].clashColor);
        lightOn(currentColor);
    #endif
  #endif			
        delay(CLASH_FX_DURATION);  // clash duration

      }
		}
    /*
     * SIMPLE BLADE MOVEMENT DETECTION FOR MOTION  TRIGGERED BLASTER FEDLECT
     * We detect swings as hilt's orientation change
     * since IMUs sucks at determining relative position in space
     */
   // movement of the hilt while blaster move deflect is activated can trigger a blaster deflect
   else if ((ActionModeSubStates==AS_BLASTERDEFLECTPRESS or (ActionModeSubStates==AS_BLASTERDEFLECTMOTION and (abs(curDeltAccel.y) > soundFont.getSwingThreshold()  // and it has suffisent power on a certain axis
                   or abs(curDeltAccel.z) > soundFont.getSwingThreshold()
                    or abs(curDeltAccel.x) > soundFont.getSwingThreshold()))) and (millis() - sndSuppress >= BLASTERBLOCK_SUPRESS)) {
    
          if (soundFont.getBlaster()) {
             SinglePlay_Sound(soundFont.getBlaster());
#if defined LEDSTRINGS
            blasterPin = random(6); //momentary shut off one led segment
            blink = 0;
            analogWrite(ledPins[blasterPin], LOW);
#endif
#if defined LUXEON
            getColor(currentColor, storage.sndProfile[storage.soundFont].blasterboltColor);
            lightOn(ledPins, currentColor);
#endif //LUXEON
#if defined NEOPIXEL
  #ifdef FIREBLADE
            getColor(14);
            lightOn(currentColor);
  #else
            blasterPixel = random(20, NUMPIXELS - 3); //momentary shut off one led segment
            blink = 0;
            getColor(storage.sndProfile[storage.soundFont].blasterboltColor);
            lightBlasterEffect(blasterPixel, 3, storage.sndProfile[storage.soundFont].mainColor);
  #endif
#endif
            delay(BLASTER_FX_DURATION);  // blaster bolt deflect duration
            blaster = BLASTER_FLASH_TIME;
            // Some Soundfont may not have Blaster sounds
            if (millis() - sndSuppress > 50) {
              //SinglePlay_Sound(soundFont.getBlaster());
              sndSuppress = millis();
            }
          }
  }
		/*
		 * SWING DETECTION
		 * We detect swings as hilt's orientation change
		 * since IMUs sucks at determining relative position in space
		 */
		else if (
				 ActionModeSubStates != AS_BLADELOCKUP
				 and abs(curRotation.w * 1000) < 999 // some rotation movement have been initiated
				 and (
#if defined BLADE_X

						(
								(millis() - sndSuppress > SWING_SUPPRESS) // The movement doesn't follow another to closely
								and (abs(curDeltAccel.y) > soundFont.getSwingThreshold()  // and it has suffisent power on a certain axis
										or abs(curDeltAccel.z) > soundFont.getSwingThreshold()
										or abs(curDeltAccel.x) > soundFont.getSwingThreshold()*10)
						)
						or (// A reverse movement follow a first one
								(millis() - sndSuppress2 > SWING_SUPPRESS)   // The reverse movement doesn't follow another reverse movement to closely
								// and it must be a reverse movement on Vertical axis
								and (
										abs(curDeltAccel.y) > abs(curDeltAccel.z)
										and abs(prevDeltAccel.y) > soundFont.getSwingThreshold()
										and (
												(prevDeltAccel.y > 0
												and curDeltAccel.y < -soundFont.getSwingThreshold())
												or (
														prevDeltAccel.y < 0
														and curDeltAccel.y	> soundFont.getSwingThreshold()
													)
											)
									)
							)
						or (// A reverse movement follow a first one
									(millis() - sndSuppress2 > SWING_SUPPRESS)  // The reverse movement doesn't follow another reverse movement to closely
									and ( // and it must be a reverse movement on Horizontal axis
											abs(curDeltAccel.z) > abs(curDeltAccel.y)
											and abs(prevDeltAccel.z) > soundFont.getSwingThreshold()
											and (
													(prevDeltAccel.z > 0
													and curDeltAccel.z < -soundFont.getSwingThreshold())
													or (
															prevDeltAccel.z < 0
															and curDeltAccel.z	> soundFont.getSwingThreshold()
														)
												)
										)
							)
					)

				// the movement must not be triggered by pure blade rotation (wrist rotation)
				and not (
						 abs(prevRotation.x * 1000 - curRotation.x * 1000) > abs(prevRotation.y * 1000 - curRotation.y * 1000)
						 and
						 abs(prevRotation.x * 1000 - curRotation.x * 1000) > abs(prevRotation.z * 1000 - curRotation.z * 1000)
						 )

#endif
#if defined BLADE_Y
						(
								(millis() - sndSuppress > SWING_SUPPRESS) // The movement doesn't follow another to closely
								and (abs(curDeltAccel.x) > soundFont.getSwingThreshold()  // and it has suffisent power on a certain axis
										or abs(curDeltAccel.z) > soundFont.getSwingThreshold()
										or abs(curDeltAccel.y) > soundFont.getSwingThreshold()*10)
						)
						or (// A reverse movement follow a first one
								(millis() - sndSuppress2 > SWING_SUPPRESS)   // The reverse movement doesn't follow another reverse movement to closely
								// and it must be a reverse movement on Vertical axis
								and (
										abs(curDeltAccel.x) > abs(curDeltAccel.z)
										and abs(prevDeltAccel.x) > soundFont.getSwingThreshold()
										and (
												(prevDeltAccel.x > 0
												and curDeltAccel.x < -soundFont.getSwingThreshold())
												or (
														prevDeltAccel.x < 0
														and curDeltAccel.x	> soundFont.getSwingThreshold()
													)
											)
									)
							)
						or (// A reverse movement follow a first one
									(millis() - sndSuppress2 > SWING_SUPPRESS)  // The reverse movement doesn't follow another reverse movement to closely
									and ( // and it must be a reverse movement on Horizontal axis
											abs(curDeltAccel.z) > abs(curDeltAccel.x)
											and abs(prevDeltAccel.z) > soundFont.getSwingThreshold()
											and (
													(prevDeltAccel.z > 0
													and curDeltAccel.z < -soundFont.getSwingThreshold())
													or (
															prevDeltAccel.z < 0
															and curDeltAccel.z	> soundFont.getSwingThreshold()
														)
												)
										)
							)
					)

				// the movement must not be triggered by pure blade rotation (wrist rotation)
				and not (
						 abs(prevRotation.y * 1000 - curRotation.y * 1000) > abs(prevRotation.x * 1000 - curRotation.x * 1000)
						 and
						 abs(prevRotation.y * 1000 - curRotation.y * 1000) > abs(prevRotation.z * 1000 - curRotation.z * 1000)
						 )
#endif
#if defined BLADE_Z
						(
								(millis() - sndSuppress > SWING_SUPPRESS) // The movement doesn't follow another to closely
								and (abs(curDeltAccel.y) > soundFont.getSwingThreshold()  // and it has suffisent power on a certain axis
										or abs(curDeltAccel.x) > soundFont.getSwingThreshold()
										or abs(curDeltAccel.z) > soundFont.getSwingThreshold()*10)
						)
						or (// A reverse movement follow a first one
								(millis() - sndSuppress2 > SWING_SUPPRESS)   // The reverse movement doesn't follow another reverse movement to closely
								// and it must be a reverse movement on Vertical axis
								and (
										abs(curDeltAccel.y) > abs(curDeltAccel.x)
										and abs(prevDeltAccel.y) > soundFont.getSwingThreshold()
										and (
												(prevDeltAccel.y > 0
												and curDeltAccel.y < -soundFont.getSwingThreshold())
												or (
														prevDeltAccel.y < 0
														and curDeltAccel.y	> soundFont.getSwingThreshold()
													)
											)
									)
							)
						or (// A reverse movement follow a first one
									(millis() - sndSuppress2 > SWING_SUPPRESS)  // The reverse movement doesn't follow another reverse movement to closely
									and ( // and it must be a reverse movement on Horizontal axis
											abs(curDeltAccel.x) > abs(curDeltAccel.y)
											and abs(prevDeltAccel.x) > soundFont.getSwingThreshold()
											and (
													(prevDeltAccel.x > 0
													and curDeltAccel.x < -soundFont.getSwingThreshold())
													or (
															prevDeltAccel.x < 0
															and curDeltAccel.x	> soundFont.getSwingThreshold()
														)
												)
										)
							)
					)

				// the movement must not be triggered by pure blade rotation (wrist rotation)
				and not (
						 abs(prevRotation.z * 1000 - curRotation.z * 1000) > abs(prevRotation.y * 1000 - curRotation.y * 1000)
						 and
						 abs(prevRotation.z * 1000 - curRotation.z * 1000) > abs(prevRotation.x * 1000 - curRotation.x * 1000)
						 )
#endif
){ // end of the condition definition for swings



			if ( ActionModeSubStates != AS_BLASTERDEFLECTMOTION and ActionModeSubStates != AS_BLASTERDEFLECTPRESS) {
				/*
		     *  THIS IS A SWING !
				 */
				prevDeltAccel = curDeltAccel;
#if defined LS_SWING_DEBUG
				Serial.print(F("SWING\ttime="));
				Serial.println(millis() - sndSuppress);
				Serial.print(F("\t\tcurRotation\tw="));
				Serial.print(curRotation.w * 1000);
				Serial.print(F("\t\tx="));
				Serial.print(curRotation.x);
				Serial.print(F("\t\ty="));
				Serial.print(curRotation.y);
				Serial.print(F("\t\tz="));
				Serial.print(curRotation.z);
				Serial.print(F("\t\tAcceleration\tx="));
				Serial.print(curDeltAccel.x);
				Serial.print(F("\ty="));
				Serial.print(curDeltAccel.y);
				Serial.print(F("\tz="));
				Serial.println(curDeltAccel.z);
//				Serial.print(F("\t\tprevRotation\tw="));
//				Serial.print(prevRotation.w * 1000);
//				Serial.print(F("\t\tx="));
//				Serial.print(prevRotation.x);
//				Serial.print(F("\t\ty="));
//				Serial.print(prevRotation.y);
//				Serial.print(F("\t\tz="));
//				Serial.println(prevRotation.z);
//				Serial.print(F("\tprevOrientation="));
//				Serial.print(F("\t"));
//				Serial.print(prevOrientation.w * 1000);
//				Serial.print(F("\t\tx="));
//				Serial.print(prevOrientation.x);
//				Serial.print(F("\t\ty="));
//				Serial.print(prevOrientation.y);
//				Serial.print(F("\t\tz="));
//				Serial.println(prevOrientation.z);
#endif


				/* SPIN DETECTION */
				if(		soundFont.getSpin()
						and (millis() - sndSuppress > SWING_SUPPRESS) // movement follow the precedent one shortly
						and (millis() - sndSuppress <= SWING_SUPPRESS +10)
						and (
#if defined BLADE_X

								(
									abs(curDeltAccel.y) > abs(curDeltAccel.z)
									and abs(prevDeltAccel.y) > soundFont.getSwingThreshold()
									and (
											(
													prevDeltAccel.y > 0
													and curDeltAccel.y > soundFont.getSwingThreshold())
													or (
															prevDeltAccel.y < 0
															and curDeltAccel.y	< -soundFont.getSwingThreshold()
														)
											)
									)
								or
								( // and it must be a reverse movement on Horizontal axis
									abs(curDeltAccel.z) > abs(curDeltAccel.y)
									and abs(prevDeltAccel.z) > soundFont.getSwingThreshold()
									and (
											(
													prevDeltAccel.z > 0
													and curDeltAccel.z > soundFont.getSwingThreshold())
													or (
															prevDeltAccel.z < 0
															and curDeltAccel.z	< -soundFont.getSwingThreshold()
														)
											)
										)


#endif
#if defined BLADE_Y
								(
									abs(curDeltAccel.x) > abs(curDeltAccel.z)
									and abs(prevDeltAccel.x) > soundFont.getSwingThreshold()
									and (
											(
													prevDeltAccel.x > 0
													and curDeltAccel.x > soundFont.getSwingThreshold())
													or (
															prevDeltAccel.x < 0
															and curDeltAccel.x	< -soundFont.getSwingThreshold()
														)
											)
									)
								or
								( // and it must be a reverse movement on Horizontal axis
									abs(curDeltAccel.z) > abs(curDeltAccel.x)
									and abs(prevDeltAccel.z) > soundFont.getSwingThreshold()
									and (
											(
													prevDeltAccel.z > 0
													and curDeltAccel.z > soundFont.getSwingThreshold())
													or (
															prevDeltAccel.z < 0
															and curDeltAccel.z	< -soundFont.getSwingThreshold()
														)
											)
										)

#endif
#if defined BLADE_Z
								(
									abs(curDeltAccel.y) > abs(curDeltAccel.x)
									and abs(prevDeltAccel.y) > soundFont.getSwingThreshold()
									and (
											(
													prevDeltAccel.y > 0
													and curDeltAccel.y > soundFont.getSwingThreshold())
													or (
															prevDeltAccel.y < 0
															and curDeltAccel.y	< -soundFont.getSwingThreshold()
														)
											)
									)
								or
								( // and it must be a reverse movement on Horizontal axis
									abs(curDeltAccel.x) > abs(curDeltAccel.y)
									and abs(prevDeltAccel.x) > soundFont.getSwingThreshold()
									and (
											(
													prevDeltAccel.x > 0
													and curDeltAccel.x > soundFont.getSwingThreshold())
													or (
															prevDeltAccel.x < 0
															and curDeltAccel.x	< -soundFont.getSwingThreshold()
														)
											)
										)
#endif
						)

			   ){

          ActionModeSubStates=AS_SPIN;
					SinglePlay_Sound(soundFont.getSpin());

				}/* SPIN DETECTION */
				else{ /* NORMAL SWING */
          ActionModeSubStates=AS_SWING;
					SinglePlay_Sound(soundFont.getSwing());
				}/* NORMAL SWING */




				if (millis() - sndSuppress > SWING_SUPPRESS) {
					sndSuppress = millis();
				}
				if (millis() - sndSuppress2 > SWING_SUPPRESS) {
					sndSuppress2 = millis();
				}

			}
		}
    else { // simply flicker
      if (ActionModeSubStates!=AS_BLASTERDEFLECTMOTION and ActionModeSubStates!=AS_BLADELOCKUP) { // do not deactivate blaster move deflect mode in case the saber is idling
        ActionModeSubStates=AS_HUM;
      }
      // relaunch hum if more than HUM_RELAUNCH time elapsed since entering AS_HUM state
      if (millis() - sndSuppress > HUM_RELAUNCH and not hum_playing) {
        HumRelaunch();
      }
#ifdef LEDSTRINGS
      lightFlicker(ledPins, soundFont.getFlickerEffect(),0,ActionModeSubStates);
#endif

#ifdef LUXEON
      lightFlicker(ledPins, currentColor,0);
#endif

#ifdef NEOPIXEL
      getColor(storage.sndProfile[storage.soundFont].mainColor);
      lightFlicker(0,ActionModeSubStates);
#endif
    }
		// ************************* blade movement detection ends***********************************

	} ////END ACTION MODE HANDLER///////////////////////////////////////////////////////////////////////////////////////
	/*//////////////////////////////////////////////////////////////////////////////////////////////////////////
	 * CONFIG MODE HANDLER
	 *//////////////////////////////////////////////////////////////////////////////////////////////////////////
	else if (SaberState==S_CONFIG) {
    if (PrevSaberState==S_STANDBY) { // entering config mode
      PrevSaberState=S_CONFIG;
			SinglePlay_Sound(3);
			delay(600);

#if defined NEOPIXEL
      neopixels_stripeKillKey_Disable();
#endif
     
#if defined LS_INFO
			Serial.println(F("START CONF"));
#endif
			enterMenu = true;
		}

		if (modification == -1) {

#if defined LS_INFO
			Serial.print(F("-:"));
#endif
			SinglePlay_Sound(2);
			delay(50);
		} else if (modification == 1) {

#if defined LS_INFO
			Serial.print(F("+:"));
#endif
			SinglePlay_Sound(1);
			delay(50);
		}

		switch (menu) {

 
		case 0: // SOUNDFONT
			confMenuStart(storage.soundFont, 5, menu);

			play = false;
			confParseValue(storage.soundFont, 2, SOUNDFONT_QUANTITY + 1, 1);
			if (modification) {

				modification = 0;
				storage.soundFont = value;
				soundFont.setID(value);
				SinglePlay_Sound(soundFont.getMenu());
				delay(150);

#if defined LS_INFO
				Serial.println(soundFont.getID());
#endif
			}
			break;

    case 1: //VOLUME
      confMenuStart(storage.volume, 4, menu);

      confParseValue(storage.volume, 0, 31, 1);

      if (modification) {

        modification = 0;
        storage.volume = value;
#ifdef OLD_DPFPLAYER_LIB
  mp3_set_volume (storage.volume);
#else
  dfplayer.setVolume(storage.volume); // Too Slow: we'll change volume on exit
#endif
        delay(50);
#if defined LS_INFO
        Serial.println(storage.volume);
#endif
      }

      break;
      
#if defined LUXEON
			case 2: // BLADE MAIN COLOR
			confMenuStart(storage.sndProfile[storage.soundFont].mainColor, 6, menu);

			confParseValue(storage.sndProfile[storage.soundFont].mainColor, 0, COLORS - 1, 1);

			if (modification) {

				modification = 0;
				storage.sndProfile[storage.soundFont].mainColor =value;
				getColor(currentColor, storage.sndProfile[storage.soundFont].mainColor);
				lightOn(ledPins, currentColor);
#if defined LS_INFO
				Serial.print(storage.sndProfile[storage.soundFont].mainColor);
				Serial.print("\tR:");
				Serial.print(currentColor[0]);
				Serial.print("\tG:");
				Serial.print(currentColor[1]);
				Serial.print(" \tB:");
				Serial.println(currentColor[2]);
#endif
			}
			break;
			case 3: //BLADE CLASH COLOR
			confMenuStart(storage.sndProfile[storage.soundFont].clashColor, 7, menu);

			confParseValue(storage.sndProfile[storage.soundFont].clashColor, 0, COLORS - 1, 1);

			if (modification) {

				modification = 0;
				storage.sndProfile[storage.soundFont].clashColor =value;
				getColor(currentColor, storage.sndProfile[storage.soundFont].clashColor);
				lightOn(ledPins, currentColor);
#if defined LS_INFO
				Serial.print(storage.sndProfile[storage.soundFont].clashColor);
				Serial.print("\tR:");
				Serial.print(currentColor[0]);
				Serial.print("\tG:");
				Serial.print(currentColor[1]);
				Serial.print(" \tB:");
				Serial.println(currentColor[2]);
#endif
			}
			break;
#endif

/*NEOPIXEL*/
#if defined NEOPIXEL
			case 2: // BLADE MAIN COLOR
			confMenuStart(storage.sndProfile[storage.soundFont].mainColor, 6, menu);

			confParseValue(storage.sndProfile[storage.soundFont].mainColor, 0,
					COLORS - 1, 1);

			if (modification) {

				modification = 0;
				storage.sndProfile[storage.soundFont].mainColor = value;
				getColor(storage.sndProfile[storage.soundFont].mainColor);
				lightOn(currentColor);
#if defined LS_DEBUG
				Serial.print(storage.sndProfile[storage.soundFont].mainColor);
				Serial.print("\tR:");
				Serial.print(currentColor.r);
				Serial.print("\tG:");
				Serial.print(currentColor.g);
				Serial.print(" \tB:");
				Serial.println(currentColor.b);
#endif
			}
			break;
			case 3: //BLADE CLASH COLOR
			confMenuStart(storage.sndProfile[storage.soundFont].clashColor, 7, menu);

			confParseValue(storage.sndProfile[storage.soundFont].clashColor, 0,
					COLORS - 1, 1);

			if (modification) {

				modification = 0;
				storage.sndProfile[storage.soundFont].clashColor = value;
				getColor(storage.sndProfile[storage.soundFont].clashColor);
				lightOn(currentColor);
#if defined LS_DEBUG
				Serial.print(storage.sndProfile[storage.soundFont].clashColor);
				Serial.print("\tR:");
				Serial.print(currentColor.r);
				Serial.print("\tG:");
				Serial.print(currentColor.g);
				Serial.print(" \tB:");
				Serial.println(currentColor.b);
#endif
			}
			break;
      case 4: //BLADE BLASTER BLOCK COLOR
      confMenuStart(storage.sndProfile[storage.soundFont].blasterboltColor, 8, menu);

      confParseValue(storage.sndProfile[storage.soundFont].blasterboltColor, 0,
          COLORS - 1, 1);

      if (modification) {

        modification = 0;
        storage.sndProfile[storage.soundFont].blasterboltColor = value;
        getColor(storage.sndProfile[storage.soundFont].blasterboltColor);
        lightOn(currentColor);
#if defined LS_DEBUG
        Serial.print(storage.sndProfile[storage.soundFont].blasterboltColor);
        Serial.print("\tR:");
        Serial.print(currentColor.r);
        Serial.print("\tG:");
        Serial.print(currentColor.g);
        Serial.print(" \tB:");
        Serial.println(currentColor.b);
#endif
      }
      break;
#endif
		default:
			menu = 0;
			break;
		}

	} //END CONFIG MODE HANDLER

	/*//////////////////////////////////////////////////////////////////////////////////////////////////////////
	 * STANDBY MODE
	 *//////////////////////////////////////////////////////////////////////////////////////////////////////////
	else if (SaberState==S_STANDBY) {

		if (ActionModeSubStates==AS_RETRACTION) { // we just leaved Action Mode
			detachInterrupt(0);

			SinglePlay_Sound(soundFont.getPowerOff());
      ActionModeSubStates=AS_HUM;
			changeMenu = false;
			//ignition = false;
			//blasterBlocks = false;
			modification = 0;
#if defined LS_INFO
			Serial.println(F("END ACTION"));
#endif
			lockupButton.setPressTicks(PRESS_CONFIG);
#if defined LUXEON
			lightRetract(ledPins, currentColor, soundFont.getPowerOffTime());
#endif
#if defined LEDSTRINGS
      lightRetract(ledPins, soundFont.getPowerOffTime(),
          soundFont.getPowerOffEffect());
#endif
#if defined NEOPIXEL
			lightRetract(soundFont.getPowerOffTime(), soundFont.getPowerOffEffect());
      neopixels_stripeKillKey_Enable();
#endif

		}
   if (PrevSaberState==S_CONFIG) { // we just leaved Config Mode
			saveConfig();
      PrevSaberState=S_STANDBY;

			/*
			 * RESET CONFIG
			 */
//			for (unsigned int i = 0; i < EEPROMSizeATmega328; i++) {
//				//			 if (EEPROM.read(i) != 0) {
//				EEPROM.update(i, 0);
//				//			 }
//			}

			SinglePlay_Sound(3);
			//browsing = false;
			enterMenu = false;
			modification = 0;
			//dfplayer.setVolume(storage.volume);
			menu = 0;
#if defined LUXEON
			getColor(currentColor, storage.sndProfile[storage.soundFont].mainColor);
#endif
#if defined NEOPIXEL
			getColor(storage.sndProfile[storage.soundFont].mainColor);
#endif

#if defined LS_INFO
			Serial.println(F("END CONF"));
#endif
		}

// switch of light in Stand-by mode
#if defined LUXEON
      lightOff(ledPins);
#else
      lightOff();
#endif

#if defined ACCENT_LED
#if defined HARD_ACCENT
		if (millis() - lastAccent <= 400) {
			analogWrite(ACCENT_LED, millis() - lastAccent);
		} else if (millis() - lastAccent > 400
				and millis() - lastAccent <= 800) {
			analogWrite(ACCENT_LED, 800 - (millis() - lastAccent));
		} else {
			lastAccent = millis();
		}
#endif
#if defined SOFT_ACCENT

		PWM();

		if (millis() - lastAccent >= 20) {
			// moved to own funciton for clarity
			fadeAccent();
			lastAccent = millis();
		}
#endif
#endif

	} // END STANDBY MODE
#ifdef JUKEBOX
  /*//////////////////////////////////////////////////////////////////////////////////////////////////////////
   * JUKEBOX MODE (a.k.a. MP3 player mode
   *//////////////////////////////////////////////////////////////////////////////////////////////////////////

  else if (SaberState==S_JUKEBOX) {
    if (PrevSaberState==S_STANDBY) {  // just entered JukeBox mode
      PrevSaberState=S_JUKEBOX;
      SinglePlay_Sound(14);  // play intro sound of JukeBox mode
      delay(2500);
#if defined LS_INFO
            Serial.println(F("START JUKEBOX"));
#endif       
      // start playing the first song
      jb_track=NR_CONFIGFOLDERFILES+1;
      SinglePlay_Sound(jb_track);  // JukeBox dir/files must be directly adjecent to config sounds on the SD card
    }
#ifdef LEDSTRINGS
    JukeBox_Stroboscope(ledPins);
#endif

#ifdef LUXEON
  JukeBox_Stroboscope();
#endif

#ifdef NEOPIXEL
  getColor(storage.sndProfile[storage.soundFont].mainColor);
  JukeBox_Stroboscope(currentColor);
#endif
  }
#endif
} //loop

// ====================================================================================
// ===           	  			MOTION DETECTION FUNCTIONS	            			===
// ====================================================================================
inline void motionEngine() {
// if programming failed, don't try to do anything
	if (!dmpReady)
		return;

// wait for MPU interrupt or extra packet(s) available
//	while (!mpuInterrupt && mpuFifoCount < packetSize) {
//		/* other program behavior stuff here
//		 *
//		 * If you are really paranoid you can frequently test in between other
//		 * stuff to see if mpuInterrupt is true, and if so, "break;" from the
//		 * while() loop to immediately process the MPU data
//		 */
//	}
// reset interrupt flag and get INT_STATUS byte
	mpuInterrupt = false;
	mpuIntStatus = mpu.getIntStatus();

// get current FIFO count
	mpuFifoCount = mpu.getFIFOCount();

// check for overflow (this should never happen unless our code is too inefficient)
	if ((mpuIntStatus & 0x10) || mpuFifoCount == 1024) {
// reset so we can continue cleanly
		mpu.resetFIFO();

// otherwise, check for DMP data ready interrupt (this should happen frequently)
	} else if (mpuIntStatus & 0x02) {
// wait for correct available data length, should be a VERY short wait
		while (mpuFifoCount < packetSize)
			mpuFifoCount = mpu.getFIFOCount();

// read a packet from FIFO
		mpu.getFIFOBytes(fifoBuffer, packetSize);

// track FIFO count here in case there is > 1 packet available
// (this lets us immediately read more without waiting for an interrupt)
		mpuFifoCount -= packetSize;

//Making the last orientation the reference for next rotation
		prevOrientation = curOrientation.getConjugate();
		prevAccel = curAccel;

//retrieve current orientation value
		mpu.dmpGetQuaternion(&curOrientation, fifoBuffer);
		mpu.dmpGetAccel(&curAccel, fifoBuffer);
		curDeltAccel.x = prevAccel.x - curAccel.x;
		curDeltAccel.y = prevAccel.y - curAccel.y;
		curDeltAccel.z = prevAccel.z - curAccel.z;

//We calculate the rotation quaternion since last orientation
		prevRotation = curRotation;
		curRotation = prevOrientation.getProduct(
				curOrientation.getNormalized());

#if defined LS_MOTION_HEAVY_DEBUG
// display quaternion values in easy matrix form: w x y z
		printQuaternion(curRotation);
#endif

	}
} //motionEngine

inline void dmpDataReady() {
	mpuInterrupt = true;
} //dmpDataReady

#if defined LS_MOTION_DEBUG
inline void printQuaternion(Quaternion quaternion) {
	Serial.print(F("\t\tQ\t\tw="));
	Serial.print(quaternion.w * 1000);
	Serial.print(F("\t\tx="));
	Serial.print(quaternion.x);
	Serial.print(F("\t\ty="));
	Serial.print(quaternion.y);
	Serial.print(F("\t\tz="));
	Serial.println(quaternion.z);
} //printQuaternion
#endif

// ====================================================================================
// ===           	  			EEPROM MANIPULATION FUNCTIONS	            		===
// ====================================================================================

inline bool loadConfig() {
	bool equals = true;
	EEPROM.readBlock(configAdress, storage);
	for (uint8_t i = 0; i <= 2; i++) {
		if (storage.version[i] != CONFIG_VERSION[i]) {
			equals = false;
     Serial.println("Wrong config!");
		}
	}
	Serial.println(storage.version);
	return equals;
} //loadConfig

inline void saveConfig() {
	EEPROM.updateBlock(configAdress, storage);
 #ifdef LS_DEBUG
 // dump values stored in EEPROM
 for (uint8_t i = 0; i < 255; i++) {
   Serial.print(i);Serial.print("\t");Serial.println(EEPROM.readByte(i));
 }
 #endif
} //saveConfig

// ====================================================================================
// ===              	    			LED FUNCTIONS		                		===
// ====================================================================================

#if defined SOFT_ACCENT

void PWM() {

	if (micros() - lastAccentTick >= 8) {

		if (pwmPin.state == LOW) {
			if (pwmPin.tick >= pwmPin.dutyCycle) {
				pwmPin.state = HIGH;
			}
		} else {
			if (pwmPin.tick >= abs(100 - pwmPin.dutyCycle)) {
				pwmPin.state = LOW;
				pwmPin.tick = 0;
			}
		}
		pwmPin.tick++;
		digitalWrite(ACCENT_LED, pwmPin.state);
		lastAccentTick = micros();
	}
}

void fadeAccent() {
	// go through each sw pwm pin, and increase
	// the pwm value. this would be like
	// calling analogWrite() on each hw pwm pin
	if (not pwmPin.revertCycle) {
		pwmPin.dutyCycle++;
		if (pwmPin.dutyCycle == 100)
			pwmPin.revertCycle = true;
	} else {
		pwmPin.dutyCycle--;
		if (pwmPin.dutyCycle == 0)
			pwmPin.revertCycle = false;
	}
}
#endif

// ====================================================================================
// ===                          SOUND FUNCTIONS                                     ===
// ====================================================================================

void HumRelaunch() {
    LoopPlay_Sound(soundFont.getHum());
    sndSuppress = millis();
    hum_playing=true;
}

void SinglePlay_Sound(uint8_t track) {
#ifdef OLD_DPFPLAYER_LIB
  mp3_play_physical(track);
#else // DFPlayer_LSOS
  dfplayer.playPhysicalTrack(track);
#endif
}

void LoopPlay_Sound(uint8_t track) {
#ifdef OLD_DPFPLAYER_LIB
  mp3_loop_play(track);
#else // DFPlayer_LSOS
  dfplayer.playSingleLoop(track);
#endif
}

void Set_Loop_Playback() {
#ifdef OLD_DPFPLAYER_LIB
  mp3_single_loop(true);
#else
  dfplayer.setSingleLoop(true);;
#endif    
}

void InitDFPlayer(){
#ifdef OLD_DPFPLAYER_LIB
  mp3_set_serial (mp3player);  //set softwareSerial for DFPlayer-mini mp3 module
  mp3player.begin(9600);
  delay(50);
  mp3_set_device(1); //playback from SD card
  delay(50);
  mp3_set_volume (storage.volume);
#else
  dfplayer.setSerial(DFPLAYER_TX, DFPLAYER_RX);
  // AK 7.9.2016: if the storage.volume has no or invalid value, it will cause the
  // sketch to repeat setup (reset itself) - up till now no idea why?
  // this can happen if the EEPROM is erased (i.e. reflash of bootloader)
    dfplayer.setVolume(storage.volume);

  //setup finished. Boot ready. We notify !
#endif
}

void Pause_Sound() {
#ifdef OLD_DPFPLAYER_LIB
  mp3_pause();
#else
  dfplayer.pause();
#endif  
}

void Resume_Sound() {
#ifdef OLD_DPFPLAYER_LIB
  mp3_play();
#else
  dfplayer.play();
#endif    
}



