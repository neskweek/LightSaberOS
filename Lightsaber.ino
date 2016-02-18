/*
 * LightSaberOS V1.0RC5
 *
 * Created on: 	10 feb 2016
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
#include "SoundFont.h"
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
#include <Wire.h>
#endif

/*
 * DO NOT MODIFY
 * Unless you know what you're doing
 */
#define CONFIG_VERSION 		"L01"
#define MEMORYBASE 			32
#define SWING_SUPPRESS 		10
#define CLASH_SUPPRESS 		40
#define CLASH_PASSES 		20
#define BRAKE_PASSES 		3

/*
 * BLADE TYPE
 * Needs to be set by user !
 */
#define LEDSTRINGS  // RGB LED USERS: Comment this line
#ifndef LEDSTRINGS
#define LUXEON
#endif

/*
 * DEFINED PINS
 * Modify to match your project setup
 * DO NOT USE PINS:
 * A4 Reserved to MPU6050 SDA
 * A5 Reserved to MPU6050 SCL
 * D2 Reserved to MPU6050 interrupts
 *
 * Spare :
 * A2,A6,A7,D13,D11
 */
#define LEDSTRING1 			3
#define LEDSTRING2 			5
#define LEDSTRING3 			6
#define LEDSTRING4 			9
#define LEDSTRING5 			10
#define LEDSTRING6 			11
#ifdef LUXEON
#define COLORS		 		48   // Number of RGB (not RGBW) colors in our array. Has to be  6x2^X. Default value is 48=6x2^4
#define LED_RED 			3
#define LED_GREEN 			5
#define LED_BLUE 			6

#define BLASTER_FLASH_TIME  5
#define CLASH_BLINK_TIME    15
#endif

#define DFPLAYER_RX			8
#define DFPLAYER_TX			7
#define SPK1				A0
#define SPK2				A1

#define MAIN_BUTTON			12
#define LOCKUP_BUTTON		4

/*
 * DEFAULT CONFIG PARAMETERS
 * Will be overriden by EEPROM settings once the first
 * save will be done
 */
#define VOL					13
#define SOUNDFONT 			2
#define	SWING 				1200
#define	CLASH_ACCEL 		11000
#define	CLASH_BRAKE 		2000

/*
 * TWEAKS PARAMETERS
 */
#define CLICK				5    // ms you need to press a button to be a click
#define PRESS_ACTION		200  // ms you need to press a button to be a long press, in action mode
#define PRESS_CONFIG		400  // ms you need to press a button to be a long press, in config mode
/* MAX_BRIGHTNESS
 * Maximum output voltage to apply to LEDS
 * Default = 200 (78,4%) Max=255 Min=0(Off)
 * WARNING ! A too high value may burn your leds. Please make your maths !
 */
#define MAX_BRIGHTNESS		200

/*
 * DEBUG PARAMETERS
 */
/* LS_INFO
 * For daily use I recommend you comment LS_INFO
 * When you plug your device to USB uncomment LS_INFO !
 */
#define LS_INFO
#ifdef LS_INFO
//#define LS_BUTTON_DEBUG
//#define LS_MOTION_DEBUG
//#define LS_MOTION_HEAVY_DEBUG
//#define LS_RELAUNCH_DEBUG
#endif
#ifdef LS_MOTION_DEBUG
//#define LS_SWING_DEBUG
//#define LS_SWING_HEAVY_DEBUG
//#define LS_CLASH_DEBUG
//#define LS_CLASH_HEAVY_DEBUG
#endif

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
static uint8_t initClash = 0;
static uint8_t initBrake = 0;
unsigned long magnitude = 0;
static bool isBigAcceleration = false;
static bool isBraking = false;
static bool isBigBrake = false;
/***************************************************************************************************
 * LED String variables
 */
#ifdef LEDSTRINGS
uint8_t ledPins[] = { LEDSTRING1, LEDSTRING2, LEDSTRING3, LEDSTRING4,
LEDSTRING5,
LEDSTRING6 };
#endif
#ifdef LUXEON
uint8_t ledPins[] = {LED_RED, LED_GREEN, LED_BLUE};

// byte color => {R,G,B,ColorNumber}
uint8_t currentColor[4];
const uint8_t rgbFactor = 100;
#endif
uint8_t brightness = 0;    // how bright the LED is
/***************************************************************************************************
 * Buttons variables
 */
OneButton mainButton(MAIN_BUTTON, true);
OneButton lockupButton(LOCKUP_BUTTON, true);
bool actionMode = false; // Play with your saber
bool configMode = false; // Configure your saber
static bool ignition = false;
static bool browsing = false;
short int modification = 0;
#ifdef LUXEON
short int blaster = -1;
uint8_t lockupBlink = 0;
#endif
/***************************************************************************************************
 * DFPLAYER variables
 */
DFPlayer mp3;
SoundFont soundFont;
volatile uint16_t lastPlayed = 0;
int16_t value = 0;
uint8_t swingSuppress = SWING_SUPPRESS * 2;
uint8_t clashSuppress = 1;
bool changePlayMode = false;
volatile bool relaunch = false;
volatile bool repeat = false;
/***************************************************************************************************
 * ConfigMode Variables
 */
uint8_t menu = 0;
bool enterMenu = false;
bool changeMenu = false;
//bool ok = true;
unsigned int configAdress = 0;
bool play = false;
#ifdef LEDSTRINGS
struct StoreStruct {
	// This is for mere detection if they are our settings
	char version[5];
	// The settings
	uint8_t volume; // 0 to 30
	uint8_t soundFont; // as many Sound font you have defined in Soundfont.h Max:253
	uint16_t swingTreshold; // treshold acceleration for Swing
	uint16_t clashAccelTreshold; // treshold acceleration for Swing
	uint16_t clashBrakeTreshold; // treshold acceleration for Swing
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
	uint16_t clashAccelTreshold;// treshold acceleration for Swing
	uint16_t clashBrakeTreshold;// treshold acceleration for Swing
	uint8_t mainColor;
	uint8_t clashColor;
	uint8_t soundFontColorPreset[SOUNDFONT_QUANTITY + 2][2];
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
		storage.clashAccelTreshold = CLASH_ACCEL;
		storage.clashBrakeTreshold = CLASH_BRAKE;
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
	lightChangeColor(storage.mainColor, false);
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
		attachInterrupt(0, dmpDataReady, RISING);
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
	/***** MP6050 MOTION DETECTOR INITIALISATION  *****/

	/***** LED SEGMENT INITIALISATION  *****/
	// initialize ledstrings segments
	pinMode(ledPins[0], OUTPUT);
	pinMode(ledPins[1], OUTPUT);
	pinMode(ledPins[2], OUTPUT);
#ifdef LEDSTRINGS
	pinMode(ledPins[3], OUTPUT);
	pinMode(ledPins[4], OUTPUT);
	pinMode(ledPins[5], OUTPUT);
#endif
	//We shutoff all pins wearing leds,just to be sure
	analogWrite(LEDSTRING1, 0);
	analogWrite(LEDSTRING2, 0);
	analogWrite(LEDSTRING3, 0);
	analogWrite(LEDSTRING4, 0);
	analogWrite(LEDSTRING5, 0);
	analogWrite(LEDSTRING6, 0);
	/***** LED SEGMENT INITIALISATION  *****/

	/***** BUTTONS INITIALISATION  *****/
	// link the Main button functions.
	mainButton.setClickTicks(CLICK);
	mainButton.setPressTicks(PRESS_CONFIG);
	mainButton.attachClick(mainClick);
	mainButton.attachDoubleClick(mainDoubleClick);
	mainButton.attachLongPressStart(mainLongPressStart);
	mainButton.attachLongPressStop(mainLongPressStop);
	mainButton.attachDuringLongPress(mainLongPress);

	// link the Lockup button functions.
	lockupButton.setClickTicks(CLICK);
	lockupButton.setPressTicks(PRESS_CONFIG);
	lockupButton.attachClick(lockupClick);
	lockupButton.attachDoubleClick(lockupDoubleClick);
	lockupButton.attachLongPressStart(lockupLongPressStart);
	lockupButton.attachLongPressStop(lockupLongPressStop);
	lockupButton.attachDuringLongPress(lockupLongPress);
	/***** BUTTONS INITIALISATION  *****/

	/***** DF PLAYER INITIALISATION  *****/
	mp3.setSerial(DFPLAYER_TX, DFPLAYER_RX);
	mp3.setVolume(storage.volume);
	pinMode(SPK1, INPUT);
	pinMode(SPK2, INPUT);
	soundFont.setFolder(storage.soundFont);
	/***** DF PLAYER INITIALISATION  *****/

	//setup finished. Boot ready. We notify !
	mp3.playTrackFromDir(16, 1);
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

	// check for DFPlayer FIFO queue overflow
	if (mp3.updateFifoCount() >= DFPLAYER_FIFO_SIZE and repeat) {
		//reset DFPlayer FIFO queue
		mp3.getSerial()->flush();
	}

	/*//////////////////////////////////////////////////////////////////////////////////////////////////////////
	 * ACTION MODE HANDLER
	 *//////////////////////////////////////////////////////////////////////////////////////////////////////////
	if (actionMode) {
		/*
		 // In case we want to time the loop
		 Serial.print(F("Action Mode"));
		 Serial.print(F(" time="));
		 Serial.println(millis());
		 */
#ifdef LUXEON
		if (blaster >= 0) {
			blaster--;
			if (blaster == 0) {
				lightChangeColor(storage.mainColor, true);
			}
		}
#endif

		if (!ignition) {
			/*
			 *  This is the very first loop after Action Mode has been turned on
			 */
			repeat = false;
			// Reduce lockup trigger time for faster lockup response
			lockupButton.setPressTicks(PRESS_ACTION);
#ifdef LS_INFO
			Serial.println(F("START ACTION"));
#endif
			//Play powerons wavs
			mp3.playTrackFromDir(soundFont.getPowerOn(), soundFont.getFolder());
			lastPlayed = mp3.getCurrentTrack();

			// Light up the ledstrings
			lightOn(ledPins);

			// Get the initial position of the motion detector
			motionEngine();
			ignition = true;
			changePlayMode = false;
			isBigAcceleration = false;
			isBigBrake = false;
		}

		/*
		 * Hum sound has been relaunched
		 */
		if (relaunch) {

			// delays are disabled in ISR functions
			delay(OPERATING_DELAY);
#ifdef LUXEON
			lightChangeColor(storage.mainColor, true);
#endif
			/* Actually we don't want to monitor this track
			 So we save some loop-time */
			//lastPlayed = mp3.getCurrentTrack();
			relaunch = false;
			repeat = true;
			changePlayMode = true;
#ifdef LS_RELAUNCH_DEBUG
			Serial.print(F("MP3 Relaunched :"));
			Serial.println(millis());
#endif

		}

		if (changePlayMode) {
			mp3.setSingleLoop(repeat);
			changePlayMode = false;
		}

		// Calculation of the amount of brightness to fade
		brightness = MAX_BRIGHTNESS
				- (abs(analogRead(SPK1) - analogRead(SPK2)));
#ifdef LS_HEAVY_DEBUG
		Serial.print(F("Brightness: "));
		Serial.print(brightness);
		Serial.print(F("   SPK1: "));
		Serial.print(analogRead(SPK1));
		Serial.print(F("   SPK2: "));
		Serial.println(analogRead(SPK2));
#endif
		lightFlicker(ledPins, brightness);

		// ************************* blade movement detection ************************************

		//Let's get our values !
		motionEngine();

		/*
		 * SWING DETECTION
		 * We detect swings as hilt's orientation change
		 * since IMUs sucks at determining relative position in space
		 */
		if (abs(quaternion.w) > storage.swingTreshold and !swingSuppress
				and aaWorld.z < 0
				and abs(quaternion.z) < (9 / 2) * storage.swingTreshold
				and (abs(quaternion.x) > (5 / 2) * storage.swingTreshold
						or abs(quaternion.y) > (5 / 2) * storage.swingTreshold)	// We don't want to treat blade Z axe rotations as a swing
				) {
			/*
			 *  THIS IS A SWING !
			 */
			repeat = false;
			swingSuppress = SWING_SUPPRESS;
			mp3.playTrackFromDir(soundFont.getSwing(), soundFont.getFolder());
			lastPlayed = mp3.getCurrentTrack();
			changePlayMode = true;

#ifdef LS_SWING_DEBUG
			Serial.print(F("SWING\t s="));
			Serial.print(swingSuppress);
			Serial.print(F("\t"));
			printAcceleration(aaWorld);
			printQuaternion(quaternion, 1);
#endif

		} else if (abs(quaternion.w) > storage.swingTreshold and !swingSuppress
				and (aaWorld.z > 0
						and abs(quaternion.z) > (13 / 2) * storage.swingTreshold
						and abs(quaternion.x) < (5 / 2) * storage.swingTreshold
						and abs(quaternion.y) < (5 / 2) * storage.swingTreshold)// We specifically  treat blade Z axe rotations as a swing
				) {
			/*
			 *  THIS IS A WRIST TWIST !
			 *  The blade did rotate around its own axe
			 */

			if (soundFont.getWrist()) {
				// Some Soundfont may not have Wrist sounds
				swingSuppress = SWING_SUPPRESS;
				repeat = false;
				mp3.playTrackFromDir(soundFont.getWrist(),
						soundFont.getFolder());
				lastPlayed = mp3.getCurrentTrack();
				changePlayMode = true;
			} else {
				swingSuppress = SWING_SUPPRESS * 10;
				delay(OPERATING_DELAY * 3);
			}

#ifdef LS_SWING_DEBUG
			Serial.print(F("WRIST\t s="));
			Serial.print(swingSuppress);
			Serial.print(F("\t"));
			printAcceleration(aaWorld);
			printQuaternion(quaternion, 1);
#endif
		} else if (swingSuppress) {
			if (swingSuppress != 0) {
				/*
				 * Disabling swing detection for a short time
				 */
				swingSuppress--;
#ifdef LS_SWING_DEBUG
			} else if (swingSuppress == 0) {
				Serial.println(F(" swing ready !"));

#endif
			}
		}

		/*
		 * CLASH DETECTION :
		 * A clash is a violent deceleration followed by an "ALMOST" full stop (violent brake)
		 * Since the accelerometer doesn't detect instantaneously the
		 * full stop we need to test it on several passes
		 */
		isBigAcceleration = magnitude > storage.clashAccelTreshold;
		isBraking = magnitude > storage.clashBrakeTreshold
				and magnitude < storage.clashAccelTreshold;
		isBigBrake = magnitude < storage.clashBrakeTreshold;
		if ((initClash == 0 or initClash <= CLASH_PASSES) and isBigAcceleration
				and not clashSuppress) {
			/*
			 * This might be a Clash ! => Violent Acceleration or Deceleration
			 * we trigger a search
			 */
			mpu.resetFIFO();
			motionEngine();
			initClash++;
#ifdef LS_CLASH_DEBUG
			Serial.print(F("INIT Clash Pass "));
			Serial.print(initClash, HEX);
			Serial.print(F("\t"));
			printMagnitude(magnitude);
			//		printAcceleration(aaWorld);
#endif

		} else if ((initClash > 1) and initClash <= CLASH_PASSES and isBraking
				and not clashSuppress and initBrake < BRAKE_PASSES) {
			/*
			 * MAYBE THAT WAS NOT A CLASH !!!
			 * Will see at next passes
			 */
			mpu.resetFIFO();
			motionEngine();
			initBrake++;
			initClash++;

#ifdef LS_CLASH_DEBUG
			Serial.print(F("BRAKING\t\t\t"));
			printMagnitude(magnitude);
			//		printAcceleration(aaWorld);
#endif
		} else if ((initClash > 1) and initClash <= CLASH_PASSES and isBraking
				and not clashSuppress and initBrake == BRAKE_PASSES) {
			/*
			 * THAT WAS NOT A CLASH !!!
			 * Depoping trigger variables
			 */
			initClash = 0;
			initBrake = 0;
#ifdef LS_CLASH_DEBUG
			Serial.print(F("NOT A CLASH (A)"));
			printMagnitude(magnitude);
			//		printAcceleration(aaWorld);
#endif

		} else if (initClash > 0 and initClash <= CLASH_PASSES and isBigBrake) {
			/*
			 * THIS IS A CLASH  ! => Violent brake !
			 */
			repeat = false;
			swingSuppress = SWING_SUPPRESS;
			mp3.playTrackFromDir(soundFont.getClash(), soundFont.getFolder());
			lastPlayed = mp3.getCurrentTrack();
			changePlayMode = true;
			initClash = 0;
			initBrake = 0;
#ifdef LUXEON
			lightChangeColor(storage.clashColor, true);
			blaster = BLASTER_FLASH_TIME;
#endif
#ifdef LS_CLASH_DEBUG
			Serial.print(F("CLASH\t"));
			Serial.print(lastPlayed);
			printMagnitude(magnitude);
			//	printAcceleration(aaWorld);
#endif
		} else {
			/*
			 * No movement recorded !
			 * Time to depop some variables
			 */

			if ((initClash > 0 and (isBigAcceleration or isBraking))
					or initClash > CLASH_PASSES) {
				/*
				 * THAT WAS DEFINITLY NOT A CLASH !!!
				 * Depoping trigger variables
				 */
				initClash = 0;
				initBrake = 0;
#ifdef LS_CLASH_DEBUG
				Serial.print(F("NOT A CLASH (B)\t"));
				printMagnitude(magnitude);
				//		printAcceleration(aaWorld);
#endif
			}
			if (clashSuppress > 0 and clashSuppress < CLASH_SUPPRESS) {
				/*
				 * Disabling Clash detection for a short time
				 */
				clashSuppress++;
			} else if (clashSuppress == CLASH_SUPPRESS) {
				clashSuppress = 0;
#ifdef LS_CLASH_DEBUG
				Serial.println(F(" clash ready !"));
#endif
			}
		}

		// ************************* blade movement detection ends***********************************
	}////END ACTION MODE HANDLER///////////////////////////////////////////////////////////////////////////////////////
	/*//////////////////////////////////////////////////////////////////////////////////////////////////////////
	 * CONFIG MODE HANDLER
	 *//////////////////////////////////////////////////////////////////////////////////////////////////////////
	else if (configMode) {
		if (!browsing) {
			mp3.playTrackFromDir(3, 1);
			delay(500);
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
			mp3.playTrackFromDir(2, 1);
		} else if (modification == 1) {

#ifdef LS_INFO
			Serial.print(F("+:"));
#endif
			mp3.playTrackFromDir(1, 1);
		}

		switch (menu) {
		case 0:
			confMenuStart(4);

			confParseValue(storage.volume, 0, 30, 1);

			if (modification) {

				modification = 0;
				storage.volume = value;
				//mp3.setVolume(storage.volume);// Too Slow: we'll change volume on exit
#ifdef LS_INFO
				Serial.println(storage.volume);
#endif
			}

			break;
		case 1:
			confMenuStart(5);

			confParseValue(storage.soundFont, 2, SOUNDFONT_QUANTITY + 1, 1);
			if (modification) {

				modification = 0;
				storage.soundFont = value;
				soundFont.setFolder(value);
				/*
				 * KNOWN BUG
				 * It doesn't play this bloody boot sound each time...
				 * Don't know why !!!
				 */
				mp3.playTrackFromDir(soundFont.getBoot(), value);

#ifdef LUXEON
				storage.mainColor = storage.soundFontColorPreset[value][0];
				storage.clashColor = storage.soundFontColorPreset[value][1];
#endif
#ifdef LS_INFO
				Serial.println(soundFont.getFolder());
#endif
			}
			break;
#ifdef LUXEON
			case 2:
			confMenuStart(9);

			confParseValue(storage.mainColor, 0, COLORS-1, 1);

			if (modification) {

				modification = 0;
				storage.mainColor = value;
				lightChangeColor(storage.mainColor, true);
#ifdef LS_INFO
				Serial.println(storage.mainColor);
#endif
			}
			break;
			case 3:
			confMenuStart(10);

			confParseValue(storage.clashColor, 0, COLORS-1, 1);

			if (modification) {

				modification = 0;
				storage.clashColor = value;
				lightChangeColor(storage.clashColor, true);
#ifdef LS_INFO
				Serial.println(storage.clashColor);
#endif
			}
			break;
			case 4:
			confMenuStart(11);

			if (modification > 0) {
				//Yes
				//We save color values to this Soundfount
				Serial.println(F("Yes"));
				mp3.playTrackFromDir(12, 1);
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
				mp3.playTrackFromDir(13, 1);
				menu++;
				changeMenu = true;
				enterMenu = true;
				delay(20);
				modification = 0;
			}
			break;
#endif
		case 5:
			confMenuStart(6);

			confParseValue(storage.swingTreshold, 1000, 3000, -100);

			if (modification) {

				modification = 0;
				storage.swingTreshold = value;
#ifdef LS_INFO
				Serial.println(storage.swingTreshold);
#endif
			}
			break;
		case 6:
			confMenuStart(7);

			confParseValue(storage.clashAccelTreshold, 5500, 15000, -500);

			if (modification) {

				modification = 0;
				storage.clashAccelTreshold = value;
#ifdef LS_INFO
				Serial.println(storage.clashAccelTreshold);
#endif
			}
			break;
		case 7:
			confMenuStart(8);

			confParseValue(storage.clashBrakeTreshold, 1500, 5000, 100);

			if (modification) {

				modification = 0;
				storage.clashBrakeTreshold = value;
#ifdef LS_INFO
				Serial.println(storage.clashBrakeTreshold);
#endif
			}
			break;
		default:
			menu = 0;
			break;
		}

	}				//END CONFIG MODE HANDLER
	/*//////////////////////////////////////////////////////////////////////////////////////////////////////////
	 * STANDBY MODE
	 *//////////////////////////////////////////////////////////////////////////////////////////////////////////
	else if (!actionMode && !configMode) {

		if (ignition) { // we just leaved Action Mode
			repeat = false;
			lockupButton.setPressTicks(PRESS_CONFIG);
			mp3.playTrackFromDir(soundFont.getPowerOff(),
					soundFont.getFolder());
			changeMenu = false;
			isBigAcceleration = false;
			ignition = false;
			relaunch = false;
			lastPlayed = 0;
			modification = 0;
#ifdef LS_INFO
			Serial.println(F("END ACTION"));
#endif
#ifdef LUXEON
			lightOff(ledPins, false);
#endif
#ifdef LEDSTRINGS
			lightOff(ledPins);
#endif
			mp3.setSingleLoop(repeat);

		}
		if (browsing) { // we just leaved Config Mode
			saveConfig();

			mp3.playTrackFromDir(3, 1);
			browsing = false;
			enterMenu = false;
			relaunch = false;
			lastPlayed = 0;
			modification = 0;

			mp3.setVolume(storage.volume);
			menu = 0;
#ifdef LUXEON
			lightOff(ledPins, false);
			lightChangeColor(storage.mainColor, false);
#endif
#ifdef LEDSTRINGS
			lightOff(ledPins);
#endif
#ifdef LS_INFO
			Serial.println(F("END CONF"));
#endif
		}

	} // END STANDBY MODE
} //loop

// ====================================================================================
// ===               			BUTTONS CALLBACK FUNCTIONS                 			===
// ====================================================================================

inline void mainClick() {
#ifdef LS_BUTTON_DEBUG
	Serial.println(F("Main button click."));
#endif
	if (actionMode) {
		if (soundFont.getForce()) {
			// Some Soundfont may not have Force sounds
			repeat = false;
			mp3.playTrackFromDir(soundFont.getForce(), soundFont.getFolder());
			lastPlayed = mp3.getCurrentTrack();
			changePlayMode = true;
			initClash = 0;
		}
	} else if (configMode) {
		//Button "+"
		modification = 1;
		play = true;
	} else if (!configMode && !actionMode) {
		// LightSaber poweron
		actionMode = true;
		;
	}
} // mainClick

inline void mainDoubleClick() {
#ifdef LS_BUTTON_DEBUG
	Serial.println(F("Main button double click."));
#endif
	if (actionMode) {
		/*
		 * ACTION TO DEFINE
		 */
	} else if (configMode) {
		/*
		 * Trigger needs to be hardened with some sort of double click combinaison
		 */
		//RESET CONFIG
		/*
		 for (unsigned int i = 0; i < EEPROMSizeATmega328; i++) {
		 //			 if (EEPROM.read(i) != 0) {
		 EEPROM.update(i, 0);
		 //			 }
		 }
		 */
	} else if (!configMode && !actionMode) {
		/*
		 * ACTION TO DEFINE
		 */
	}
} // mainDoubleClick

inline void mainLongPressStart() {
#ifdef LS_BUTTON_DEBUG
	Serial.println(F("Main button longPress start"));
#endif
	if (actionMode) {
		// LightSaber shutdown
		actionMode = false;
	} else if (configMode) {
		// Change Menu
		changeMenu = true;
		enterMenu = true;
		menu++;
#ifdef LEDSTRINGS
		lightOff(ledPins);
		if (menu == 2) {
			menu = 5;
		}
		analogWrite(ledPins[menu], 160);
#endif

	} else if (!configMode && !actionMode) {
		/*
		 * ACTION TO DEFINE
		 */
	}
} // mainLongPressStart

inline void mainLongPress() {
#ifdef LS_BUTTON_DEBUG
	Serial.println(F("Main button longPress..."));
#endif
	if (actionMode) {
		/*
		 * ACTION TO DEFINE
		 */

	} else if (configMode) {
		/*
		 * ACTION TO DEFINE
		 */
	} else if (!configMode && !actionMode) {
		/*
		 * ACTION TO DEFINE
		 */
	}
} // mainLongPress

inline void mainLongPressStop() {
#ifdef LS_BUTTON_DEBUG
	Serial.println(F("Main button longPress stop"));
#endif
	if (!configMode && !actionMode) {
		/*
		 * ACTION TO DEFINE
		 */
	}
} // mainLongPressStop

inline void lockupClick() {
#ifdef LS_BUTTON_DEBUG
	Serial.println(F("Lockup button click."));
#endif
	if (actionMode) {
		// Blaster
#ifdef LEDSTRINGS
		analogWrite(ledPins[random(6)], LOW); //momentary shut off one led segment
#endif
#ifdef LUXEON
		lightChangeColor(storage.clashColor, true);
		blaster = BLASTER_FLASH_TIME;
#endif
		if (soundFont.getBlaster()) {
			// Some Soundfont may not have Blaster sounds
			repeat = false;
			mp3.playTrackFromDir(soundFont.getBlaster(), soundFont.getFolder());
			lastPlayed = mp3.getCurrentTrack();
			changePlayMode = true;
			initClash = 0;
		}

	} else if (configMode) {
		// Button "-"
		modification = -1;
		play = true;
	} else if (!configMode && !actionMode) {
		/*
		 * ACTION TO DEFINE
		 */
	}
} // lockupClick

inline void lockupDoubleClick() {
#ifdef LS_BUTTON_DEBUG
	Serial.println(F("Lockup button double click."));
#endif
	if (actionMode) {
		/*
		 * ACTION TO DEFINE
		 */
	} else if (configMode) {
		/*
		 * ACTION TO DEFINE
		 */
	} else if (!configMode && !actionMode) {
		/*
		 * ACTION TO DEFINE
		 */
	}
} // lockupDoubleClick

inline void lockupLongPressStart() {
#ifdef LS_BUTTON_DEBUG
	Serial.println(F("Lockup button longPress start"));
#endif
	if (actionMode) {
		//Lockup Start
#ifdef LUXEON
		lightChangeColor(storage.clashColor, true);
		lockupBlink = random(CLASH_BLINK_TIME);
#endif
		//		Serial.println(soundFont.getLockup());
		if (soundFont.getLockup()) {
			mp3.playTrackFromDir(soundFont.getLockup(), soundFont.getFolder());
			repeat = true;
			changePlayMode = true;
			initClash = 0;
			lastPlayed = mp3.getCurrentTrack();
		}
	} else if (configMode) {
		//Leaving Config Mode
		changeMenu = false;
		repeat = true;
		configMode = false;

	} else if (!configMode && !actionMode) {
		//Entering Config Mode
		configMode = true;

	}
} // lockupLongPressStart

inline void lockupLongPress() {
#ifdef LS_BUTTON_DEBUG
	Serial.println(F("Lockup button longPress..."));
#endif
	if (actionMode) {
#ifdef LUXEON
		//
		if (lockupBlink > 0) {
			lockupBlink--;
		} else {
			if (currentColor[3] == storage.clashColor) {
				lockupBlink = random(CLASH_BLINK_TIME);
				lightChangeColor(storage.mainColor, false);
			} else if (currentColor[3] == storage.mainColor) {
				lockupBlink = random(CLASH_BLINK_TIME);
				lightChangeColor(storage.clashColor, false);
			}
		}
#endif
	} else if (configMode) {
		/*
		 * ACTION TO DEFINE
		 */
	} else if (!configMode && !actionMode) {
		/*
		 * ACTION TO DEFINE
		 */
	}
} // lockupLongPress

inline void lockupLongPressStop() {
#ifdef LS_BUTTON_DEBUG
	Serial.println(F("Lockup button longPress stop"));
#endif
	if (actionMode) {
		//Lockup Stop
#ifdef LUXEON
		lightChangeColor(storage.mainColor, true);
		lockupBlink = 0;
#endif
		mp3.playTrackFromDir(4, soundFont.getFolder());
		repeat = true;
		lastPlayed = mp3.getCurrentTrack();
		changePlayMode = true;
		clashSuppress = 0;
		swingSuppress = 0;
		initClash = 0;

	}
} // lockupLongPressStop

// ====================================================================================
// ===              	    			LED FUNCTIONS		                		===
// ====================================================================================

#ifdef LEDSTRINGS
inline void lightOn(uint8_t ledPins[]) {

// Light up the ledstrings
	for (uint8_t i = 0; i <= 5; i++) {
		digitalWrite(ledPins[i], HIGH);
		if (i < 5) {
			delay(83);
		}
	}

}				//lightOn

inline void lightOff(uint8_t ledPins[]) {
// Light off the ledstrings
	for (uint8_t i = 6; i > 0; i--) {
		digitalWrite(ledPins[i - 1], LOW);
		if (i - 1 > 0) {
			delay(83);
		}
	}
}				//lightOff

inline void lightFlicker(uint8_t ledPins[], uint8_t value) {
// lightFlicker the ledstrings
	for (uint8_t i = 0; i <= 5; i++) {
		analogWrite(ledPins[i], value);
	}
} //lightFlicker
#endif

#ifdef LUXEON

inline void lightOn(uint8_t ledPins[]) {

	// Fade in to Maximum brightness
	for (unsigned int fadeIn = 255; fadeIn > 0; fadeIn--) {
		for (uint8_t i = 0; i <= 2; i++) {
			analogWrite(ledPins[i],
					(MAX_BRIGHTNESS / fadeIn) * currentColor[i] / 100);
		}
		delay(2);
	}
} //lightOn

inline void lightOff(uint8_t ledPins[], bool fade) {
	// Fade out
	if (fade) {
		for (unsigned int fadeOut = 255; fadeOut > 0; fadeOut--) {
			for (uint8_t i = 0; i <= 2; i++) {
				analogWrite(ledPins[i],
						(MAX_BRIGHTNESS - (MAX_BRIGHTNESS / fadeOut))
						* currentColor[i] / 100);
			}
			delay(2);
		}
	}
	// shut Off
	for (uint8_t i = 0; i <= 2; i++) {
		analogWrite(ledPins[i], LOW);
	}
} //lightOff

inline void lightFlicker(uint8_t ledPins[], uint8_t value) {
	for (uint8_t i = 0; i <= 2; i++) {
		analogWrite(ledPins[i], value * currentColor[i] / rgbFactor);
	}
} //lightFlicker

inline void getColor(uint8_t id) {
	uint8_t tint = (COLORS / 6);
	uint8_t step = 100 / tint;
	currentColor[3] = id;
	if ((id >= 0) and (id < (1 * tint))) {
		//From Red to Yellow
		currentColor[0] = 100;
		currentColor[1] = step * (id % tint);
		currentColor[2] = 0;
	} else if ((id >= (1 * tint)) and (id < (2 * tint))) {
		// From Yellow to Green
		currentColor[0] = 100 - (step * (id % tint));
		currentColor[1] = 100;
		currentColor[2] = 0;
	} else if ((id >= (2 * tint)) and (id < (3 * tint))) {
		// From Green to Aqua
		currentColor[0] = 0;
		currentColor[1] = 100;
		currentColor[2] = step * (id % tint);
	} else if ((id >= (3 * tint)) and (id < (4 * tint))) {
		// From Aqua to Blue
		currentColor[0] = 0;
		currentColor[1] = 100 - (step * (id % tint));
		currentColor[2] = 100;
	} else if ((id >= (4 * tint)) and (id < (5 * tint))) {
		// From Blue to Purple/Pink
		currentColor[0] = step * (id % tint);
		currentColor[1] = 0;
		currentColor[2] = 100;
	} else if (id >= (5 * tint)) {
		// From Purple/Pink to Red
		currentColor[0] = 100;
		currentColor[1] = 0;
		currentColor[2] = 100 - (step * (id % tint));
	}

}

inline void lightChangeColor(uint8_t color, bool lightup) {
	getColor(color);
	if (lightup) {
		for (uint8_t i = 0; i <= 2; i++) {
			analogWrite(ledPins[i],
					MAX_BRIGHTNESS * currentColor[i] / 100);
		}
	}
} //lightChangeColor
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
	while (!mpuInterrupt && mpuFifoCount < packetSize) {
		/* other program behavior stuff here
		 *
		 * If you are really paranoid you can frequently test in between other
		 * stuff to see if mpuInterrupt is true, and if so, "break;" from the
		 * while() loop to immediately process the MPU data
		 */
	}

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

		//Magnitude to calculate force of impact.
		magnitude = 1000 * sqrt(
		sq(
				aaWorld.x
				/ 1000) + sq(aaWorld.y / 1000) + sq(aaWorld.z / 1000)); //Magnitude to calculate force of impact.
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

inline void printMagnitude(long magnitude) {
	Serial.print(F("\t\tM="));
	Serial.println(magnitude);
} //printMagnitude
#endif

// ====================================================================================
// ===           	  	 			CONFIG MODE FUNCTIONS	                		===
// ====================================================================================

inline void confParseValue(uint16_t variable, uint16_t min, uint16_t max,
		short int multiplier) {
	value = variable + (multiplier * modification);
	if (value < (int) min) {
		value = max;
	} else if (value > (int) max) {
		value = min;
	} else if (value == (int) min and play) {
		play = false;
		mp3.playTrackFromDir(15, 1);
		delay(100);
	} else if (value == (int) max and play) {
		play = false;
		mp3.playTrackFromDir(14, 1);
		delay(100);
	}
} //confParseValue

inline void confMenuStart(uint8_t sound) {
	if (enterMenu) {
		mp3.playTrackFromDir(sound, 1);

#ifdef LS_INFO
		switch (sound) {
		case 4:
#ifdef LEDSTRINGS
			lightOff(ledPins);
			analogWrite(ledPins[0], MAX_BRIGHTNESS);
#endif
			Serial.print(F("VOLUME\nCur:"));
			Serial.println(storage.volume);
			value = storage.volume;
			break;
		case 5:
#ifdef LEDSTRINGS
			lightOff(ledPins);
			analogWrite(ledPins[1], MAX_BRIGHTNESS);
#endif
			Serial.print(F("SOUNDFONT\nCur:"));
			Serial.println(soundFont.getFolder());
			value = soundFont.getFolder();
			break;
		case 6:
#ifdef LEDSTRINGS
			lightOff(ledPins);
			analogWrite(ledPins[2], MAX_BRIGHTNESS);
#endif
			Serial.print(F("SWING\nCur:"));
			Serial.println(storage.swingTreshold);
			value = storage.swingTreshold;
			break;
		case 7:
#ifdef LEDSTRINGS
			lightOff(ledPins);
			analogWrite(ledPins[3], MAX_BRIGHTNESS);
#endif
			Serial.print(F("CLASH1\nCur:"));
			Serial.println(storage.clashAccelTreshold);
			value = storage.clashAccelTreshold;
			break;
		case 8:
#ifdef LEDSTRINGS
			lightOff(ledPins);
			analogWrite(ledPins[4], MAX_BRIGHTNESS);
#endif
			Serial.print(F("CLASH2\nCur:"));
			Serial.println(storage.clashBrakeTreshold);
			value = storage.clashBrakeTreshold;
			break;
#ifdef LUXEON
			case 9:
			lightOff(ledPins, false);
			Serial.print(F("COLOR1\nCur:"));
			Serial.println(storage.mainColor);
			lightChangeColor(storage.mainColor, true);
			value=storage.mainColor;
			break;
			case 10:
			lightOff(ledPins, false);
			Serial.print(F("COLOR2\nCur:"));
			Serial.println(storage.clashColor);
			lightChangeColor(storage.clashColor, true);
			value=storage.clashColor;
			break;
			case 11:
			lightOff(ledPins, false);
			Serial.println(F("SAVE TO SOUNDFONT?\nMain :Yes/Lockup: No"));
			break;
#endif
		}
#endif
		enterMenu = false;
		delay(100);
	}
} //confMenuStart

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
// ===             			DFPLAYER MANIPULATION FUNCTIONS	            			===
// ====================================================================================
inline void handle_interrupt() {
	if (mp3.getSerial()->getActiveObject()) {
		mp3.getSerial()->getActiveObject()->recv();
	}

	if (not repeat and not relaunch and not mp3.isQuerying()) {
		/*
		 * Last soundfile is over and has stopped :
		 * We relaunch the hum !
		 */
		while (mp3.getSerial()->available()
				and mp3.getSerial()->available() % DFPLAYER_BUFFER_LENGTH == 0) {
			mp3.receive();
			if (mp3.getRecvBuffer()[3] == TF_END_PLAY
					and (int) lastPlayed
							== (256 * mp3.getRecvBuffer()[5]
									+ mp3.getRecvBuffer()[6])) {
				// we don't want to receive the track number right now:
				// we want to leave this function as soon as possible !:
				mp3.playTrackFromDir(soundFont.getHum(), soundFont.getFolder());
				repeat = true;
				relaunch = true;
			}
		}
	}

}				//handle_interrupt

#if defined(PCINT0_vect)
ISR(PCINT0_vect) {
	handle_interrupt();
}
#endif

#if defined(PCINT1_vect)
ISR(PCINT1_vect, ISR_ALIASOF(PCINT0_vect));
#endif

#if defined(PCINT2_vect)
ISR(PCINT2_vect, ISR_ALIASOF(PCINT0_vect));
#endif

#if defined(PCINT3_vect)
ISR(PCINT3_vect, ISR_ALIASOF(PCINT0_vect));
#endif

