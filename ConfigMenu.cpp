/*
 * Config.cpp
 *
 * author: 		Sebastien CAPOU (neskweek@gmail.com) and Andras Kun (kun.andras@yahoo.de)
 * Source : 	https://github.com/neskweek/LightSaberOS
 *      Author: neskw
 */
#include "ConfigMenu.h"
#include "Config.h"
#include "Light.h"

extern int8_t modification;
extern bool play;
extern int16_t value;
extern void SinglePlay_Sound(uint8_t track);
extern void LoopPlay_Sound(uint8_t track);
// ====================================================================================
// ===           	  	 			CONFIG MODE FUNCTIONS	                		===
// ====================================================================================

// this function ensures that config menu items which have values between a min and a max value
// wrap back to min/max upon reaching max/min. It also plays a sound notifying the user if either min or max value has beeb reached.
// This function is also in charge of changing the actual value of a setting via the value global variable.
void confParseValue(uint16_t variable, uint16_t min, uint16_t max,
		short int multiplier) {

	value = variable + (multiplier * modification);

	if (value < (int) min) {
		value = max;
	} else if (value > (int) max) {
		value = min;
	} else if (value == (int) min and play) {
		play = false;
		SinglePlay_Sound(10);
		delay(150);
	} else if (value == (int) max and play) {
		play = false;
		SinglePlay_Sound(9);
		delay(150);
	}
} //confParseValue

// this functions parses in the value of the config variable and based on it plays sounds or activates LEDs
void confMenuStart(uint16_t variable, uint16_t sound) {
	extern uint8_t ledPins[];
#if defined LUXEON
	extern uint8_t currentColor[];
#endif
#if defined NEOPIXEL
	extern cRGB currentColor;
#endif
	extern bool enterMenu;
	if (enterMenu) {
		SinglePlay_Sound(sound);
		delay(500);

		switch (sound) {
		case 4:
#if defined LS_INFO
			Serial.print(F("VOL\nCur:"));
#endif
#if defined LEDSTRINGS
			lightOff();
			lightOn(ledPins, 0);
#endif
			break;
		case 5:
#if defined LS_INFO
			Serial.print(F("SNDFT\nCur:"));
#endif
#if defined LEDSTRINGS
			lightOff();
			lightOn(ledPins, 1);
#endif
			break;
		case 6:
#if defined LS_INFO
			Serial.print(F("SWING\nCur:"));
#endif
#if defined LEDSTRINGS
			lightOff();
			lightOn(ledPins, 5);
#endif
			break;
#if defined LUXEON
			case 9:
			lightOff(ledPins);
#if defined LS_INFO
			Serial.print(F("COLOR1\nCur:"));
#endif
			getColor(currentColor, variable);
			lightOn(ledPins, currentColor);
			break;
			case 10:
			lightOff(ledPins);
#if defined LS_INFO
			Serial.print(F("COLOR2\nCur:"));
#endif
			getColor(currentColor, variable);
			lightOn(ledPins, currentColor);
			break;
			case 11:
			lightOff(ledPins);
#if defined LS_INFO
			Serial.println(F("SAVE?\n"));
#endif
			break;
#endif
#if defined LEDSTRINGS
			case 17:
#if defined LS_INFO
			Serial.print(F("PWRON\nCur:"));
#endif
			lightOff();
			lightOn(ledPins, 2);
			break;
			case 18:
#if defined LS_INFO
			Serial.print(F("PWROFF\nCur:"));
#endif
			lightOff();
			lightOn(ledPins, 3);
			break;
			case 19:
#if defined LS_INFO
			Serial.print(F("FLICK\nCur:"));
#endif
			lightOff();
			lightOn(ledPins, 4);
			break;
#endif //LEDSTRINGS
#if defined NEOPIXEL
    case 8:
      lightOff();

#if defined LS_INFO
      Serial.print(F("COLOR3\nCur:"));
#endif
      getColor(variable);
      for (uint8_t i = 0; i < 3; i++) {
        digitalWrite(ledPins[i], HIGH);
      }
      lightOn(currentColor);
      break;
    case 9:
			lightOff();

#if defined LS_INFO
			Serial.print(F("COLOR1\nCur:"));
#endif
			getColor(variable);
			for (uint8_t i = 0; i < 3; i++) {
				digitalWrite(ledPins[i], HIGH);
			}
			lightOn(currentColor);
			break;
		case 10:
			lightOff();
#if defined LS_INFO
			Serial.print(F("COLOR2\nCur:"));
#endif
			getColor(variable);
			lightOn(currentColor);
			break;
//			case 11:
//
//#if defined LS_INFO
//			Serial.println(F("SAVE?\n"));
//#endif
//			break;
		case 17:
			lightOff();
			for (uint8_t i = 0; i < 3; i++) {
				digitalWrite(ledPins[i], LOW);
			}
#if defined LS_INFO
			Serial.print(F("PWRON\nCur:"));
#endif
			break;
		case 18:
#if defined LS_INFO
			Serial.print(F("PWROFF\nCur:"));
#endif
			break;
		case 19:

#if defined LS_INFO
			Serial.print(F("FLICK\nCur:"));
#endif
			lightOff();
			lightOn(currentColor);
			break;
#endif
		}

#if defined LS_INFO
		Serial.println(variable);
#endif
		value = variable;
		enterMenu = false;
		delay(100);
	}
} //confMenuStart

