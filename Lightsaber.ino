/*
 * LightSaberOS V1.0RC1
 * author: 		Sébastien CAPOU (neskweek@gmail.com)
 * Source : 	https://github.com/neskweek/LightSaberOS.
 * Date: 		2016-02-11
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
#include "SoundFont.h"
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
#include <Wire.h>
#endif

#define CONFIG_VERSION "LS01"
#define VOL	13
#define SOUNDFONT 2
#define	SWING 750
#define	CLASH_ACCEL 9000
#define	CLASH_BRAKE 4000
#define SWING_SUPPRESS 110
#define CLASH_SUPPRESS 60
#define MEMORYBASE 32
#define SOUNDFONT_FOLDERS 3
#define CLASH_PASSES 10

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
int ledPins[] = { 3, 4, 5, 6, 7, 8 };
int brightness = 0;    // how bright the LED is

/*
 * Buttons variables
 */
bool actionMode = false; // Play with your saber
bool configMode = false; // Play with your saber
bool ignition = false;
bool browsing = false;
const int mainButtonPin = 9;
const int clashButtonPin = 10;

int mainButtonState = HIGH;        // the current readingMain from the input pin
int lastMainButtonState = HIGH;   // the previous readingMain from the input pin
int readingMain = HIGH;
unsigned long lastMainDebounceTime = 0; // the last time the output pin was toggled

int lockupButtonState = HIGH;      // the current readingMain from the input pin
int lastLockupButtonState = HIGH; // the previous readingMain from the input pin
int readingLockup = HIGH;
int lockupLastPlayed = 0;
unsigned long lastLockupDebounceTime; // the last time the output pin was toggled
bool lockupPressed = false;
bool mainPressed = false;
unsigned long startLockupPressing;

unsigned long debounceDelay = 5; // the debounce time; increase if the output flickers

/*
 * DFPLAYER variables
 */
DFPlayer mp3;
SoundFont soundFont;
int SPK1 = A0;
int SPK2 = A1;
int lastPlayed = 0;
int lastPlayed2 = 0;
bool repeat = false;
bool changePlayMode = false;
uint8_t hum[] = { 0x7E, 0xFF, 0x06, 0x0F, 0x00, 0x01, 0x05, 0xFE, 0xE6, 0xEF };
uint8_t last_send_buf[10] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF };
long swingSuppress = 1;
long clashSuppress = 1;
/*
 * ConfigMode Variables
 */
int menu = 0;
bool enterMenu = false;
bool changeMenu = false;
bool ok = true;
int configAdress = 0;
long lastValue = 0;
struct StoreStruct {
	// The settings
	int volume;						// 0 to 30
	int soundFont; 		// as many as Sound font you have defined in Soundfont.h
	long swingTreshold;				// treshold acceleration for Swing
	long clashAccelTreshold;		// treshold acceleration for Swing
	long clashBrakeTreshold;		// treshold acceleration for Swing
	// This is for mere detection if they are our settings
	char version[5];
} storage = { VOL, SOUNDFONT, SWING, CLASH_ACCEL, CLASH_BRAKE, CONFIG_VERSION };

void setup() {
	turnOff(ledPins);

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
		storage.soundFont = SOUNDFONT;
		storage.volume = VOL;
		storage.swingTreshold = SWING;
		storage.clashAccelTreshold = CLASH_ACCEL;
		storage.clashBrakeTreshold = CLASH_BRAKE;
		for (int i = 0; i <= 5; i++)
			storage.version[i] = CONFIG_VERSION[i];
#ifdef LS_INFO
		Serial.println(F("Config reverted to default value"));
#endif
	}
#ifdef LS_INFO
	else {
		Serial.println(F("Config Loaded"));
	}
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
	pinMode(ledPins[3], OUTPUT);
	pinMode(ledPins[4], OUTPUT);
	pinMode(ledPins[5], OUTPUT);
	/***** LED SEGMENT INITIALISATION  *****/

	/***** DF PLAYER INITIALISATION  *****/
	mp3.setSerial(12, 13);
	mp3.setVolume(storage.volume);
	pinMode(SPK1, INPUT);
	pinMode(SPK2, INPUT);
	soundFont.setFolder(storage.soundFont);
	/***** DF PLAYER INITIALISATION  *****/
	//setup finished. Boot ready
	lastPlayed = mp3.playTrackFromDir(soundFont.getBoot(),
			soundFont.getFolder());
}
/*
 *
 *
 *
 *
 *
 *
 *
 */

void loop() {

// if MPU6050 DMP programming failed, don't try to do anything : EPIC FAIL !
	if (!dmpReady) {
		return;
	}

// read the state of the pushbuttons value:
	readingMain = digitalRead(mainButtonPin);
	readingLockup = digitalRead(clashButtonPin);

	MainButton();
	LockupButton();

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

		if (!ignition) {
			/*
			 *  This is the very first loop after Action Mode has been turned on
			 */
#ifdef LS_INFO
			Serial.println(F("-----------------ENTERING ACTION MODE"));
#endif
			//Play powerons wavs
			lastPlayed = mp3.playTrackFromDir(soundFont.getPowerOn(),
					soundFont.getFolder());

			// Light up the ledstrings
			turnOn(ledPins);

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

		//brightness = random(128, 190);
		brightness = 200 - (abs(analogRead(SPK1) - analogRead(SPK2)));
#ifdef LS_HEAVY_DEBUG
		Serial.print(F("Brightness: "));
		Serial.print(brightness);
		Serial.print(F("   SPK1: "));
		Serial.print(analogRead(SPK1));
		Serial.print(F("   SPK2: "));
		Serial.println(analogRead(SPK2));
#endif
		flicker(ledPins, brightness);

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
#endif
			Serial.println(F(""));
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
			/*
			 if (swingSuppress > 0 and swingSuppress < SWING_SUPPRESS) {
			 swingSuppress++;
			 } else if (swingSuppress == SWING_SUPPRESS) {
			 swingSuppress = 0;
			 }
			 */
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
	}
	/*
	 * END ACTION MODE HANDLER
	 */

	/*
	 * CONFIG MODE HANDLER
	 */
	else if (configMode) {
		long value = 0;
		if (!browsing) {
			//mp3.playTrackFromDir(1, 3,false);
#ifdef LS_INFO
			Serial.println(F("-----------------ENTERING CONFIG MODE"));
#endif
			browsing = true;
		}

		if (lockupLastPlayed == -1 and !repeat) {
#ifdef LS_INFO
			Serial.print(F("Down:"));
#endif
			mp3.playTrackFromDir(2, 1, false);
			repeat = true;
		} else if (lockupLastPlayed == 1 and !repeat) {
#ifdef LS_INFO
			Serial.print(F("Up:"));
#endif
			mp3.playTrackFromDir(1, 1, false);
			repeat = true;
		}

		switch (menu) {
		case 0:
			if (enterMenu) {
				mp3.playTrackFromDir(4, 1, false);
				turnOff(ledPins);
				analogWrite(ledPins[0], 160);
#ifdef LS_INFO
				Serial.print(F("-------VOLUME-------\nCurrent Volume :"));
				Serial.println(storage.volume);
#endif
				enterMenu = false;
			}
			lastValue = storage.volume;
			value = storage.volume + lockupLastPlayed;
			lockupLastPlayed = 0;
			if (value < 0) {
				value = 30;
			} else if (value > 30) {
				value = 0;
			}
			if (value != lastValue) {
				storage.volume = value;
				mp3.setVolume(storage.volume);
#ifdef LS_INFO
				Serial.println(storage.volume);
#endif
			}
			break;
		case 1:
			if (enterMenu) {
				mp3.playTrackFromDir(5, 1, false);
				turnOff(ledPins);
				analogWrite(ledPins[1], 160);
#ifdef LS_INFO
				Serial.println(F("-----SOUNDFONT------\nCurrent value :"));
				Serial.println(soundFont.getFolder());
#endif
				enterMenu = false;
			}
			lastValue = soundFont.getFolder();
			value = soundFont.getFolder() + lockupLastPlayed;
			lockupLastPlayed = 0;
			if (value < 2) {
				value = SOUNDFONT_QUANTITY + 1;
			} else if (value > SOUNDFONT_QUANTITY + 1) {
				value = 2;
			}
			if (value != lastValue) {
				storage.soundFont = value;
				soundFont.setFolder(value);
				Serial.println(soundFont.getFolder());
			}
			break;
		case 2:
			if (enterMenu) {
				mp3.playTrackFromDir(6, 1, false);
				turnOff(ledPins);
				analogWrite(ledPins[2], 160);
#ifdef LS_INFO
				Serial.print(F("--SWING SENSITIVITY--\nCurrent value :"));
				Serial.print(storage.swingTreshold);
//				Serial.print(F(" Recommended value :"));
//				Serial.print(SWING);
//				Serial.println(F(" Range :500 <-> 2000 Step: 100"));
#endif
				enterMenu = false;
			}
			lastValue = storage.swingTreshold;
			value = storage.swingTreshold + 100 * lockupLastPlayed;
			lockupLastPlayed = 0;
			if (value < 500) {
				value = 2000;
			} else if (value > 2000) {
				value = 500;
			}
			if (value != lastValue) {
				storage.swingTreshold = value;
#ifdef LS_INFO
				Serial.println(storage.swingTreshold);
#endif
			}
			break;
		case 3:
			if (enterMenu) {
				mp3.playTrackFromDir(7, 1, false);
				turnOff(ledPins);
				analogWrite(ledPins[3], 160);
#ifdef LS_INFO
				Serial.print(
						F(
								"--CLASH  ACCELERATION SENSITIVITY--\nCurrent value :"));
				Serial.print(storage.clashAccelTreshold);
//				Serial.print(F(" Recommended value :"));
//				Serial.print(CLASH_ACCEL);
//				Serial.println(F(" Range :8000 <-> 15000 Step: 250"));
#endif
				enterMenu = false;
			}
			lastValue = storage.clashAccelTreshold;
			value = storage.clashAccelTreshold + 250 * lockupLastPlayed;
			lockupLastPlayed = 0;
			if (value < 8000) {
				value = 15000;
			} else if (value > 15000) {
				value = 8000;
			}
			if (value != lastValue) {
				storage.clashAccelTreshold = value;
#ifdef LS_INFO
				Serial.println(storage.clashAccelTreshold);
#endif
			}
			break;
		case 4:
			if (enterMenu) {
				mp3.playTrackFromDir(8, 1, false);
				turnOff(ledPins);
				analogWrite(ledPins[4], 160);
#ifdef LS_INFO
				Serial.print(
						F("--CLASH  BRAKE SENSITIVITY--\nCurrent value :"));
				Serial.print(storage.clashBrakeTreshold);
//				Serial.print(F(" Recommended value :"));
//				Serial.print(CLASH_BRAKE);
//				Serial.println(F(" Range :1000 <-> 5000 Step: 250"));
#endif
				enterMenu = false;
			}
			lastValue = storage.clashBrakeTreshold;
			value = storage.clashBrakeTreshold + 250 * lockupLastPlayed;
			lockupLastPlayed = 0;
			if (value < 1000) {
				value = 5000;
			} else if (value > 5000) {
				value = 1000;
			}
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

	}
	/*
	 * END CONFIG MODE HANDLER
	 */
	/*
	 * STANDBY MODE
	 */
	else if (!actionMode && !configMode) {  // circuit is OFF

		if (ignition) { // Leaving Action Mode
			repeat = false;
#ifdef LS_INFO
			Serial.println(F("-----------------LEAVING ACTION MODE"));
#endif
			lastPlayed = mp3.playTrackFromDir(soundFont.getPowerOff(),
					soundFont.getFolder());
			changeMenu = false;
			isBigAcceleration = false;

			turnOff(ledPins);
			mp3.setSingleLoop(repeat);
			ignition = false;
			lastPlayed = 0;
			lockupLastPlayed = 0;

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
			lockupLastPlayed = 0;
			turnOff(ledPins);
			menu = 0;
			configMode = false;
			repeat = false;
#ifdef LS_INFO
			Serial.println(F("-----------------LEAVING CONFIG MODE"));
#endif
		}

	}
	/*
	 * END STANDBY MODE
	 */

	lastMainButtonState = readingMain;
	lastLockupButtonState = readingLockup;

}

// ================================================================
// ===               INTERRUPT DETECTION ROUTINE                ===
// ================================================================

volatile bool mpuInterrupt = false; // indicates whether MPU interrupt pin has gone high
void dmpDataReady() {
	mpuInterrupt = true;
}

void MainButton() {
// check to see if you just lockupPressed the button
// (i.e. the input went from LOW to HIGH),  and you've waited
// long enough since the last press to ignore any noise:
	long time = millis();

// If the switch changed, due to noise or pressing:
	if (readingMain != lastMainButtonState) {
		// reset the debouncing timer
		lastMainDebounceTime = time;
	}
	unsigned long diff = time - lastMainDebounceTime;

	if (diff > debounceDelay and diff < debounceDelay + 10) {
		// whatever the readingMain is at, it's been there for longer
		// than the debounce delay, so take it as the actual current state:

		// if the button state has changed:
		if (readingMain != mainButtonState) {
			mainButtonState = readingMain;
			if (!configMode) {
				// only toggle the LED if the new button state is HIGH
				if (mainButtonState == LOW) {
#ifdef LS_BUTTON_DEBUG
					Serial.println(F("Main Button lockupPressed"));
#endif
					actionMode = !actionMode;
				}
			}
			if (configMode) {
				//ConfigMode or Down button ?
				if (mainButtonState == LOW and !mainPressed) {
#ifdef LS_BUTTON_DEBUG
					Serial.print(F("Main Short Press "));
					Serial.print(F(" time="));
					Serial.println(millis());
#endif
					mainPressed = true;
					repeat = true;
				}
				// Trigger Down button
				if (mainButtonState == HIGH and !changeMenu) {
#ifdef LS_BUTTON_DEBUG
					Serial.println(F("Main Short released"));
#endif
					mainPressed = false;
					lockupLastPlayed = 1;
					repeat = false;

				}
				// End Entering/Leaving config Mode
				if (mainButtonState == HIGH and changeMenu) {
#ifdef LS_BUTTON_DEBUG
					Serial.println(F("Main Long released"));
#endif
					mainPressed = false;
					changeMenu = false;
					repeat = true;
				}
			}
		}
	} else if (diff > debounceDelay + 300) {

		if (readingMain == mainButtonState) {
			if (configMode) {

				//Trigger Entering/Leaving ConfigMode or change menu in Config Mode
				if (mainButtonState == LOW and mainPressed and !changeMenu) {
#ifdef LS_BUTTON_DEBUG
					Serial.print(F("Main Long Press"));
					Serial.print(F("changePlayMode:"));
					Serial.println(changeMenu);
					Serial.print(F(" time="));
					Serial.println(millis());
#endif
					changeMenu = true;
					enterMenu = true;
					repeat = true;
					menu++;

				}
			}
		}
	}

}

void LockupButton() {
	int toPlay = 0;
	unsigned long time = millis();

// If the switch changed, due to noise or pressing:
	if (readingLockup != lastLockupButtonState) {
// reset the debouncing timer
		lastLockupDebounceTime = time;
	}

	unsigned long diff = time - lastLockupDebounceTime;

	if (diff > debounceDelay and diff < debounceDelay + 10) {

// if the button state has changed:
		if (readingLockup != lockupButtonState) {
			lockupButtonState = readingLockup;

			if (actionMode) {
				// Blaster or Lockup ?
				if (lockupButtonState == LOW and !lockupPressed) {
#ifdef LS_BUTTON_DEBUG
					Serial.print(F("Lockup Short Press "));
					Serial.print(F(" time="));
					Serial.println(millis());
#endif
					clashSuppress = -1;
					swingSuppress = -1;
					lockupPressed = true;
				}
				// Trigger Blaster
				if (lockupButtonState == HIGH
						and lastPlayed != lockupLastPlayed) {
#ifdef LS_BUTTON_DEBUG
					Serial.println(F("Lockup Short released"));
#endif
					toPlay = 1;
					lockupPressed = false;

				}
				// Stops Lockup
				if (lockupButtonState == HIGH
						and lastPlayed == lockupLastPlayed) {
#ifdef LS_BUTTON_DEBUG
					Serial.println(F("Lockup Long released"));
#endif
					clashSuppress = -1;
					swingSuppress = -1;
					toPlay = 4;
					lockupPressed = false;
				}
			} else {
				//ConfigMode or Down button ?
				if (lockupButtonState == LOW and !lockupPressed) {
#ifdef LS_BUTTON_DEBUG
					Serial.print(F("Config Short Press "));
					Serial.print(F(" time="));
					Serial.println(millis());
#endif
					lockupPressed = true;
					repeat = true;
				}
				// Trigger Down button
				if (lockupButtonState == HIGH and !changeMenu) {
#ifdef LS_BUTTON_DEBUG
					Serial.println(F("Config Short released"));
#endif
					lockupPressed = false;
					lockupLastPlayed = -1;
					repeat = false;

				}
				// End Entering/Leaving config Mode
				if (lockupButtonState == HIGH and changeMenu) {
#ifdef LS_BUTTON_DEBUG
					Serial.println(F("Config Long released"));
#endif
					lockupPressed = false;
					changeMenu = false;
					repeat = true;
				}
			}
		}
	} else if (diff > debounceDelay + 300) {
		if (readingLockup == lockupButtonState) {
			if (actionMode) {
				//Triggers Lockup
				if (lockupButtonState == LOW and lockupPressed
						and lastPlayed != lockupLastPlayed
						and lockupLastPlayed == 0) {
#ifdef LS_BUTTON_DEBUG
					Serial.print(F("Lockup Long Press"));
					Serial.print(F(" time="));
					Serial.println(millis());
#endif
					toPlay = 5;
					lockupPressed = false;

				}
			} else { // configMode
				//Trigger Entering/Leaving ConfigMode or change menu in Config Mode
				if (lockupButtonState == LOW and lockupPressed
						and !changeMenu) {
#ifdef LS_BUTTON_DEBUG
					Serial.print(F("Config Long Press"));
					Serial.print(F(" time="));
					Serial.println(millis());
#endif
					changeMenu = true;
					enterMenu = true;
					configMode = !configMode;
					lockupPressed = false;
					repeat = true;
				}
			}
		}
	}

	if (actionMode) {
		if (toPlay == 4) {
			lastPlayed = mp3.playTrackFromDir(4, soundFont.getFolder());
			repeat = true;
			changePlayMode = true;
			lockupLastPlayed = 0;
			clashSuppress = 0;
			swingSuppress = 0;

		} else if (toPlay == 5) {
			if (soundFont.getLockup()) {
				lastPlayed = mp3.playTrackFromDir(soundFont.getLockup(),
						soundFont.getFolder());
			}
			lockupLastPlayed = lastPlayed;
			repeat = true;
			changePlayMode = true;
		} else if (toPlay == 1) {
			if (soundFont.getBlaster()) {
				lastPlayed = mp3.playTrackFromDir(soundFont.getBlaster(),
						soundFont.getFolder());
			}
			lockupLastPlayed = 0;
			repeat = false;
			changePlayMode = true;
		}
	}
}

void turnOn(int ledPins[]) {
// Light up the ledstrings
	int i;
	for (i = 0; i < 6; i++) {
		digitalWrite(ledPins[i], HIGH);
		if (i < 5) {
			delay(83);
		}
	}
}				//turnOn

void turnOff(int ledPins[]) {
// Light up the ledstrings
	int i;
	for (i = 5; i >= 0; i--) {
		digitalWrite(ledPins[i], LOW);
		if (i > 0) {
			delay(83);
		}
	}
}				//turnOff

void flicker(int ledPins[], int value) {
// Flicker the ledstrings
	int i;
	for (i = 5; i >= 0; i--) {
		analogWrite(ledPins[i], value);
	}
} //flicker

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

void motionEngine() {
//	VectorInt16 aaWorld_tmp;
//	Quaternion quaternion_tmp;
	long multiplier = 100000;
// if programming failed, don't try to do anything
	if (!dmpReady)
		return;

// wait for MPU interrupt or extra packet(s) available
	while (!mpuInterrupt && mpuFifoCount < packetSize) {
		// other program behavior stuff here
		// .
		// .
		// .
		// if you are really paranoid you can frequently test in between other
		// stuff to see if mpuInterrupt is true, and if so, "break;" from the
		// while() loop to immediately process the MPU data
		// .
		// .
		// .
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

