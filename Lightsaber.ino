/*
 * LightSaberOS V1.0RC2
 * author: 		Sébastien CAPOU (neskweek@gmail.com)
 * Source : 	https://github.com/neskweek/LightSaberOS.
 * Date: 		2016-02-14
 * Description:	Operating System for Arduino based LightSaber
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-sa/4.0/.
 */

#include <Arduino.h>
#include <DFPlayer_SoftwareSerial.h>
#include <I2Cdev.h>
#include <MPU6050_6Axis_MotionApps20.h>
#include <SoftwareSerial.h>
#include <EEPROMex.h>
#include <OneButton.h>
#include <TimerOne.h>
#include "SoundFont.h"
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
#include <Wire.h>
#endif

/*
 * DO NOT MODIFY
 */
#define CONFIG_VERSION 		"LS01"
#define MEMORYBASE 			32

#define LEDSTRINGS  // RGB LED USERS: Comment this line
#ifndef LEDSTRINGS
#define LUXEON
#endif

/*
 * DEFINE PINS
 * Modify to match your project setup
 * DO NOT USE PINS:
 * A4 Reserved to MPU6050 SDA
 * A5 Reserved to MPU6050 SCL
 * D2 Reserved to MPU6050 interrupts
 *
 * Spare :
 * A2,13,D11
 */

#define LEDSTRING1 			3
#define LEDSTRING2 			5
#define LEDSTRING3 			6
#define LEDSTRING4 			9
#define LEDSTRING5 			10
#define LEDSTRING6 			11

#ifdef LUXEON
#define COLORS		 		54   // Number of RGB (not RGBW) colors in our array. Do we need that much ?
#define LED_RED 			3
#define LED_GREEN 			5
#define LED_BLUE 			6
//#define LED_WHITE 			9   //Not used right now

#define BLASTER_FLASH_TIME  20
#define CLASH_BLINK_TIME  100
#endif

#define DFPLAYER_RX			8
#define DFPLAYER_TX			7
#define SPK1				A0
#define SPK2				A1
#define MAIN_BUTTON			12
#define LOCKUP_BUTTON		4

/*
 * DEFAULT CONF PARAMETERS
 * Will be overriden by EEPROM settings once the first
 * save will be done
 */
#define VOL					13
#define SOUNDFONT 			2
#define	SWING 				850
#define	CLASH_ACCEL 		9000
#define	CLASH_BRAKE 		4500

/*
 * OTHER PARAMETERS
 */
#define CLICK				5
#define PRESS_ACTION		150
#define PRESS_CONFIG		500
#define SWING_SUPPRESS 		110
#define CLASH_SUPPRESS 		60
#define CLASH_PASSES 		10
/* MAX_BRIGHTNESS
 * Maximum output voltage to apply to LEDS
 * Default = 200 (78,4%) Max=255 Min=0(Off)
 * WARNING ! A too high value may burn your leds. Please make your maths !
 */
#define MAX_BRIGHTNESS		200

/* For daily use I recommend you comment LS_INFO
 * When you plug your device to USB uncomment LS_INFO !
 */
#define LS_INFO
//#define LS_MOTION_DEBUG
//#define LS_BUTTON_DEBUG
//#define LS_CLASH_DEBUG
//#define LS_SWING_DEBUG
//#define LS_RELAUNCH_DEBUG

/*
 * Motion detection
 */
MPU6050 mpu;
// MPU control/status vars
volatile bool mpuInterrupt = false; // indicates whether MPU interrupt pin has gone high
bool dmpReady = false;  // set true if DMP init was successful
uint8_t mpuIntStatus;   // holds actual interrupt status byte from MPU
uint8_t devStatus; // return status after each device operation (0 = success, !0 = error)
uint16_t packetSize;    // expected DMP packet size (default is 42 bytes)
uint16_t mpuFifoCount;     // count of all bytes currently in FIFO
uint8_t fifoBuffer[64]; // FIFO storage buffer
// orientation/motion vars
Quaternion quaternion;           // [w, x, y, z]         quaternion container
Quaternion quaternion_last;         // [w, x, y, z]         quaternion container
Quaternion quaternion_reading;      // [w, x, y, z]         quaternion container
VectorInt16 aa;         // [x, y, z]            accel sensor measurements
VectorInt16 aaReal; // [x, y, z]            gravity-free accel sensor measurements
VectorInt16 aaWorld; // [x, y, z]            world-frame accel sensor measurements
VectorInt16 aaWorld_last; // [x, y, z]            world-frame accel sensor measurements
VectorInt16 aaWorld_reading; // [x, y, z]            world-frame accel sensor measurements
VectorFloat gravity;    // [x, y, z]            gravity vector
int initClash = 0;
long magnitude = 0;
bool isBigAcceleration = false;
bool isBigBrake = false;

/*
 * LED String variables
 */
#ifdef LEDSTRINGS
int ledPins[] = {LEDSTRING1, LEDSTRING2, LEDSTRING3, LEDSTRING4, LEDSTRING5,
	LEDSTRING6};
#endif
#ifdef LUXEON
int ledPins[] = { LED_RED, LED_GREEN, LED_BLUE /*,LED_WHITE*/};

// byte color => {R,G,B,ColorNumber}
byte currentColor[4];
const int rgbFactor = 100;
#endif
int brightness = 0;    // how bright the LED is

/*
 * Buttons variables
 */
OneButton mainButton(MAIN_BUTTON, true);
OneButton lockupButton(LOCKUP_BUTTON, true);

bool actionMode = false; // Play with your saber
bool configMode = false; // Play with your saber
bool ignition = false;
bool browsing = false;
int modification = 0;
#ifdef LUXEON
int blaster = -1;
int lockupBlink = 0;
#endif
/*
 * DFPLAYER variables
 */
DFPlayer mp3;
SoundFont soundFont;
int lastPlayed = 0;
//int lastPlayed2 = 0;
long value = 0;
long lastValue = 0;
bool repeat = false;
bool changePlayMode = false;
long swingSuppress = 1;
long clashSuppress = 1;
/*
 * ConfigMode Variables
 */
int menu = 0;
bool enterMenu = false;
bool changeMenu = false;
//bool ok = true;
int configAdress = 0;
bool play = false;
#ifdef LEDSTRINGS
struct StoreStruct {
	// This is for mere detection if they are our settings
	char version[5];
	// The settings
	int volume;// 0 to 30
	int soundFont;// as many as Sound font you have defined in Soundfont.h
	long swingTreshold;// treshold acceleration for Swing
	long clashAccelTreshold;// treshold acceleration for Swing
	long clashBrakeTreshold;// treshold acceleration for Swing
}storage;
#endif
#ifdef LUXEON
struct StoreStruct {
	// This is for mere detection if they are our settings
	char version[5];
	// The settings
	int volume; // 0 to 30
	int soundFont; // as many as Sound font you have defined in Soundfont.h
	long swingTreshold; // treshold acceleration for Swing
	long clashAccelTreshold; // treshold acceleration for Swing
	long clashBrakeTreshold; // treshold acceleration for Swing
	byte mainColor[4];
	byte clashColor[4];
	byte soundFontColorPreset[SOUNDFONT_QUANTITY + 2][2];
} storage;
#endif

// ================================================================
// ===               	   SETUP ROUTINE  	 	                ===
// ================================================================
void setup() {

	// join I2C bus (I2Cdev library doesn't do this automatically)
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
	Wire.begin();
	TWBR = 24; // 400kHz I2C clock (200kHz if CPU is 8MHz). Comment this line if having compilation difficulties with TWBR.
#elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
			Fastwire::setup(400, true);
#endif

//#ifdef LS_INFO
	// Serial line for debug
	Serial.begin(115200);
//#endif
	/***** LOAD CONFIG *****/
	// Get config from EEPROM if there is one
	// or initialise value with default ones set in StoreStruct
	EEPROM.setMemPool(MEMORYBASE, EEPROMSizeATmega328); //Set memorypool base to 32, assume Arduino Uno board
	configAdress = EEPROM.getAddress(sizeof(StoreStruct)); // Size of config object

	if (!loadConfig()) {
		for (int i = 0; i <= 5; i++)
			storage.version[i] = CONFIG_VERSION[i];
		storage.soundFont = SOUNDFONT;
		storage.volume = VOL;
		storage.swingTreshold = SWING;
		storage.clashAccelTreshold = CLASH_ACCEL;
		storage.clashBrakeTreshold = CLASH_BRAKE;
#ifdef LUXEON
		storage.mainColor[0] = 0;
		storage.mainColor[1] = 100;
		storage.mainColor[2] = 0;
		storage.mainColor[3] = 8;
		storage.clashColor[0] = 100;
		storage.clashColor[1] = 0;
		storage.clashColor[2] = 0;
		storage.clashColor[3] = 0;
		storage.soundFontColorPreset[2][0] = 8;
		storage.soundFontColorPreset[2][1] = 0;
		storage.soundFontColorPreset[3][0] = 0;
		storage.soundFontColorPreset[3][1] = 16;
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
	 * Those offsets are unique to each MPU6050 device.
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
	// For "security" we shutoff all pins wearing leds
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
	//setup finished. Boot ready
	mp3.playTrackFromDir(16, 1);
}

// ================================================================
// ===               	   LOOP ROUTINE  	 	                ===
// ================================================================
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
	 */
	if (actionMode) {




		/*
		 Serial.print(F("Action Mode");
		 Serial.print(F(" time=");
		 Serial.println(millis());
		 */
#ifdef LUXEON
		if (blaster >= 0) {
			blaster--;
			if (blaster == 0) {
				lightChangeColor(storage.mainColor, false);
			}
		}
#endif

		if (!ignition) {
			/*
			 *  This is the very first loop after Action Mode has been turned on
			 */

			// Reduce lockup trigger time
			lockupButton.setPressTicks(PRESS_ACTION);
#ifdef LS_INFO
			Serial.println(F("START ACTION"));
#endif
			//Play powerons wavs
			lastPlayed = mp3.playTrackFromDir(soundFont.getPowerOn(),
					soundFont.getFolder());

			// Light up the ledstrings
			lightOn(ledPins);

			// Get the initial position of the motion detector
			motionEngine();

			ignition = true;
			repeat = false;
			changePlayMode = false;
			isBigAcceleration = false;
			isBigBrake = false;
		}

		// Do we want next soundfile to be play continuously ?
		if (changePlayMode) {
			mp3.setSingleLoop(repeat);
			changePlayMode = false;
		}

		// Amount of brightness to fade
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
		motionEngine();

		isBigAcceleration = magnitude > storage.clashAccelTreshold;
		isBigBrake = magnitude <= storage.clashBrakeTreshold;

		if (abs(quaternion.w) > storage.swingTreshold and !swingSuppress) {
			/*
			 *  THIS IS A SWING !
			 *  A swing is a "violent" change angle orientation of the accelerometer
			 */

			swingSuppress++;
			lastPlayed = mp3.playTrackFromDir(soundFont.getSwing(),
					soundFont.getFolder());
			repeat = false;
			changePlayMode = true;
#ifdef LS_SWING_DEBUG
			Serial.print(F("SWING !!!\tWav played="));
			Serial.print(lastPlayed);

			printMagnitude(magnitude);
			printAcceleration(aaWorld);
			Serial.println(F(""));
#endif

		} else {
			if (swingSuppress > 0 and swingSuppress < SWING_SUPPRESS) {
				swingSuppress++;
			} else if (swingSuppress == SWING_SUPPRESS) {
				swingSuppress = 0;
			}
		}

		/*
		 * CLASH DETECTION :
		 * A clash is a violent deceleration followed by an "ALMOST" full stop (violent brake)
		 * Since the accelerometer doesn't detect instantaneously the "ALMOST"
		 * full stop we need to test it on several passes
		 */
		if ((initClash == 0 or initClash <= CLASH_PASSES) and isBigAcceleration
				and not clashSuppress) {
			/*
			 * This might be a Clash ! => Violent Acceleration or Deceleration
			 */
			initClash++;
#ifdef LS_CLASH_DEBUG
			Serial.print(F("INIT Clash Pass "));
			Serial.print(initClash);
#endif
#ifdef LS_CLASH_DEBUG and LS_MOTION_DEBUG
			printMagnitude(magnitude);
			printAcceleration(aaWorld);
#endif
#ifdef LS_CLASH_DEBUG
			Serial.println(F(""));
#endif
		} else if (initClash > 1 and initClash <= CLASH_PASSES and isBigBrake
				and not clashSuppress) {
			/*
			 * THIS IS A CLASH  ! => Violent brake !
			 */
			swingSuppress = 1;

			lastPlayed = mp3.playTrackFromDir(soundFont.getClash(),
					soundFont.getFolder());
			repeat = false;
			changePlayMode = true;
			initClash = 0;
#ifdef LS_CLASH_DEBUG
			Serial.print(F("CLASH !!!!\tWav played="));
			Serial.print(lastPlayed);
#endif
#ifdef LS_CLASH_DEBUG and LS_MOTION_DEBUG
			printMagnitude(magnitude);
			printAcceleration(aaWorld);
#endif
#ifdef LS_CLASH_DEBUG
			Serial.println(F("");
#endif
		} else {
			/*
			 * No movement recorded !
			 * Time to depop some variables and to check for
			 * Hum relaunch
			 */

			if ((isBigAcceleration and initClash > CLASH_PASSES)
					or (initClash > 0 and initClash <= CLASH_PASSES
							and not isBigBrake)
					or (initClash > 0 and initClash <= CLASH_PASSES
							and not isBigAcceleration)) {

				/*
				 * THAT WAS NOT A CLASH !!!
				 * Depoping variables trigger
				 */
				initClash = 0;
				isBigAcceleration = false;
				isBigBrake = false;
#ifdef LS_HEAVY_DEBUG
				Serial.print(F("NOT A CLASH !!!!"));
#endif
#ifdef LS_CLASH_DEBUG and LS_MOTION_DEBUG
				printMagnitude(magnitude);
				printAcceleration(aaWorld);
#endif
#ifdef LS_CLASH_DEBUG
				Serial.println(F(""));
#endif
			}
			/*
			 if (verboseprint) {
			 Serial.println(F("No activity, continue hum"));
			 }
			 */
#ifdef LS_RELAUNCH_DEBUG
			Serial.print(F("mp3 fifoCount="));
			Serial.println(mp3.getFifoCount());
#endif
			if (clashSuppress > 0 and clashSuppress < CLASH_SUPPRESS) {
				clashSuppress++;
			} else if (clashSuppress == CLASH_SUPPRESS) {
				clashSuppress = 0;
			}
#ifdef LS_SWING_DEBUG
			if (swingSuppress) {
				Serial.print(F("swingSuppress= "));
				Serial.print(swingSuppress);
			}
#endif
#ifdef LS_CLASH_DEBUG
			if (clashSuppress) {
				Serial.print(F("clashSuppress= "));
				Serial.print(clashSuppress);
			}
#endif
#ifdef LS_CLASH_DEBUG or LS_SWING_DEBUG
			if(swingSuppress or clashSuppress) {
				Serial.println(F(""));
			}
#endif
			if (!repeat) {
				/*
				 * Last sound played was triggered by an action (swing, clash or pressed button)
				 * So we want to chek if we need to relaunch a hum
				 */
				mp3.receive();

#ifdef LS_RELAUNCH_DEBUG
				Serial.print(F("We may need to relaunch hum sound ! "));
				Serial.print(millis());
				Serial.print(F("ms mp3 fifoCount="));
				Serial.print(mp3.getFifoCount());
				Serial.println(F(" buffer="));

				if (lastPlayed != lastPlayed2) {
					Serial.print(F("lastPlayed="));
					Serial.print(lastPlayed);
					Serial.print(F("\trepeat="));
					Serial.println(repeat);
					lastPlayed2 = lastPlayed;
				}

#endif

				int track = 256 * mp3.getRecvBuffer()[5]
						+ mp3.getRecvBuffer()[6];

				if (mp3.getRecvBuffer()[3] == TF_END_PLAY
						and track == lastPlayed) {
					/*
					 * Last soundfile is over and has stopped :
					 * We relaunch the hum !
					 */
#ifdef LUXEON
					lightChangeColor(storage.mainColor, true);
#endif
#ifdef LS_RELAUNCH_DEBUG
					Serial.print(F("MP3 stopped :"));
					Serial.print(millis());
					Serial.print(F("\tlastPlayed="));
					Serial.print(lastPlayed);
					Serial.print(F("\ttrack="));
					Serial.println(track);
#endif
					lastPlayed = mp3.playTrackFromDir(soundFont.getHum(),
							soundFont.getFolder());
					repeat = true;
					changePlayMode = true;
#ifdef LS_RELAUNCH_DEBUG
					Serial.print(F("MP3 Relaunched :"));
					Serial.println(millis());
#endif
					clashSuppress = 0;
					swingSuppress = 0;
				}
#ifdef LS_RELAUNCH_DEBUG
				else {
					if (mp3.getRecvBuffer()[3] == TF_END_PLAY) {
						Serial.print(F("command="));
						Serial.print(mp3.getRecvBuffer()[3], HEX);
						Serial.print(F("\ttrack="));
						Serial.println(track);
					}
				}
#endif
			}
		}
		// ************************* blade movement detection ends***********************************
	}		//END ACTION MODE HANDLER

	/*
	 * CONFIG MODE HANDLER
	 */
	else if (configMode) {

#ifdef LUXEON
		byte rgbArray[COLORS][3] = {
				// RGB Array 3 wide 54 tall, stores RGB values
				// (There's gotta be a prettier way to do this!)
				{ 100, 0, 0 }, /*Red*/
				{ 100, 15, 0 }, { 100, 40, 0 }, { 100, 60, 0 }, { 100, 100, 0 },
				{ 60, 100, 0 }, { 40, 100, 0 }, { 15, 100, 0 },
				{ 0, 100, 0 } /*Green*/, { 0, 100, 15 }, { 0, 100, 40 }, { 0,
						100, 60 }, { 0, 100, 100 }, { 0, 60, 100 },
				{ 0, 40, 100 }, { 0, 15, 100 }, { 0, 0, 100 }, /*Blue*/
				{ 15, 0, 100 }, { 40, 0, 100 }, { 60, 0, 100 }, { 100, 0, 100 },
				{ 100, 0, 60 }, { 100, 0, 40 }, { 100, 0, 15 },
				//Hue one, a lighter starts at Array pointer 40100
				{ 100, 15, 15 }, { 100, 40, 15 }, { 100, 60, 15 }, { 100, 100,
						15 }, { 60, 100, 15 }, { 40, 100, 15 }, { 15, 100, 15 },
				{ 15, 100, 40 }, { 15, 100, 60 }, { 15, 100, 100 }, { 15, 60,
						100 }, { 15, 40, 100 }, { 15, 15, 100 },
				{ 40, 15, 100 }, { 60, 15, 100 }, { 100, 15, 100 }, { 100, 15,
						60 }, { 100, 15, 40 },
				//Hue two, Lightest starts at Array pointer 10040
				{ 100, 40, 40 }, { 100, 60, 40 }, { 100, 100, 40 }, { 60, 100,
						40 }, { 40, 100, 40 }, { 40, 100, 60 },
				{ 40, 100, 100 }, { 40, 60, 100 }, { 40, 40, 100 }, { 60, 40,
						100 }, { 100, 40, 100 }, { 100, 40, 60 }, };
#endif

		if (!browsing) {
			mp3.playTrackFromDir(3, 1/*,false*/);
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
			mp3.playTrackFromDir(2, 1/*, false*/);
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

			if (value != lastValue) {
				storage.volume = value;
				mp3.setVolume(storage.volume);
				delay(10);
#ifdef LS_INFO
				Serial.println(storage.volume);
#endif
			}

			break;
		case 1:
			confMenuStart(5);

			confParseValue(storage.soundFont, 2, SOUNDFONT_QUANTITY + 1, 1);

			if (value != lastValue) {
				storage.soundFont = value;
				soundFont.setFolder(value);
				mp3.playTrackFromDir(soundFont.getBoot(),
						soundFont.getFolder()/*, false*/);
				delay(500);
#ifdef LUXEON
				storage.mainColor[3] = storage.soundFontColorPreset[value][0];
				storage.mainColor[0] = rgbArray[storage.mainColor[3]][0];
				storage.mainColor[1] = rgbArray[storage.mainColor[3]][1];
				storage.mainColor[2] = rgbArray[storage.mainColor[3]][2];

				storage.clashColor[3] = storage.soundFontColorPreset[value][1];
				storage.clashColor[0] = rgbArray[storage.clashColor[3]][0];
				storage.clashColor[1] = rgbArray[storage.clashColor[3]][1];
				storage.clashColor[2] = rgbArray[storage.clashColor[3]][2];
#endif
#ifdef LS_INFO
				Serial.println(soundFont.getFolder());
#endif
			}
			break;
#ifdef LUXEON
		case 2:
			confMenuStart(9);

			confParseValue(storage.mainColor[3], 0, COLORS, 1);

			if (value != lastValue) {
				storage.mainColor[0] = rgbArray[value][0];
				storage.mainColor[1] = rgbArray[value][1];
				storage.mainColor[2] = rgbArray[value][2];
				storage.mainColor[3] = value;
				lightChangeColor(storage.mainColor, true);
#ifdef LS_INFO
				Serial.println(storage.mainColor[3]);
#endif
			}
			break;
		case 3:
			confMenuStart(10);

			confParseValue(storage.clashColor[3], 0, COLORS, 1);

			if (value != lastValue) {
				storage.clashColor[0] = rgbArray[value][0];
				storage.clashColor[1] = rgbArray[value][1];
				storage.clashColor[2] = rgbArray[value][2];
				storage.clashColor[3] = value;
				lightChangeColor(storage.clashColor, true);
#ifdef LS_INFO
				Serial.println(storage.clashColor[3]);
#endif
			}
			break;
		case 4:
			confMenuStart(11);

			if (modification > 0) {
				//Yes
				//We save color values to this Soundfount
				Serial.println(F("Yes"));
				mp3.playTrackFromDir(12, 1/*, false*/);
				storage.soundFontColorPreset[storage.soundFont][0] =
						storage.mainColor[3];
				storage.soundFontColorPreset[storage.soundFont][1] =
						storage.clashColor[3];
				menu++;
				changeMenu = true;
				enterMenu = true;
				delay(500);
			} else if (modification < 0) {
				//No
				// we do nothing and leave this menu
				Serial.println(F("No"));
				mp3.playTrackFromDir(13, 1/*, false*/);
				menu++;
				changeMenu = true;
				enterMenu = true;
				delay(20);
			}
			modification = 0;
			break;
#endif
		case 5:
			confMenuStart(6);

			confParseValue(storage.swingTreshold, 500, 2000, 100);

			if (value != lastValue) {
				storage.swingTreshold = value;
#ifdef LS_INFO
				Serial.println(storage.swingTreshold);
#endif
			}
			break;
		case 6:
			confMenuStart(7);

			confParseValue(storage.clashAccelTreshold, 8000, 15000, 250);

			if (value != lastValue) {
				storage.clashAccelTreshold = value;
#ifdef LS_INFO
				Serial.println(storage.clashAccelTreshold);
#endif
			}
			break;
		case 7:
			confMenuStart(8);

			confParseValue(storage.clashBrakeTreshold, 1000, 5000, 250);

			if (value != lastValue) {
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
	/*
	 * STANDBY MODE
	 */
	else if (!actionMode && !configMode) {  // circuit is OFF

		if (ignition) { // Leaving Action Mode
			repeat = false;
#ifdef LS_INFO
			Serial.println(F("END ACTION"));
#endif
			lockupButton.setPressTicks(PRESS_CONFIG);
			lastPlayed = mp3.playTrackFromDir(soundFont.getPowerOff(),
					soundFont.getFolder());
			changeMenu = false;
			isBigAcceleration = false;

			lightOff(ledPins);
			mp3.setSingleLoop(repeat);
			ignition = false;
			lastPlayed = 0;
			modification = 0;

		}
		if (browsing) { // Leaving Config Mode
			saveConfig();

			//RESET CONFIG
			/*
			 for (int i = 0; i < EEPROMSizeATmega328; i++) {
			 //			 if (EEPROM.read(i) != 0) {
			 EEPROM.update(i, 0);
			 //			 }
			 }
			 */
			mp3.playTrackFromDir(3, 1);
			browsing = false;
			enterMenu = false;
			lastPlayed = 0;
			modification = 0;
			lightOff(ledPins);
			menu = 0;
#ifdef LUXEON
			lightChangeColor(storage.mainColor, false);
#endif
#ifdef LS_INFO
			Serial.println(F("END CONF"));
#endif
		}

	} // END STANDBY MODE
} //loop

// ================================================================
// ===               BUTTONS CALLBACK FUNCTIONS                 ===
// ================================================================

void mainClick() {
#ifdef LS_BUTTON_DEBUG
	Serial.println("Main button click.");
#endif
	if (actionMode) {
		/*
		 * ACTION TO DEFINE
		 */
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

void mainDoubleClick() {
#ifdef LS_BUTTON_DEBUG
	Serial.println("Main button double click.");
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
} // mainDoubleClick

void mainLongPressStart() {
#ifdef LS_BUTTON_DEBUG
	Serial.println("Main button longPress start");
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

void mainLongPress() {
#ifdef LS_BUTTON_DEBUG
	Serial.println("Main button longPress...");
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

void mainLongPressStop() {
#ifdef LS_BUTTON_DEBUG
	Serial.println("Main button longPress stop");
#endif
	if (!configMode && !actionMode) {
		/*
		 * ACTION TO DEFINE
		 */
	}
} // mainLongPressStop

void lockupClick() {
#ifdef LS_BUTTON_DEBUG
	Serial.println("Lockup button click.");
#endif
	if (actionMode) {
		// Blaster
#ifdef LEDSTRINGS
		analogWrite(ledPins[random(6)], LOW); //momentary shut off one led segment
#endif
#ifdef LUXEON
		lightChangeColor(storage.clashColor, false);
		blaster = BLASTER_FLASH_TIME;
#endif
		if (soundFont.getBlaster()) {
			// Some Soundfont may not have Blaster sounds
			lastPlayed = mp3.playTrackFromDir(soundFont.getBlaster(),
					soundFont.getFolder());
		} else {
			Serial.println(soundFont.getBlaster());
		}
		repeat = false;
		changePlayMode = true;
		initClash = 0;
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

void lockupDoubleClick() {
#ifdef LS_BUTTON_DEBUG
	Serial.println("Lockup button double click.");
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

void lockupLongPressStart() {
#ifdef LS_BUTTON_DEBUG
	Serial.println("Lockup button longPress start");
#endif
	if (actionMode) {
		//Lockup Start
#ifdef LUXEON
		lightChangeColor(storage.clashColor, true);
		lockupBlink = random(CLASH_BLINK_TIME);
#endif
		//		Serial.println(soundFont.getLockup());
		//		if (soundFont.getLockup()) {
		lastPlayed = mp3.playTrackFromDir(soundFont.getLockup(),
				soundFont.getFolder());
		//		}
		repeat = true;
		changePlayMode = true;
		initClash = 0;
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

void lockupLongPress() {
#ifdef LS_BUTTON_DEBUG
	Serial.println("Lockup button longPress...");
#endif
	if (actionMode) {
#ifdef LUXEON
		//
		if (lockupBlink > 0) {
			lockupBlink--;
		} else {
			if (currentColor[3] == storage.clashColor[3]) {
				lockupBlink = random(CLASH_BLINK_TIME);
				lightChangeColor(storage.mainColor, false);
			} else if (currentColor[3] == storage.mainColor[3]) {
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

void lockupLongPressStop() {
#ifdef LS_BUTTON_DEBUG
	Serial.println("Lockup button longPress stop");
#endif
	if (actionMode) {
		//Lockup Stop
#ifdef LUXEON
		lightChangeColor(storage.mainColor, false);
		lockupBlink = 0;
#endif
		lastPlayed = mp3.playTrackFromDir(4, soundFont.getFolder());
		repeat = true;
		changePlayMode = true;
		clashSuppress = 0;
		swingSuppress = 0;
		initClash = 0;

	}
} // lockupLongPressStop

// ================================================================
// ===              	    LED FUNCTIONS		                ===
// ================================================================

#ifdef LEDSTRINGS
void lightOn(int ledPins[]) {

// Light up the ledstrings
	for (int i = 0; i <= 5; i++) {
		digitalWrite(ledPins[i], HIGH);
		if (i < 5) {
			delay(83);
		}
	}

}				//lightOn

void lightOff(int ledPins[]) {
// Light off the ledstrings
	for (int i = 5; i >= 0; i--) {
		digitalWrite(ledPins[i], LOW);
		if (i > 0) {
			delay(83);
		}
	}
}				//lightOff

void lightFlicker(int ledPins[], int value) {
// lightFlicker the ledstrings
	for (int i = 5; i >= 0; i--) {
		analogWrite(ledPins[i], value);
	}
} //lightFlicker
#endif

#ifdef LUXEON
void ledOff(int ledPins[]) {
// Light off the leds
	for (int i = 0; i <= 2; i++) {
		digitalWrite(ledPins[i], LOW);
	}
} //ledOff

void lightOn(int ledPins[]) {

// Light up the leds
	for (int fadeIn = 255; fadeIn > 0; fadeIn--) {
		for (int i = 0; i <= 2; i++) {
			analogWrite(ledPins[i],
					(MAX_BRIGHTNESS / fadeIn) * currentColor[i] / 100);
		}
		delay(2);
	}
} //lightOn

void lightOff(int ledPins[]) {
// Light off the leds
// Fade out
	for (int fadeOut = 255; fadeOut >= 1; fadeOut--) {
		for (int i = 2; i >= 0; i--) {
			analogWrite(ledPins[i],
					(MAX_BRIGHTNESS - (MAX_BRIGHTNESS / fadeOut))
							* currentColor[i] / 100);
		}
		delay(2);
	}
// shut Off
	for (int i = 0; i <= 2; i++) {
		analogWrite(ledPins[i], LOW);
	}
} //lightOff

void lightFlicker(int ledPins[], int value) {
// lightFlicker the leds
	for (int i = 0; i < 2; i++) {
		analogWrite(ledPins[i], value * currentColor[i] / rgbFactor);
	}
} //lightFlicker

void lightChangeColor(byte color[], bool lightup) {
// lightFlicker the leds
	for (int i = 0; i <= 3; i++) {
		currentColor[i] = color[i];
		if (lightup) {
			analogWrite(ledPins[i], MAX_BRIGHTNESS * currentColor[i] / 100);
		}
	}
} //lightFlicker
#endif

// ================================================================
// ===           	  MOTION DETECTION FUNCTIONS	            ===
// ================================================================
void motionEngine() {
	long multiplier = 100000;
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
#ifdef LS_MOTION_DEBUG
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

void dmpDataReady() {
	mpuInterrupt = true;
}

#ifdef LS_MOTION_DEBUG
void printQuaternion(Quaternion quaternion, long multiplier = 100000) {
	Serial.print(F("\t\tQuaternion\t\tw="));
	Serial.print(quaternion_reading.w * multiplier);
	Serial.print(F("\t\tx="));
	Serial.print(quaternion_reading.x * multiplier);
	Serial.print(F("\t\ty="));
	Serial.print(quaternion_reading.y * multiplier);
	Serial.print(F("\t\tz="));
	Serial.println(quaternion_reading.z * multiplier);
}
void printAcceleration(VectorInt16 aaWorld) {
	Serial.print(F("\t\tAcceleration\t\tx="));
	Serial.print(aaWorld.x);
	Serial.print(F("\t\ty="));
	Serial.print(aaWorld.y);
	Serial.print(F("\t\tz="));
	Serial.println(aaWorld.z);
}
void printMagnitude(long magnitude) {
	Serial.print(F("\t\tMagnitude="));
	Serial.print(magnitude);
}
#endif

// ================================================================
// ===           	  	 CONFIG MODE FUNCTIONS	                ===
// ================================================================

void confParseValue(int variable, int min, int max, int multiplier) {
	lastValue = variable;
	value = variable + multiplier * modification;
	modification = 0;
	if (value < min) {
		value = max;
	} else if (value > max) {
		value = min;
	} else if (value == min and play) {
		play = false;
		mp3.playTrackFromDir(15, 1/*, false*/);
		delay(50);
	} else if (value == max and play) {
		play = false;
		mp3.playTrackFromDir(14, 1/*, false*/);
		delay(50);
	}
}

void confMenuStart(int sound) {
	if (enterMenu) {
		mp3.playTrackFromDir(sound, 1);
#ifdef LS_INFO
		switch (sound) {
		case 4:
			Serial.print(F("VOLUME\nCur:"));
			Serial.println(storage.volume);
			break;
		case 5:
			Serial.println(F("SOUNDFONT\nCur:"));
			Serial.println(soundFont.getFolder());
			break;
		case 6:
			Serial.print(F("SWING\nCur:"));
			Serial.println(storage.swingTreshold);
			break;
#ifdef LUXEON
		case 9:
			Serial.print(F("COLOR1\nCur:"));
			Serial.println(storage.mainColor[3]);
			break;
		case 10:
			Serial.print(F("COLOR2\nCur:"));
			Serial.println(storage.clashColor[3]);
			break;
		case 11:
			Serial.println(F("SAVE TO SOUNDFONT?\nMain :Yes/Lockup: No"));
			break;
#endif
		case 7:
			Serial.print(F("CLASH1\nCur:"));
			Serial.println(storage.clashAccelTreshold);
			break;
		case 8:
			Serial.print(F("CLASH2\nCur:"));
			Serial.println(storage.clashBrakeTreshold);
			break;
		}
#endif
		enterMenu = false;
		delay(500);
	}
}

// ================================================================
// ===           	  EEPROM MANIPULATION FUNCTIONS	            ===
// ================================================================

bool loadConfig() {
	bool equals = true;
	EEPROM.readBlock(configAdress, storage);
	for (int i = 0; i <= 4; i++) {
		if (storage.version[i] != CONFIG_VERSION[i]) {
			equals = false;
		}
	}
	return equals;
}

void saveConfig() {
	EEPROM.updateBlock(configAdress, storage);
}

