/*
 * LightSaberOS V1.0
 *
 * released on: 10 mar 2016
 * author: 		Sebastien CAPOU (neskweek@gmail.com)
 * Source : 	https://github.com/neskweek/LightSaberOS
 * Description:	Operating System for Arduino based LightSaber
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-sa/4.0/.
 */

#include <Arduino.h>
#include <DFPlayer.h>
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

/*
 * DO NOT MODIFY
 * Unless you know what you're doing
 *************************************/
#define CONFIG_VERSION 		"L01"
#define MEMORYBASE 			32
#define SWING_SUPPRESS 		420
/************************************/

/*
 * DEFAULT CONFIG PARAMETERS
 * Will be overriden by EEPROM settings
 * once the first save will be done
 *************************************/
#define VOL					13
#define SOUNDFONT 			2
#define	SWING 				300
/************************************/

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
bool blasterBlocks = false;
uint8_t clash = 0;
bool lockup = false;
uint8_t blink = 0;
uint8_t randomBlink = 0;

/***************************************************************************************************
 * Buttons variables
 */
OneButton mainButton(MAIN_BUTTON, true);
OneButton lockupButton(LOCKUP_BUTTON, true);
bool actionMode = false; // Play with your saber
bool configMode = false; // Configure your saber
static bool ignition = false;
static bool browsing = false;

/***************************************************************************************************
 * DFPLAYER variables
 */
DFPlayer dfplayer;
SoundFont soundFont;
unsigned long sndSuppress = millis();
unsigned long sndSuppress2 = millis();

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
	uint8_t volume;     // 0 to 30
	uint8_t soundFont; // as many Sound font you have defined in Soundfont.h Max:253
	uint16_t swingTreshold;     // treshold acceleration for Swing
	uint8_t sndProfile[SOUNDFONT_QUANTITY + 2][3]; // sndProfile[sndft][0] : PowerOn effect
						       // sndProfile[sndft][1] : PowerOff effect
						       // sndProfile[sndft][2] : Flicker effect
} storage;
#endif
#if defined LUXEON
struct StoreStruct {
	// This is for mere detection if they are our settings
	char version[5];
	// The settings
	uint8_t volume;// 0 to 30
	uint8_t soundFont;// as many as Sound font you have defined in Soundfont.h Max:253
	uint16_t swingTreshold;// treshold acceleration for Swing
	uint8_t mainColor;//colorID
	uint8_t clashColor;//colorID
	uint8_t sndProfile[SOUNDFONT_QUANTITY + 2][2];	// sndProfile[sndft][0] : main colorID
							// sndProfile[sndft][1] : clash colorID
}storage;
#endif

#if defined NEOPIXEL
struct StoreStruct {
	// This is for mere detection if they are our settings
	char version[5];
	// The settings
	uint8_t volume;// 0 to 30
	uint8_t soundFont;// as many as Sound font you have defined in Soundfont.h Max:253
	uint16_t swingTreshold;// treshold acceleration for Swing
	struct Profile {
		uint8_t mainColor;	//colorID
		uint8_t clashColor;//colorID
		uint8_t pwrOn;
		uint8_t pwrOff;
		uint8_t flicker;
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

	// join I2C bus (I2Cdev library doesn't do this automatically)
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
	Wire.begin();
	TWBR = 24; // 400kHz I2C clock (200kHz if CPU is 8MHz). Comment this line if having compilation difficulties with TWBR.
#elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
			Fastwire::setup(400, true);
#endif

//#if defined LS_INFO
	// Serial line for debug
	Serial.begin(9600);
//#endif

#if defined LS_DEBUG
	// Serial line for debug
	Serial.begin(9600);
#endif
	/***** LOAD CONFIG *****/
	// Get config from EEPROM if there is one
	// or initialise value with default ones set in StoreStruct
	EEPROM.setMemPool(MEMORYBASE, EEPROMSizeATmega328); //Set memorypool base to 32, assume Arduino Uno board
	configAdress = EEPROM.getAddress(sizeof(StoreStruct)); // Size of config object

	if (!loadConfig()) {
		for (uint8_t i = 0; i <= 3; i++)
			storage.version[i] = CONFIG_VERSION[i];
		storage.soundFont = SOUNDFONT;
		storage.volume = VOL;
		storage.swingTreshold = SWING;
#if defined LEDSTRINGS
		storage.sndProfile[2][0] = 0; //PowerOn
		storage.sndProfile[2][1] = 0; //PowerOff
		storage.sndProfile[2][2] = 0; //Flickering
		storage.sndProfile[3][0] = 1;
		storage.sndProfile[3][1] = 1;
		storage.sndProfile[3][2] = 0;
#endif
#if defined LUXEON
		storage.mainColor = 4;
		storage.clashColor = 5;
		storage.sndProfile[2][0] = 2;
		storage.sndProfile[2][1] = 0;
		storage.sndProfile[3][0] = 0;
		storage.sndProfile[3][1] = 5;
#endif
#if defined NEOPIXEL
		storage.sndProfile[2].mainColor = 1;
		storage.sndProfile[2].clashColor = 0;
//		storage.sndProfile[3].mainColor = 0;
//		storage.sndProfile[3].clashColor = 2;
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
	devStatus = mpu.dmpInitialize();

	/*
	 * Those offsets are specific to each MPU6050 device.
	 * they are found via calibration process.
	 * See this script http://www.i2cdevlib.com/forums/index.php?app=core&module=attach&section=attach&attach_id=27
	 */
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
#if defined LS_CLASH_DEBUG
	Serial.println("MPU6050 register setup:");
	Serial.print("INT_PIN_CFG\t");
	Serial.print(mpu.getInterruptMode());
	Serial.print("\t");
	Serial.print(mpu.getInterruptDrive());
	Serial.print("\t");
	Serial.print(mpu.getInterruptLatch());
	Serial.print("\t");
	Serial.print(mpu.getInterruptLatchClear());
	Serial.print("\t");
	Serial.print(mpu.getFSyncInterruptLevel());
	Serial.print("\t");
	Serial.print(mpu.getFSyncInterruptEnabled());
	Serial.print("\t");
	Serial.println(mpu.getI2CBypassEnabled());
	// list INT_ENABLE register contents
	Serial.print("INT_ENABLE\t");
	Serial.print(mpu.getIntFreefallEnabled());
	Serial.print("\t");
	Serial.print(mpu.getIntMotionEnabled());
	Serial.print("\t");
	Serial.print(mpu.getIntZeroMotionEnabled());
	Serial.print("\t");
	Serial.print(mpu.getIntFIFOBufferOverflowEnabled());
	Serial.print("\t");
	Serial.print(mpu.getIntI2CMasterEnabled());
	Serial.print("\t");
	Serial.println(mpu.getIntDataReadyEnabled());
#endif
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
	lockupButton.setClickTicks(CLICK);
	lockupButton.setPressTicks(PRESS_CONFIG);
	lockupButton.attachClick(lockupClick);
	//lockupButton.attachDoubleClick(lockupDoubleClick);
	lockupButton.attachLongPressStart(lockupLongPressStart);
	lockupButton.attachLongPressStop(lockupLongPressStop);
	lockupButton.attachDuringLongPress(lockupLongPress);
	/***** BUTTONS INITIALISATION  *****/

	/***** DF PLAYER INITIALISATION  *****/
	dfplayer.setSerial(DFPLAYER_TX, DFPLAYER_RX);
	dfplayer.setVolume(storage.volume);
	delay(200);
	pinMode(SPK1, INPUT);
	pinMode(SPK2, INPUT);

	soundFont.setID(storage.soundFont);

	/***** DF PLAYER INITIALISATION  *****/

	//setup finished. Boot ready. We notify !
	dfplayer.playPhysicalTrack(16);
	delay(20);
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
	if (actionMode) {
		/*
		 // In case we want to time the loop
		 Serial.print(F("Action Mode"));
		 Serial.print(F(" time="));
		 Serial.println(millis());
		 */

		if (!ignition) {
			/*
			 *  This is the very first loop after Action Mode has been turned on
			 */
			attachInterrupt(0, dmpDataReady, RISING);
			// Reduce lockup trigger time for faster lockup response
			lockupButton.setPressTicks(PRESS_ACTION);
#if defined LS_INFO
			Serial.println(F("START ACTION"));
#endif
			//Play powerons wavs
			dfplayer.playPhysicalTrack(soundFont.getPowerOn());
			// Light up the ledstrings
#if defined LEDSTRINGS
			lightIgnition(ledPins, soundFont.getPowerOnTime(),
					storage.sndProfile[storage.soundFont][0]);

#endif
#if defined LUXEON
			lightIgnition(ledPins, currentColor, soundFont.getPowerOnTime());
#endif
#if defined NEOPIXEL
			for (uint8_t i = 0; i < 3; i++) {
				digitalWrite(ledPins[i], HIGH);
			}
			lightIgnition(currentColor, soundFont.getPowerOnTime(), 0);

#endif
			sndSuppress = millis();
			sndSuppress2 = millis();
#if defined LIGHT_EFFECTS
			/*
			 *  Interrupt Timer2 configuration
			 */
			OCR2A = 2;  // Around 44100 Hz
			TCCR2A |= (1 << WGM21); // Set to CTC Mode
			// set prescaler to 256
			TCCR2B |= (1 << CS21) | (1 << CS22);
			// start timer2 compare interrupt:
			TIMSK2 |= (1 << OCIE2A);
#endif

			// Get the initial position of the motion detector
			motionEngine();
			ignition = true;

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
		if (mpuIntStatus > 60 and mpuIntStatus < 70 and not lockup) {
			/*
			 * THIS IS A CLASH  !
			 */
#if defined LUXEON
			getColor(currentColor, storage.clashColor);
			lightOn(ledPins, currentColor);
#endif
#if defined LS_CLASH_DEBUG
			Serial.print(F("CLASH\tmpuIntStatus="));
			Serial.println(mpuIntStatus);
#endif
			if (millis() - sndSuppress >= 100) {
				blink = 0;
				clash = CLASH_FLASH_TIME;
				dfplayer.playPhysicalTrack(soundFont.getClash());
				sndSuppress = millis();
				sndSuppress2 = millis();
			}
		}
		/*
		 * SWING DETECTION
		 * We detect swings as hilt's orientation change
		 * since IMUs sucks at determining relative position in space
		 */
		else if (
				 not lockup
				 and abs(curRotation.w * 1000) < 999 // some rotation movement have been initiated
				 and (
#if defined BLADE_X

						(
								(millis() - sndSuppress > SWING_SUPPRESS) // The movement doesn't follow another to closely
								and (abs(curDeltAccel.y) > storage.swingTreshold  // and it has suffisent power on a certain axis
										or abs(curDeltAccel.z) > storage.swingTreshold
										or abs(curDeltAccel.x) > storage.swingTreshold*10)
						)
						or (// A reverse movement follow a first one
								(millis() - sndSuppress2 > SWING_SUPPRESS)   // The reverse movement doesn't follow another reverse movement to closely
								// and it must be a reverse movement on Vertical axis
								and (
										abs(curDeltAccel.y) > abs(curDeltAccel.z)
										and abs(prevDeltAccel.y) > storage.swingTreshold
										and (
												(prevDeltAccel.y > 0
												and curDeltAccel.y < -storage.swingTreshold)
												or (
														prevDeltAccel.y < 0
														and curDeltAccel.y	> storage.swingTreshold
													)
											)
									)
							)
						or (// A reverse movement follow a first one
									(millis() - sndSuppress2 > SWING_SUPPRESS)  // The reverse movement doesn't follow another reverse movement to closely
									and ( // and it must be a reverse movement on Horizontal axis
											abs(curDeltAccel.z) > abs(curDeltAccel.y)
											and abs(prevDeltAccel.z) > storage.swingTreshold
											and (
													(prevDeltAccel.z > 0
													and curDeltAccel.z < -storage.swingTreshold)
													or (
															prevDeltAccel.z < 0
															and curDeltAccel.z	> storage.swingTreshold
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
								and (abs(curDeltAccel.x) > storage.swingTreshold  // and it has suffisent power on a certain axis
										or abs(curDeltAccel.z) > storage.swingTreshold
										or abs(curDeltAccel.y) > storage.swingTreshold*10)
						)
						or (// A reverse movement follow a first one
								(millis() - sndSuppress2 > SWING_SUPPRESS)   // The reverse movement doesn't follow another reverse movement to closely
								// and it must be a reverse movement on Vertical axis
								and (
										abs(curDeltAccel.x) > abs(curDeltAccel.z)
										and abs(prevDeltAccel.x) > storage.swingTreshold
										and (
												(prevDeltAccel.x > 0
												and curDeltAccel.x < -storage.swingTreshold)
												or (
														prevDeltAccel.x < 0
														and curDeltAccel.x	> storage.swingTreshold
													)
											)
									)
							)
						or (// A reverse movement follow a first one
									(millis() - sndSuppress2 > SWING_SUPPRESS)  // The reverse movement doesn't follow another reverse movement to closely
									and ( // and it must be a reverse movement on Horizontal axis
											abs(curDeltAccel.z) > abs(curDeltAccel.x)
											and abs(prevDeltAccel.z) > storage.swingTreshold
											and (
													(prevDeltAccel.z > 0
													and curDeltAccel.z < -storage.swingTreshold)
													or (
															prevDeltAccel.z < 0
															and curDeltAccel.z	> storage.swingTreshold
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
								and (abs(curDeltAccel.y) > storage.swingTreshold  // and it has suffisent power on a certain axis
										or abs(curDeltAccel.x) > storage.swingTreshold
										or abs(curDeltAccel.z) > storage.swingTreshold*10)
						)
						or (// A reverse movement follow a first one
								(millis() - sndSuppress2 > SWING_SUPPRESS)   // The reverse movement doesn't follow another reverse movement to closely
								// and it must be a reverse movement on Vertical axis
								and (
										abs(curDeltAccel.y) > abs(curDeltAccel.x)
										and abs(prevDeltAccel.y) > storage.swingTreshold
										and (
												(prevDeltAccel.y > 0
												and curDeltAccel.y < -storage.swingTreshold)
												or (
														prevDeltAccel.y < 0
														and curDeltAccel.y	> storage.swingTreshold
													)
											)
									)
							)
						or (// A reverse movement follow a first one
									(millis() - sndSuppress2 > SWING_SUPPRESS)  // The reverse movement doesn't follow another reverse movement to closely
									and ( // and it must be a reverse movement on Horizontal axis
											abs(curDeltAccel.x) > abs(curDeltAccel.y)
											and abs(prevDeltAccel.x) > storage.swingTreshold
											and (
													(prevDeltAccel.x > 0
													and curDeltAccel.x < -storage.swingTreshold)
													or (
															prevDeltAccel.x < 0
															and curDeltAccel.x	> storage.swingTreshold
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
){



			if (!blasterBlocks) {
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
									and abs(prevDeltAccel.y) > storage.swingTreshold
									and (
											(
													prevDeltAccel.y > 0
													and curDeltAccel.y > storage.swingTreshold)
													or (
															prevDeltAccel.y < 0
															and curDeltAccel.y	< -storage.swingTreshold
														)
											)
									)
								or
								( // and it must be a reverse movement on Horizontal axis
									abs(curDeltAccel.z) > abs(curDeltAccel.y)
									and abs(prevDeltAccel.z) > storage.swingTreshold
									and (
											(
													prevDeltAccel.z > 0
													and curDeltAccel.z > storage.swingTreshold)
													or (
															prevDeltAccel.z < 0
															and curDeltAccel.z	< -storage.swingTreshold
														)
											)
										)


#endif
#if defined BLADE_Y
								(
									abs(curDeltAccel.x) > abs(curDeltAccel.z)
									and abs(prevDeltAccel.x) > storage.swingTreshold
									and (
											(
													prevDeltAccel.x > 0
													and curDeltAccel.x > storage.swingTreshold)
													or (
															prevDeltAccel.x < 0
															and curDeltAccel.x	< -storage.swingTreshold
														)
											)
									)
								or
								( // and it must be a reverse movement on Horizontal axis
									abs(curDeltAccel.z) > abs(curDeltAccel.x)
									and abs(prevDeltAccel.z) > storage.swingTreshold
									and (
											(
													prevDeltAccel.z > 0
													and curDeltAccel.z > storage.swingTreshold)
													or (
															prevDeltAccel.z < 0
															and curDeltAccel.z	< -storage.swingTreshold
														)
											)
										)

#endif
#if defined BLADE_Z
								(
									abs(curDeltAccel.y) > abs(curDeltAccel.x)
									and abs(prevDeltAccel.y) > storage.swingTreshold
									and (
											(
													prevDeltAccel.y > 0
													and curDeltAccel.y > storage.swingTreshold)
													or (
															prevDeltAccel.y < 0
															and curDeltAccel.y	< -storage.swingTreshold
														)
											)
									)
								or
								( // and it must be a reverse movement on Horizontal axis
									abs(curDeltAccel.x) > abs(curDeltAccel.y)
									and abs(prevDeltAccel.x) > storage.swingTreshold
									and (
											(
													prevDeltAccel.x > 0
													and curDeltAccel.x > storage.swingTreshold)
													or (
															prevDeltAccel.x < 0
															and curDeltAccel.x	< -storage.swingTreshold
														)
											)
										)
#endif
						)

			   ){


					dfplayer.playPhysicalTrack(soundFont.getSpin());

				}/* SPIN DETECTION */
				else{ /* NORMAL SWING */
					dfplayer.playPhysicalTrack(soundFont.getSwing());
				}/* NORMAL SWING */




				if (millis() - sndSuppress > SWING_SUPPRESS) {
					sndSuppress = millis();
				}
				if (millis() - sndSuppress2 > SWING_SUPPRESS) {
					sndSuppress2 = millis();
				}

			} else { /* BLASTER MODE */
				if (soundFont.getBlaster()) {
#if defined LEDSTRINGS
					blasterPin = random(6); //momentary shut off one led segment
					blink = 0;
#endif
#if defined LUXEON
					getColor(currentColor, storage.clashColor);
					lightOn(ledPins, currentColor);
#endif //LUXEON
#if defined NEOPIXEL
					blasterPixel = random(20, NUMPIXELS - 20); //momentary shut off one led segment
					blink = 0;
//					getColor(storage.sndProfile[storage.soundFont].clashColor);
					getColor(255);//Pure white
#endif
					blaster = BLASTER_FLASH_TIME;
					// Some Soundfont may not have #endifBlaster sounds
					if (millis() - sndSuppress > 50) {
						dfplayer.playPhysicalTrack(soundFont.getBlaster());
						sndSuppress = millis();
					}
				}
			} /* BLASTER MODE */
		}
		// ************************* blade movement detection ends***********************************

	} ////END ACTION MODE HANDLER///////////////////////////////////////////////////////////////////////////////////////
	/*//////////////////////////////////////////////////////////////////////////////////////////////////////////
	 * CONFIG MODE HANDLER
	 *//////////////////////////////////////////////////////////////////////////////////////////////////////////
	else if (configMode) {
		if (!browsing) {
			dfplayer.playPhysicalTrack(3);
			delay(600);
#if defined LS_INFO
			Serial.println(F("START CONF"));
#endif
			browsing = true;
			enterMenu = true;
		}

		if (modification == -1) {

#if defined LS_INFO
			Serial.print(F("-:"));
#endif
			dfplayer.playPhysicalTrack(2);
			delay(50);
		} else if (modification == 1) {

#if defined LS_INFO
			Serial.print(F("+:"));
#endif
			dfplayer.playPhysicalTrack(1);
			delay(50);
		}

		switch (menu) {
		case 0: //VOLUME
			confMenuStart(storage.volume, 4, dfplayer);

			confParseValue(storage.volume, 0, 30, 1, dfplayer);

			if (modification) {

				modification = 0;
				storage.volume = value;
				dfplayer.setVolume(storage.volume); // Too Slow: we'll change volume on exit
				delay(50);
#if defined LS_INFO
				Serial.println(storage.volume);
#endif
			}

			break;
		case 1: // SOUNDFONT
			confMenuStart(storage.soundFont, 5, dfplayer);

			play = false;
			confParseValue(storage.soundFont, 2, SOUNDFONT_QUANTITY + 1, 1,
					dfplayer);
			if (modification) {

				modification = 0;
				storage.soundFont = value;
				soundFont.setID(value);
				dfplayer.playPhysicalTrack(soundFont.getBoot());
				delay(150);

#if defined LUXEON
				storage.mainColor = storage.sndProfile[value][0];
				storage.clashColor = storage.sndProfile[value][1];
#endif
#if defined LS_INFO
				Serial.println(soundFont.getID());
#endif
			}
			break;

#if defined LUXEON
			case 2: // BLADE MAIN COLOR
			confMenuStart(storage.mainColor, 9, dfplayer);

			confParseValue(storage.mainColor, 0, COLORS - 1, 1, dfplayer);

			if (modification) {

				modification = 0;
				storage.mainColor = value;
				getColor(currentColor, storage.mainColor);
				lightOn(ledPins, currentColor);
#if defined LS_INFO
				Serial.print(storage.mainColor);
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
			confMenuStart(storage.clashColor, 10, dfplayer);

			confParseValue(storage.clashColor, 0, COLORS - 1, 1, dfplayer);

			if (modification) {

				modification = 0;
				storage.clashColor = value;
				getColor(currentColor, storage.clashColor);
				lightOn(ledPins, currentColor);
#if defined LS_INFO
				Serial.print(storage.clashColor);
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

#if defined LEDSTRINGS
		case 4: // POWERON EFFECT
			confMenuStart(storage.sndProfile[storage.soundFont][0], 17,
					dfplayer);

			confParseValue(storage.sndProfile[storage.soundFont][0], 0, 1, 1,
					dfplayer);

			if (modification) {

				modification = 0;
				storage.sndProfile[storage.soundFont][0] = value;
#if defined LS_INFO
				Serial.println(storage.sndProfile[storage.soundFont][0]);
#endif
			}
			break;
		case 5: //POWEROFF EFFECT
			confMenuStart(storage.sndProfile[storage.soundFont][1], 18,
					dfplayer);

			confParseValue(storage.sndProfile[storage.soundFont][1], 0, 1, 1,
					dfplayer);

			if (modification) {

				modification = 0;
				storage.sndProfile[storage.soundFont][1] = value;
#if defined LS_INFO
				Serial.println(storage.sndProfile[storage.soundFont][1]);
#endif
			}
			break;
		case 6: //FLICKER EFFECT
			confMenuStart(storage.sndProfile[storage.soundFont][2], 19,
					dfplayer);

			confParseValue(storage.sndProfile[storage.soundFont][2], 0, 2, 1,
					dfplayer);

			if (modification) {

				modification = 0;
				storage.sndProfile[storage.soundFont][2] = value;
#if defined LS_INFO
				Serial.println(storage.sndProfile[storage.soundFont][2]);
#endif
			}
			break;
#endif //LEDSTRINGS

/*LUXEON*/
#if defined LUXEON
			case 2: // BLADE MAIN COLOR
			confMenuStart(storage.mainColor, 9, dfplayer);
			confParseValue(storage.mainColor, 0, COLORS - 1, 1, dfplayer);
			if (modification) {
				modification = 0;
				storage.mainColor = value;
				getColor(currentColor, storage.mainColor);
				lightOn(ledPins, currentColor);
#ifdef LS_INFO
				Serial.print(storage.mainColor);
				Serial.print("\tR:");
				Serial.print(currentColor[0]);
				Serial.print("\tG:");
				Serial.print(currentColor[1]);
				Serial.print(" \tB:");
				Serial.println(currentColor[2]);
#endif //LS_INFO
			}
			break;
			case 3: //BLADE CLASH COLOR
			confMenuStart(storage.clashColor, 10, dfplayer);

			confParseValue(storage.clashColor, 0, COLORS - 1, 1, dfplayer);

			if (modification) {

				modification = 0;
				storage.clashColor = value;
				getColor(currentColor, storage.clashColor);
				lightOn(ledPins, currentColor);
#ifdef LS_INFO
				Serial.print(storage.clashColor);
				Serial.print("\tR:");
				Serial.print(currentColor[0]);
				Serial.print("\tG:");
				Serial.print(currentColor[1]);
				Serial.print(" \tB:");
				Serial.println(currentColor[2]);
#endif //LS_INFO
			}

			break;
			case 4: //SAVE TO SOUNDFONT ?
			confMenuStart(0, 11, dfplayer);

			if (modification > 0) {
				//Yes
				//We save color values to this Soundfount
				Serial.println(F("Yes"));
				dfplayer.playPhysicalTrack(12);
				storage.soundFontColorPreset[storage.soundFont][0] =
				storage.mainColor;
				storage.soundFontColorPreset[storage.soundFont][1] =
				storage.clashColor;
				menu++;
				changeMenu = true;
				enterMenu = true;
				delay(500);
				modification = 0;
			} else if (modification < 0) {
				//No
				// we do nothing and leave this menu
				Serial.println(F("No"));
				dfplayer.playPhysicalTrack(13);
				menu++;
				changeMenu = true;
				enterMenu = true;
				delay(20);
				modification = 0;
			}
			break;
#endif 
/*LUXEON*/













/*NEOPIXEL*/
#if defined NEOPIXEL
			case 2: // BLADE MAIN COLOR
			confMenuStart(storage.sndProfile[storage.soundFont].mainColor, 9,
					dfplayer);

			confParseValue(storage.sndProfile[storage.soundFont].mainColor, 0,
					COLORS - 1, 1, dfplayer);

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
			confMenuStart(storage.sndProfile[storage.soundFont].clashColor, 10,
					dfplayer);

			confParseValue(storage.sndProfile[storage.soundFont].clashColor, 0,
					COLORS - 1, 1, dfplayer);

			if (modification) {

				modification = 0;
				storage.sndProfile[storage.soundFont].clashColor = value;
				getColor(storage.sndProfile[storage.soundFont].clashColor);
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

			case 4: // POWERON EFFECT

			confMenuStart(storage.sndProfile[storage.soundFont].pwrOn, 17,
					dfplayer);

			confParseValue(storage.sndProfile[storage.soundFont].pwrOn, 0, 1, 1,
					dfplayer);

			if (modification) {

				modification = 0;
				storage.sndProfile[storage.soundFont].pwrOn = value;
#if defined LS_INFO
				Serial.println(storage.sndProfile[storage.soundFont].pwrOn);
#endif
			}
			break;
			case 5: //POWEROFF EFFECT
			confMenuStart(storage.sndProfile[storage.soundFont].pwrOff, 18,
					dfplayer);

			confParseValue(storage.sndProfile[storage.soundFont].pwrOff, 0, 1,
					1, dfplayer);

			if (modification) {

				modification = 0;
				storage.sndProfile[storage.soundFont].pwrOff = value;
#if defined LS_INFO
				Serial.println(storage.sndProfile[storage.soundFont].pwrOff);
#endif
			}
			break;
			case 6: //FLICKER EFFECT
			confMenuStart(storage.sndProfile[storage.soundFont].flicker, 19,
					dfplayer);

			confParseValue(storage.sndProfile[storage.soundFont].flicker, 0, 2,
					1, dfplayer);

			if (modification) {

				modification = 0;
				storage.sndProfile[storage.soundFont].flicker = value;
#if defined LS_INFO
				Serial.println(storage.sndProfile[storage.soundFont].flicker);
#endif
			}
			break;
#endif 
/*NEOPIXEL*/

		case 7: //SWING SENSIBILITY
			confMenuStart(storage.swingTreshold, 6, dfplayer);

			confParseValue(storage.swingTreshold, 200, 2000, -100, dfplayer);

			if (modification) {

				modification = 0;
				storage.swingTreshold = value;
#if defined LS_INFO
				Serial.println(storage.swingTreshold);
#endif
			}
			break;
		default:
			menu = 0;
			break;
		}

	} //END CONFIG MODE HANDLER

	/*//////////////////////////////////////////////////////////////////////////////////////////////////////////
	 * STANDBY MODE
	 *//////////////////////////////////////////////////////////////////////////////////////////////////////////
	else if (!actionMode && !configMode) {

		if (ignition) { // we just leaved Action Mode
			detachInterrupt(0);
#if defined LIGHT_EFFECTS
			TIMSK2 &= ~(1 << OCIE2A);
#endif
			dfplayer.playPhysicalTrack(soundFont.getPowerOff());
			changeMenu = false;
			ignition = false;
			blasterBlocks = false;
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
					storage.sndProfile[storage.soundFont][1]);
#endif
#if defined NEOPIXEL

			lightRetract(soundFont.getPowerOffTime(), 0);
			for (uint8_t i = 0; i < 3; i++) {
				digitalWrite(ledPins[i], LOW);
			}

#endif

		}
		if (browsing) { // we just leaved Config Mode
			saveConfig();

			/*
			 * RESET CONFIG
			 */
//			for (unsigned int i = 0; i < EEPROMSizeATmega328; i++) {
//				//			 if (EEPROM.read(i) != 0) {
//				EEPROM.update(i, 0);
//				//			 }
//			}
			lightOff();
			dfplayer.playPhysicalTrack(3);
			browsing = false;
			enterMenu = false;
			modification = 0;
			//dfplayer.setVolume(storage.volume);
			menu = 0;
#if defined LUXEON
			getColor(currentColor, storage.mainColor);
#endif
#if defined NEOPIXEL
			getColor(storage.sndProfile[storage.soundFont].mainColor);
#endif

#if defined LS_INFO
			Serial.println(F("END CONF"));
#endif
		}

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
		}
	}
	Serial.println(storage.version);
	return equals;
} //loadConfig

inline void saveConfig() {
	EEPROM.updateBlock(configAdress, storage);
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
/*
 * If no other interrupt has been triggered, and if my calculation are right
 * this timer has an almost 44100 khz frequency triggering :
 * each 22 \B5s this method is called and modifies the blade brightness
 * The parameter is defined in ignition block
 */
#if defined LIGHT_EFFECTS
ISR(TIMER2_COMPA_vect, ISR_NOBLOCK) {

#ifdef LEDSTRINGS
	lightFlicker(ledPins, storage.sndProfile[storage.soundFont][2]);
#endif

#ifdef LUXEON
	lightFlicker(ledPins, currentColor,0);
#endif
	if (clash) {

		if (blink == 0) {
#if defined LUXEON
			getColor(currentColor, storage.clashColor);
			lightOn(ledPins, currentColor);
#endif
#if defined NEOPIXEL
			if (not isFlickering) {
				getColor(storage.sndProfile[storage.soundFont].clashColor);
				lightOn(currentColor);
			}
#endif
#if defined FoCSTRING
			FoCOn(FoCSTRING);
#endif
			blink++;
		} else if (blink < 14) {
			blink++;
#if defined LEDSTRINGS
			lightFlicker(ledPins, storage.sndProfile[storage.soundFont][2],
			MAX_BRIGHTNESS - (blink / 2));
#endif
#if defined LUXEON
			lightFlicker(ledPins, currentColor, MAX_BRIGHTNESS - (blink / 2));
#endif

		} else if (blink == 14) {
#if defined LUXEON
			getColor(currentColor, storage.mainColor);
			lightOn(ledPins, currentColor);
#endif
#if defined NEOPIXEL
			if (not isFlickering) {
				getColor(storage.sndProfile[storage.soundFont].mainColor);
				lightOn(currentColor);
			}
#endif
#if defined FoCSTRING
			FoCOff(FoCSTRING);
#endif
			blink = 0;
			clash = 0;
		}
	} else if (lockup) {
		uint8_t brightness = 0;

		if (blink == 0) {
			brightness = random(MAX_BRIGHTNESS - 10, MAX_BRIGHTNESS);
			randomBlink = random(7, 15);
			blink++;
#if defined FoCSTRING
			FoCOn(FoCSTRING);
#endif
#if defined LUXEON
			getColor(currentColor, storage.clashColor);
			lightOn(ledPins, currentColor);
#endif
#if defined NEOPIXEL
//			while (isFlickering) {
//			}
			getColor(storage.sndProfile[storage.soundFont].clashColor);
			lightOn(currentColor);
#endif
		} else if (blink < randomBlink) {
			blink++;
		} else if (blink == randomBlink and randomBlink >= 14) {
			blink = 0;
		} else if (blink == randomBlink and randomBlink < 14) {
			randomBlink += random(7, 15);
			brightness = 0;
#if defined FoCSTRING
			FoCOff(FoCSTRING);
#endif
#if defined LUXEON
			getColor(currentColor, storage.mainColor);
			lightOn(ledPins, currentColor);
#endif
#if defined NEOPIXEL
			getColor(storage.sndProfile[storage.soundFont].mainColor);
			if (not isFlickering) {
				lightOn(currentColor);
			}
#endif
		}
#if defined LEDSTRINGS
		lightFlicker(ledPins, storage.sndProfile[storage.soundFont][2],
				brightness);
#endif
#if defined LUXEON
		lightFlicker(ledPins, currentColor, brightness);
#endif

	} else if (not lockup && randomBlink != 0) { // We have released lockup button
#if defined FoCSTRING
			FoCOff(FoCSTRING);
#endif
#if defined LUXEON
		getColor(currentColor, storage.mainColor);
		lightOn(ledPins, currentColor);
#endif
#if defined NEOPIXEL
		getColor(storage.sndProfile[storage.soundFont].mainColor);
		if (not isFlickering) {
			lightOn(currentColor);
		}
#endif
		randomBlink = 0;

	} else if (blaster > 0) {

#if defined NEOPIXEL
		if (blink == 2) {
			getColor(storage.sndProfile[storage.soundFont].clashColor);
		}
#endif
		if (blink < 14) {
#if defined LEDSTRINGS
			analogWrite(ledPins[blasterPin], LOW);
#if defined FoCSTRING
			FoCOn(FoCSTRING);
#endif
#endif
#if defined LUXEON
			getColor(currentColor, storage.clashColor);
			lightOn(ledPins, currentColor);
#endif

#if defined NEOPIXEL
			if (not isFlickering) {
				lightBlasterEffect(blasterPixel, blink);
			}
#endif
			blink++;
		}
   
#if defined NEOPIXEL
      else if (blink == 14) 
      {
        getColor(storage.sndProfile[storage.soundFont].mainColor);
        blink++;
      } 
      else if (blink >= 15 and blink < 29) 
#endif

#if defined LEDSTRINGS
		else if (blink >= 14 and blink < 19) {

			lightFlicker(ledPins, storage.sndProfile[storage.soundFont][2]);

			if (blasterPin > 0)
				analogWrite(ledPins[blasterPin - 1], LOW);

			if (blasterPin < 5)
				analogWrite(ledPins[blasterPin + 1], LOW);

			blink++;

		}
		else if (blink >= 19 and blink < 29)
    
#endif
#if defined LUXEON
		else if (blink >= 14 and blink < 29)
    
#endif
			{
#if defined NEOPIXEL
        if (not isFlickering) {
          lightBlasterEffect(blasterPixel, blink - 14);
        }
#endif
#if defined LEDSTRINGS
			lightFlicker(ledPins, storage.sndProfile[storage.soundFont][2]);
#endif
#if defined LUXEON
			getColor(currentColor, storage.mainColor);
			lightOn(ledPins, currentColor);
#endif
#if defined FoCSTRING
			FoCOff(FoCSTRING);
#endif
			blink++;
		}
		else if (blink == 29) 
		{
#if defined LUXEON
			getColor(currentColor, storage.mainColor);
			lightOn(ledPins, currentColor);
#endif

#if defined FoCSTRING
			FoCOff(FoCSTRING);
#endif
#if defined LEDSTRINGS
			lightFlicker(ledPins, storage.sndProfile[storage.soundFont][2]);
#endif
//#if defined NEOPIXEL
//			getColor(storage.sndProfile[storage.soundFont].mainColor);
//#endif
			blink = 0;
			blaster--;
		}

	} 
	else 
	{
#if defined LEDSTRINGS
		lightFlicker(ledPins, storage.sndProfile[storage.soundFont][2]);
#endif
#if defined NEOPIXEL
		// Neopixels string might be to slow to fill all LEDs register
		// So we let the function finish it's duty before launching a new one
		if (not isFlickering) {
			lightFlicker(0);
		}
#endif

#if defined LUXEON
		lightFlicker(ledPins, currentColor,0);
#endif
	}
}

#endif //LIGHT_EFFECTS
