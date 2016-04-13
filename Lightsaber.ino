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
#include <avr/sleep.h>
#include <avr/power.h>

#include "Buttons.h"
#include "Config.h"
#include "ConfigMenu.h"
#include "Light.h"
#include "SoundFont.h"

#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
#include <Wire.h>
#endif

/*
 * DO NOT MODIFY
 * Unless you know what you're doing
 *************************************/
#define CONFIG_VERSION 		"L01"
#define MEMORYBASE 			32
#define SWING_SUPPRESS 		400
#ifdef WRIST_MOVEMENTS
#define	WRIST_SENSIBILITY	1000
#endif
/************************************/

/*
 * BATTERY SAFETY MONITOR
 * Modify to match your baterries specs
 * rule of thumbs for serial wired batteries:
 * LOW_BATTERY = 3200 * Nb_of_battery_cells
 * CRITICAL_BATTERY = 3050 * Nb_of_battery_cells
 *************************************/
#define LOW_BATTERY 		3200
#define CRITICAL_BATTERY 	3050
/************************************/

/*
 * DEFAULT CONFIG PARAMETERS
 * Will be overriden by EEPROM settings
 * once the first save will be done
 *************************************/
#define VOL					13
#define SOUNDFONT 			2
#define	SWING 				800
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
Quaternion quaternion;           // [w, x, y, z]         quaternion container
VectorInt16 aaWorld; // [x, y, z]            world-frame accel sensor measurements
static Quaternion quaternion_last;  // [w, x, y, z]         quaternion container
static Quaternion quaternion_reading; // [w, x, y, z]         quaternion container
static VectorInt16 aaWorld_last; // [x, y, z]            world-frame accel sensor measurements
static VectorInt16 aaWorld_reading; // [x, y, z]            world-frame accel sensor measurements

/***************************************************************************************************
 * LED String variables
 */
#ifdef LEDSTRINGS
uint8_t ledPins[] = { LEDSTRING1, LEDSTRING2, LEDSTRING3, LEDSTRING4,
LEDSTRING5, LEDSTRING6 };
uint8_t blasterPin;
#endif
#ifdef LUXEON
uint8_t ledPins[] = {LED_RED, LED_GREEN, LED_BLUE};
uint8_t currentColor[4]; //0:Red 1:Green 2:Blue 3:ColorID
#endif
# ifdef ACCENT_LED
unsigned long lastAccent = millis();
#ifdef SOFT_ACCENT
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
long lastBatterycheck = 0;
bool lowBattery = false;
#ifdef DEEP_SLEEP
static long timeToDeepSleep = millis();
#endif
volatile uint8_t portbhistory = 0xFF;     // default is high because the pull-up
#ifdef LEDSTRINGS
struct StoreStruct {
	// This is for mere detection if they are our settings
	char version[5];
	// The settings
	uint8_t volume; // 0 to 30
	uint8_t soundFont; // as many Sound font you have defined in Soundfont.h Max:253
	uint16_t swingTreshold; // treshold acceleration for Swing
	uint8_t soundStringPreset[SOUNDFONT_QUANTITY + 2][3];
} storage;
#endif
#ifdef LUXEON
struct StoreStruct {
	// This is for mere detection if they are our settings
	char version[5];
	// The settings
	uint8_t volume;// 0 to 30
	uint8_t soundFont;// as many as Sound font you have defined in Soundfont.h Max:253
	uint16_t swingTreshold;// treshold acceleration for Swing
	uint8_t mainColor;//colorID
	uint8_t clashColor;//colorID
	uint8_t soundFontColorPreset[SOUNDFONT_QUANTITY + 2][2];//colorIDs
															// soundFontColorPreset[sndft][0] : main colorID
															// soundFontColorPreset[sndft][1] : clash colorID
}storage;
#endif
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

#ifdef LS_INFO
	// Serial line for debug
	Serial.begin(115200);
#endif
#ifdef LS_DEBUG
	// Serial line for debug
	Serial.begin(115200);
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
#ifdef LEDSTRINGS
		storage.soundStringPreset[2][0] = 0; //PowerOn
		storage.soundStringPreset[2][1] = 0; //PowerOff
		storage.soundStringPreset[2][2] = 0; //Flickering
		storage.soundStringPreset[3][0] = 1;
		storage.soundStringPreset[3][1] = 1;
		storage.soundStringPreset[3][2] = 0;
#endif
#ifdef LUXEON
		storage.mainColor = 4;
		storage.clashColor = 5;
		storage.soundFontColorPreset[2][0] = 2;
		storage.soundFontColorPreset[2][1] = 0;
		storage.soundFontColorPreset[3][0] = 0;
		storage.soundFontColorPreset[3][1] = 5;
#endif
#ifdef LS_INFO
		Serial.println(F("DEFAULT VALUE"));
#endif
	}
#ifdef LS_INFO
	else {
		Serial.println(F("EEPROM LOADED"));
	}
#endif

#ifdef LUXEON
	//initialise start color
	getColor(currentColor, storage.mainColor);
#endif

	/***** LOAD CONFIG *****/

	/***** MP6050 MOTION DETECTOR INITIALISATION  *****/

	// initialize device
#ifdef LS_INFO
	Serial.println(F("Initializing I2C devices..."));
#endif
	mpu.initialize();

	// verify connection
#ifdef LS_INFO
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
	mpu.setXAccelOffset(-2645);
	mpu.setYAccelOffset(-5491);
	mpu.setZAccelOffset(3881);
	mpu.setXGyroOffset(27);
	mpu.setYGyroOffset(-135);
	mpu.setZGyroOffset(-38);

	// make sure it worked (returns 0 if so)
	if (devStatus == 0) {
		// turn on the DMP, now that it's ready
#ifdef LS_INFO
		Serial.println(F("Enabling DMP..."));
#endif
		mpu.setDMPEnabled(true);

		// enable Arduino interrupt detection
#ifdef LS_INFO
		Serial.println(
				F(
						"Enabling interrupt detection (Arduino external interrupt 0)..."));
#endif
//		attachInterrupt(0, dmpDataReady, RISING);
		mpuIntStatus = mpu.getIntStatus();

		// set our DMP Ready flag so the main loop() function knows it's okay to use it
#ifdef LS_INFO
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
#ifdef LS_INFO
		Serial.print(F("DMP Initialization failed (code "));
		Serial.print(devStatus);
		Serial.println(F(")"));
#endif
	}

	// configure the motion interrupt for clash recognition
	// INT_PIN_CFG register
	// in the working code of MPU6050_DMP all bits of the INT_PIN_CFG are false (0)

	mpu.setInterruptMode(false); // INT_PIN_CFG register INT_LEVEL (0-active high, 1-active low)
	mpu.setInterruptDrive(false); // INT_PIN_CFG register INT_OPEN (0-push/pull, 1-open drain)
	mpu.setInterruptLatch(false); // INT_PIN_CFG register LATCH_INT_EN (0 - emits 50us pulse upon trigger, 1-pin is held until int is cleared)
	mpu.setInterruptLatchClear(false); // INT_PIN_CFG register INT_RD_CLEAR (0-clear int only on reading int status reg, 1-any read clears int)
	mpu.setFSyncInterruptLevel(false);
	mpu.setFSyncInterruptEnabled(false);
	mpu.setI2CBypassEnabled(false);
	// Enable/disable interrupt sources - enable only motion interrupt
	mpu.setIntFreefallEnabled(false);
	mpu.setIntMotionEnabled(true);
	mpu.setIntZeroMotionEnabled(false);
	mpu.setIntFIFOBufferOverflowEnabled(false);
	mpu.setIntI2CMasterEnabled(false);
	mpu.setIntDataReadyEnabled(false);
	mpu.setIntMotionEnabled(true); // INT_ENABLE register enable interrupt source  motion detection
	mpu.setMotionDetectionThreshold(10); // 1mg/LSB
	mpu.setMotionDetectionDuration(2); // number of consecutive samples above threshold to trigger int
	mpuIntStatus = mpu.getIntStatus();
#ifdef LS_CLASH_DEBUG
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
	DDRB |= B00001110;

	//We shut off all pins that could wearing leds,just to be sure
	PORTD &= B10010111;
	PORTB &= B11110001;

#ifdef FoCSTRING
	pinMode(FoCSTRING, OUTPUT);
	FoCOff(FoCSTRING);
#endif

#ifdef ACCENT_LED
	pinMode(ACCENT_LED, OUTPUT);
#endif

	//Randomize randomness (no really that's what it does)
	randomSeed(analogRead(2));

	/***** LED SEGMENT INITIALISATION  *****/

	/***** BUTTONS INITIALISATION  *****/
#ifdef DEEP_SLEEP
	PCMSK0 = (1 << PCINT4); // set PCINT4 (PIN 12) to trigger an interrupt on state change
	PCMSK2 = (1 << PCINT20); // set PCINT20 (PIN 4) to trigger an interrupt on state change
#endif

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

	//Check battery level
	if (lastBatterycheck == 0 or millis() - lastBatterycheck >= 60000) {
		uint16_t voltage = readVcc();
		if (voltage <= CRITICAL_BATTERY) {
			dfplayer.playPhysicalTrack(7);
#ifdef DEEP_SLEEP
			timeToDeepSleep = millis();
#endif
			delay(500);
			if (actionMode) {
				delay(500);
				actionMode = false;
#ifdef LIGHT_EFFECTS
				TIMSK2 = 0;
#endif
				dfplayer.playPhysicalTrack(soundFont.getPowerOff());
				changeMenu = false;
				ignition = false;
				blasterBlocks = false;
				modification = 0;
#ifdef LS_INFO
				Serial.println(F("CRITICAL BATTERY !!!"));
#endif
#ifdef LUXEON
				lightRetract(ledPins, currentColor, soundFont.getPowerOffTime());
#endif
#ifdef LEDSTRINGS
				lightRetract(ledPins, soundFont.getPowerOffTime(),
						storage.soundStringPreset[storage.soundFont][1]);
#endif
			}
			delay(70);
			dfplayer.setVolume(15);
			delay(70);
			dfplayer.playPhysicalTrack(8); // Critical Battery Alert
			delay(50);
			dfplayer.setSingleLoop(true);

			while (lastBatterycheck == lastBatterycheck) {
				// volontary infinite loop
				// WE WANT to lock the user so HE/SHE MUST change/charge
				// his/her battery before ruining it

#ifdef DEEP_SLEEP
				timeToDeepSleep = millis() - (2 * SLEEP_TIMER);
				deepSleep();
#endif
			}
		} else if (voltage <= LOW_BATTERY) {
			dfplayer.playPhysicalTrack(7); // Low Battery warning
			if (actionMode) {
				lowBattery = true;
			}
			delay(1000);
			dfplayer.playPhysicalTrack(soundFont.getHum());
			delay(40);
			if (actionMode) {

				dfplayer.setSingleLoop(true);
				delay(40);
				lowBattery = false;
				randomBlink = 0;
				blink = 0;
			}
		}

		lastBatterycheck = millis();

#ifdef LS_DEBUG
		Serial.print(F("Vcc:"));
		Serial.println(readVcc());
#endif
	}

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
#ifdef LS_INFO
			Serial.println(F("START ACTION"));
#endif
			//Play powerons wavs
			dfplayer.playPhysicalTrack(soundFont.getPowerOn());
			// Light up the ledstrings
#ifdef LEDSTRINGS
			lightIgnition(ledPins, soundFont.getPowerOnTime(),
					storage.soundStringPreset[storage.soundFont][0]);
#endif
#ifdef LUXEON
			lightIgnition(ledPins, currentColor, soundFont.getPowerOnTime());
#endif

			sndSuppress = millis();
#ifdef LIGHT_EFFECTS
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

#ifdef ACCENT_LED
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
		mpuIntStatus = mpu.getIntStatus();
		if (mpuIntStatus > 60 and mpuIntStatus < 70 and not lockup) {
			/*
			 * THIS IS A CLASH  !
			 */
#ifdef LUXEON
			getColor(currentColor, storage.clashColor);
			lightOn(ledPins, currentColor);
#endif
#ifdef LS_CLASH_DEBUG
			Serial.print(F("CLASH\tmpuIntStatus="));
			Serial.println(mpuIntStatus);
#endif
			if (millis() - sndSuppress >= 100) {
				blink = 0;
				clash = CLASH_FLASH_TIME;
				dfplayer.playPhysicalTrack(soundFont.getClash());
				sndSuppress = millis();
			}
		}
		/*
		 * SWING DETECTION
		 * We detect swings as hilt's orientation change
		 * since IMUs sucks at determining relative position in space
		 */
#ifdef BLADE_Z
		else if ((millis() - sndSuppress >= SWING_SUPPRESS) and not lockup
				and abs(quaternion.w) > storage.swingTreshold and aaWorld.z < 0
				and abs(quaternion.z) < (9 / 2) * storage.swingTreshold
				and (abs(quaternion.x) > 3 * storage.swingTreshold
						or abs(quaternion.y) > 3 * storage.swingTreshold) // We don't want to treat blade Z axe rotations as a swing
				)
#endif
#ifdef BLADE_Y
				else if ((millis() - sndSuppress >= SWING_SUPPRESS)
						and not lockup
						and abs(quaternion.w) > storage.swingTreshold
						and aaWorld.y < 0
						and abs(quaternion.y) < (9 / 2) * storage.swingTreshold
						and (abs(quaternion.x) > 3 * storage.swingTreshold
								or abs(quaternion.z) > 3 * storage.swingTreshold) // We don't want to treat blade Y axe rotations as a swing
				)
#endif
#ifdef BLADE_X
				else if ((millis() - sndSuppress >= SWING_SUPPRESS)
						and not lockup
						and abs(quaternion.w) > storage.swingTreshold
						and aaWorld.x < 0
						and abs(quaternion.x) < (9 / 2) * storage.swingTreshold
						and (abs(quaternion.z) > 3 * storage.swingTreshold
								or abs(quaternion.y) > 3 * storage.swingTreshold) // We don't want to treat blade X axe rotations as a swing
				)
#endif
				{

			if (!blasterBlocks) {
				/*
				 *  THIS IS A SWING !
				 */
#ifdef LS_SWING_DEBUG
				Serial.print(F("SWING\ttime="));
				Serial.print(millis() - sndSuppress);
				Serial.print(F("\t"));
				printAcceleration(aaWorld);
				printQuaternion(quaternion, 1);
#endif
				dfplayer.playPhysicalTrack(soundFont.getSwing());
				sndSuppress = millis();
			} else {
				if (soundFont.getBlaster()) {
#ifdef LEDSTRINGS
					blasterPin = random(6); //momentary shut off one led segment
					blink = 0;
#endif
#ifdef LUXEON
					getColor(currentColor, storage.clashColor);
					lightOn(ledPins, currentColor);

#endif
					blaster = BLASTER_FLASH_TIME;
					// Some Soundfont may not have Blaster sounds
					if (millis() - sndSuppress > 50) {
						dfplayer.playPhysicalTrack(soundFont.getBlaster());
						sndSuppress = millis();
					}
				}
			}
		}

#ifdef WRIST_MOVEMENTS

#ifdef BLADE_Z
		else if ((millis() - sndSuppress >= SWING_SUPPRESS)
				and not lockup
				and abs(quaternion.w) > storage.swingTreshold
				and (aaWorld.z > 0
						and abs(quaternion.z) > (13 / 2) * WRIST_SENSIBILITY
						and abs(quaternion.x) < 3 * storage.swingTreshold
						and abs(quaternion.y) < 3 * storage.swingTreshold)) // We specifically  treat blade Z axe rotations as a swing
#endif
#ifdef BLADE_Y
		else if ((millis() - sndSuppress >= SWING_SUPPRESS)
				and not lockup
				and abs(quaternion.w) > storage.swingTreshold
				and (aaWorld.y > 0
						and abs(quaternion.y) > (13 / 2) * WRIST_SENSIBILITY
						and abs(quaternion.x) < 3 * storage.swingTreshold
						and abs(quaternion.z) < 3 * storage.swingTreshold) ) // We specifically  treat blade Z axe rotations as a swing
#endif
#ifdef BLADE_X
		else if ((millis() - sndSuppress >= SWING_SUPPRESS)
				and not lockup
				and abs(quaternion.w) > storage.swingTreshold
				and (aaWorld.x > 0
						and abs(quaternion.x) > (13 / 2) * WRIST_SENSIBILITY
						and abs(quaternion.z) < 3 * storage.swingTreshold
						and abs(quaternion.y) < 3 * storage.swingTreshold)) // We specifically  treat blade Z axe rotations as a swing
#endif
		{
			/*
			 *  THIS IS A WRIST TWIST !
			 *  The blade did rotate around its own axe
			 */

			if (soundFont.getWrist()) {
				// Some Soundfont may not have Wrist sounds
#ifdef LS_SWING_DEBUG
				Serial.print(F("WRIST\ttime="));
				Serial.print(millis() - sndSuppress);
				Serial.print(F("\t"));
				printAcceleration(aaWorld);
				printQuaternion(quaternion, 1);
#endif
				dfplayer.playPhysicalTrack(soundFont.getWrist());
				sndSuppress = millis();
			}

		}
#endif
		// ************************* blade movement detection ends***********************************

	} ////END ACTION MODE HANDLER///////////////////////////////////////////////////////////////////////////////////////
	/*//////////////////////////////////////////////////////////////////////////////////////////////////////////
	 * CONFIG MODE HANDLER
	 *//////////////////////////////////////////////////////////////////////////////////////////////////////////
	else if (configMode) {
		if (!browsing) {
			dfplayer.playPhysicalTrack(3);
			delay(600);
#ifdef LS_INFO
			Serial.println(F("START CONF"));
#endif
			browsing = true;
			enterMenu = true;
		}

		if (modification == -1) {

#ifdef LS_INFO
			Serial.print(F("-:"));
#endif
			dfplayer.playPhysicalTrack(2);
			delay(50);
		} else if (modification == 1) {

#ifdef LS_INFO
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
#ifdef LS_INFO
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

#ifdef LUXEON
				storage.mainColor = storage.soundFontColorPreset[value][0];
				storage.clashColor = storage.soundFontColorPreset[value][1];
#endif
#ifdef LS_INFO
				Serial.println(soundFont.getID());
#endif
			}
			break;
#ifdef LEDSTRINGS
		case 2: // POWERON EFFECT
			confMenuStart(storage.soundStringPreset[storage.soundFont][0], 17,
					dfplayer);

			confParseValue(storage.soundStringPreset[storage.soundFont][0], 0,
					1, 1, dfplayer);

			if (modification) {

				modification = 0;
				storage.soundStringPreset[storage.soundFont][0] = value;
#ifdef LS_INFO
				Serial.println(storage.soundStringPreset[storage.soundFont][0]);
#endif
			}
			break;
		case 3: //POWEROFF EFFECT
			confMenuStart(storage.soundStringPreset[storage.soundFont][1], 18,
					dfplayer);

			confParseValue(storage.soundStringPreset[storage.soundFont][1], 0,
					1, 1, dfplayer);

			if (modification) {

				modification = 0;
				storage.soundStringPreset[storage.soundFont][1] = value;
#ifdef LS_INFO
				Serial.println(storage.soundStringPreset[storage.soundFont][1]);
#endif
			}
			break;
		case 4: //FLICKER EFFECT
			confMenuStart(storage.soundStringPreset[storage.soundFont][2], 19,
					dfplayer);

			confParseValue(storage.soundStringPreset[storage.soundFont][2], 0,
					2, 1, dfplayer);

			if (modification) {

				modification = 0;
				storage.soundStringPreset[storage.soundFont][2] = value;
#ifdef LS_INFO
				Serial.println(storage.soundStringPreset[storage.soundFont][2]);
#endif
			}
			break;
#endif

#ifdef LUXEON
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
#ifdef LS_INFO
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
		case 5: //SWING SENSIBILITY
			confMenuStart(storage.swingTreshold, 6, dfplayer);

			confParseValue(storage.swingTreshold, 500, 3000, -100, dfplayer);

			if (modification) {

				modification = 0;
				storage.swingTreshold = value;
#ifdef LS_INFO
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
#ifdef LIGHT_EFFECTS
			TIMSK2 &= ~(1<<OCIE2A);
#endif
			dfplayer.playPhysicalTrack(soundFont.getPowerOff());
			changeMenu = false;
			ignition = false;
			blasterBlocks = false;
			modification = 0;
#ifdef DEEP_SLEEP
			timeToDeepSleep = millis();
#endif

#ifdef LS_INFO
			Serial.println(F("END ACTION"));
#endif
			lockupButton.setPressTicks(PRESS_CONFIG);
#ifdef LUXEON
			lightRetract(ledPins, currentColor, soundFont.getPowerOffTime());
#endif
#ifdef LEDSTRINGS
			lightRetract(ledPins, soundFont.getPowerOffTime(),
					storage.soundStringPreset[storage.soundFont][1]);
#endif
		}
		if (browsing) { // we just leaved Config Mode
			saveConfig();
			lightOff(ledPins);
			dfplayer.playPhysicalTrack(3);
			browsing = false;
			enterMenu = false;
			modification = 0;
#ifdef DEEP_SLEEP
			timeToDeepSleep = millis();
#endif
			//dfplayer.setVolume(storage.volume);
			menu = 0;
#ifdef LUXEON
			getColor(currentColor, storage.mainColor);
#endif

#ifdef LS_INFO
			Serial.println(F("END CONF"));
#endif
		}

#ifdef ACCENT_LED
#ifdef HARD_ACCENT
		if (millis() - lastAccent <= 400) {
			analogWrite(ACCENT_LED, millis() - lastAccent);
		} else if (millis() - lastAccent > 400
				and millis() - lastAccent <= 800) {
			analogWrite(ACCENT_LED, 800 - (millis() - lastAccent));
		} else {
			lastAccent = millis();
		}
#endif
#ifdef SOFT_ACCENT

		PWM();

		if (millis() - lastAccent >= 7) {
			// moved to own funciton for clarity
			fadeAccent();
			lastAccent = millis();
		}
#endif
#endif

#ifdef DEEP_SLEEP
		deepSleep();
		timeToDeepSleep = millis();
#endif

	} // END STANDBY MODE
} //loop

// ====================================================================================
// ===           	  			POWER MANAGEMENT	            		        	===
// ====================================================================================

/*
 * DEEP SLEEP FUNCTION
 * NEEDS TESTING
 *  I have difficulties to get out of deep sleep mode
 */
#ifdef DEEP_SLEEP
void deepSleep() {

	if (millis() - timeToDeepSleep >= SLEEP_TIMER) {
#ifdef LS_INFO
		Serial.println(F("Powersave mode"));
#endif
		delay(20);

		set_sleep_mode(SLEEP_MODE_PWR_DOWN); // sleep mode is set here
		noInterrupts ();
		// timed sequence follows

		// enables the sleep bit in the mcucr register
		sleep_enable()
		;
		// disable ADC
		byte old_ADCSRA = ADCSRA;
		ADCSRA = 0;

		// turn off various modules
		power_all_disable ();

		//Reduce processor frequency
		byte oldCLKPR = CLKPR;
		CLKPR = bit(CLKPCE);
		CLKPR = clock_div_256;

		// turn off brown-out enable in software
		MCUCR = bit (BODS) | bit(BODSE);  // turn on brown-out enable select
		MCUCR = bit(BODS);   // this must be done within 4 clock cycles of above
		// guarantees next instruction executed
		// sleep within 3 clock cycles of above
		interrupts ();
		sleep_cpu ()
		;

		PCIFR |= bit (PCIF0) | bit(PCIF1) | bit(PCIF2); // clear any outstanding interrupts
		PCICR |= bit (PCIE0) | bit(PCIE2); // enable pin change interrupts

		sleep_mode()
		;

		// THE PROGRAM CONTINUES FROM HERE AFTER WAKING UP
		// cancel sleep as a precaution
		sleep_disable()
		;
		CLKPR = oldCLKPR;
		power_all_enable ();
		ADCSRA = old_ADCSRA;
		CLKPR = bit(CLKPCE);
		CLKPR = clock_div_1;
		interrupts ();
		delay(20);
#ifdef LS_INFO
		Serial.println(F("Normal mode"));
#endif
		delay(20);

	}
}
#endif

// ====================================================================================
// ===           	  			MOTION DETECTION FUNCTIONS	            			===
// ====================================================================================
inline void motionEngine() {
	long multiplier = 100000;
	VectorInt16 aa;    // [x, y, z]            accel sensor measurements
	VectorInt16 aaReal; // [x, y, z]            gravity-free accel sensor measurements

	VectorFloat gravity;    // [x, y, z]            gravity vector
// if programming failed, don't try to do anything
	if (!dmpReady)
		return;

// wait for MPU interrupt or extra packet(s) available
//	while (!mpuInterrupt && mpuFifoCount < packetSize) {
		/* other program behavior stuff here
		 *
		 * If you are really paranoid you can frequently test in between other
		 * stuff to see if mpuInterrupt is true, and if so, "break;" from the
		 * while() loop to immediately process the MPU data
		 */
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

//Save last values
		quaternion_last = quaternion_reading;
		aaWorld_last = aaWorld_reading;

//retrieve values
		mpu.dmpGetQuaternion(&quaternion_reading, fifoBuffer);
		mpu.dmpGetGravity(&gravity, &quaternion_reading);
		mpu.dmpGetAccel(&aa, fifoBuffer);
		mpu.dmpGetLinearAccel(&aaReal, &aa, &gravity);
		mpu.dmpGetLinearAccelInWorld(&aaWorld, &aaReal, &quaternion_reading);
#ifdef LS_MOTION_HEAVY_DEBUG
// display quaternion values in easy matrix form: w x y z
		printQuaternion(quaternion,multiplier);

// display initial world-frame acceleration, adjusted to remove gravity
// and rotated based on known orientation from quaternion
		printAcceleration(aaWorld);
#endif

//We multiply by multiplier to obtain a more precise detection
		quaternion.w = quaternion_reading.w * multiplier
				- quaternion_last.w * multiplier;
		quaternion.x = quaternion_reading.x * multiplier
				- quaternion_last.x * multiplier;
		quaternion.y = quaternion_reading.y * multiplier
				- quaternion_last.y * multiplier;
		quaternion.z = quaternion_reading.z * multiplier
				- quaternion_last.z * multiplier;
	}
} //motionEngine

inline void dmpDataReady() {
	mpuInterrupt = true;
} //dmpDataReady

#ifdef LS_MOTION_DEBUG
inline void printQuaternion(Quaternion quaternion, long multiplier) {
	Serial.print(F("\t\tQ\t\tw="));
	Serial.print(quaternion.w * multiplier);
	Serial.print(F("\t\tx="));
	Serial.print(quaternion.x * multiplier);
	Serial.print(F("\t\ty="));
	Serial.print(quaternion.y * multiplier);
	Serial.print(F("\t\tz="));
	Serial.println(quaternion.z * multiplier);
} //printQuaternion

inline void printAcceleration(VectorInt16 aaWorld) {
	Serial.print(F("\t\tA\t\tx="));
	Serial.print(aaWorld.x);
	Serial.print(F("\t\ty="));
	Serial.print(aaWorld.y);
	Serial.print(F("\t\tz="));
	Serial.print(aaWorld.z);
} //printAcceleration
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

inline uint16_t readVcc() { // courtesy of Scott Daniels : http://provideyourown.com/2012/secret-arduino-voltmeter-measure-battery-voltage/
// Read 1.1V reference against AVcc
// set the reference to Vcc and the measurement to the internal 1.1V reference
#if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
#elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
ADMUX = _BV(MUX5) | _BV(MUX0);
#elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
ADMUX = _BV(MUX3) | _BV(MUX2);
#else
	ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
#endif

	delay(2); // Wait for Vref to settle
	ADCSRA |= _BV(ADSC); // Start conversion
	while (bit_is_set(ADCSRA, ADSC))
		; // measuring

	uint8_t low = ADCL; // must read ADCL first - it then locks ADCH
	uint8_t high = ADCH; // unlocks both

	long result = (high << 8) | low;

	result = 1125300L / result; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
	return result; // Vcc in millivolts
}
// ====================================================================================
// ===              	    			LED FUNCTIONS		                		===
// ====================================================================================

#ifdef SOFT_ACCENT

void PWM() {

	if (micros() - lastAccentTick >= 8) {

		if (pwmPin.state == LOW) {
			if (pwmPin.tick >= pwmPin.dutyCycle) {
				pwmPin.state = HIGH;
			}
		} else {
			if (pwmPin.tick >= 100 - pwmPin.dutyCycle) {
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
 * each 22 µs this method is called and modifies the blade brightness
 * The parameter is defined in ignition block
 */
#ifdef LIGHT_EFFECTS
ISR(TIMER2_COMPA_vect, ISR_NOBLOCK) {

#ifdef LEDSTRINGS
	lightFlicker(ledPins, storage.soundStringPreset[storage.soundFont][2]);
#endif

#ifdef LUXEON
	lightFlicker(ledPins, currentColor,0);
#endif
	if (clash) {

		if (blink == 0) {
#ifdef LUXEON
			getColor(currentColor, storage.clashColor);
			lightOn(ledPins, currentColor);
#endif
#ifdef FoCSTRING
			FoCOn(FoCSTRING);
#endif
			blink++;
		} else if (blink < 14) {
			blink++;
#ifdef LEDSTRINGS
			lightFlicker(ledPins,
					storage.soundStringPreset[storage.soundFont][2],
					MAX_BRIGHTNESS - (blink / 2));
#endif
#ifdef LUXEON
			lightFlicker(ledPins, currentColor, MAX_BRIGHTNESS - (blink / 2));
#endif

		} else if (blink == 14) {
#ifdef LUXEON
			getColor(currentColor, storage.mainColor);
			lightOn(ledPins, currentColor);
#endif
#ifdef FoCSTRING
			FoCOff(FoCSTRING);
#endif
			blink = 0;
			clash = 0;
		}
	} else if (lockup) {
		uint8_t brightness;

		if (blink == 0) {
			brightness = random(MAX_BRIGHTNESS - 10, MAX_BRIGHTNESS);
			randomBlink = random(7, 15);
			blink++;
#ifdef FoCSTRING
			FoCOn(FoCSTRING);
#endif
#ifdef LUXEON
			getColor(currentColor, storage.clashColor);
			lightOn(ledPins, currentColor);
#endif
		} else if (blink < randomBlink) {
			blink++;
		} else if (blink == randomBlink and randomBlink >= 14) {
			blink = 0;
		} else if (blink == randomBlink and randomBlink < 14) {
			randomBlink += random(7, 15);
			brightness = 0;
#ifdef FoCSTRING
			FoCOff(FoCSTRING);
#endif
#ifdef LUXEON
			getColor(currentColor, storage.mainColor);
			lightOn(ledPins, currentColor);
#endif
		}
#ifdef LEDSTRINGS
		lightFlicker(ledPins, storage.soundStringPreset[storage.soundFont][2],
				brightness);
#endif
#ifdef LUXEON
		lightFlicker(ledPins, currentColor, brightness);
#endif

	} else if (not lockup && randomBlink != 0) { // We have released lockup button
#ifdef FoCSTRING
		FoCOff(FoCSTRING);
#endif
#ifdef LUXEON
		getColor(currentColor, storage.mainColor);
		lightOn(ledPins, currentColor);
#endif
		randomBlink = 0;
	} else if (blaster > 0) {
		if (blink < 14) {
#ifdef LEDSTRINGS
			analogWrite(ledPins[blasterPin], LOW);
#ifdef FoCSTRING
			FoCOn(FoCSTRING);
#endif
#endif
#ifdef LUXEON
			getColor(currentColor, storage.clashColor);
			lightOn(ledPins, currentColor);
#endif

			blink++;
		}
#ifdef LEDSTRINGS
		else if (blink >= 14 and blink < 19) {

			lightFlicker(ledPins,
					storage.soundStringPreset[storage.soundFont][2]);

			if (blasterPin > 0)
				analogWrite(ledPins[blasterPin - 1], LOW);

			if (blasterPin < 5)
				analogWrite(ledPins[blasterPin + 1], LOW);

			blink++;

		} else if (blink >= 19 and blink < 24) {
#endif
#ifdef LUXEON
			else if (blink >= 14 and blink < 24) {
#endif
#ifdef LEDSTRINGS
			lightFlicker(ledPins,
					storage.soundStringPreset[storage.soundFont][2]);
#endif
#ifdef LUXEON
			getColor(currentColor, storage.mainColor);
			lightOn(ledPins, currentColor);
#endif
#ifdef FoCSTRING
			FoCOff(FoCSTRING);
#endif
			blink++;
		}
			else if (blink == 24) {
#ifdef LUXEON
			getColor(currentColor, storage.mainColor);
			lightOn(ledPins, currentColor);
#endif
#ifdef FoCSTRING
			FoCOff(FoCSTRING);
#endif
#ifdef LEDSTRINGS
			lightFlicker(ledPins,
					storage.soundStringPreset[storage.soundFont][2]);
#endif
			blink = 0;
			blaster--;
		}

	} else if (lowBattery) {
		uint8_t brightness;
		if (blink == 0) {
			randomBlink = random(7, 15);
			blink++;
			brightness = random(MAX_BRIGHTNESS - 50, MAX_BRIGHTNESS - 70);
		} else if (blink < randomBlink) {
			blink++;
		} else if (blink == randomBlink and randomBlink >= 14) {
			blink = 0;
		} else if (blink == randomBlink and randomBlink < 14) {
			randomBlink += random(7, 15);
			brightness = 0;
		}
#ifdef LEDSTRINGS
		lightFlicker(ledPins, storage.soundStringPreset[storage.soundFont][2],
				brightness);
#endif
#ifdef LUXEON
		lightFlicker(ledPins, currentColor, brightness);
#endif
	}

}

#endif //LIGHT_EFFECTS
