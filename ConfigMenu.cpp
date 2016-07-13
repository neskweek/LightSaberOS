/*
 * Config.cpp
 *
 * author: 		Sebastien CAPOU (neskweek@gmail.com)
 * Source : 	https://github.com/neskweek/LightSaberOS
 *      Author: neskw
 */
#include "ConfigMenu.h"
#include "Config.h"
#include "Light.h"

extern int8_t modification;
extern bool play;
extern int16_t value;

// ====================================================================================
// ===           	  	 			CONFIG MODE FUNCTIONS	                		===
// ====================================================================================

void confParseValue(uint16_t variable, uint16_t min, uint16_t max,
		short int multiplier, DFPlayer& dfplayer) {

	value = variable + (multiplier * modification);

	if (value < (int) min) {
		value = max;
	} else if (value > (int) max) {
		value = min;
	} else if (value == (int) min and play) {
		play = false;
		dfplayer.playPhysicalTrack(15);
		delay(150);
	} else if (value == (int) max and play) {
		play = false;
		dfplayer.playPhysicalTrack(14);
		delay(150);
	}
} //confParseValue

void confMenuStart(uint16_t variable, uint16_t sound, DFPlayer& dfplayer) {
	extern uint8_t ledPins[];
#if defined LUXEON
	extern uint8_t currentColor[];
#endif
#if defined NEOPIXEL
	extern cRGB currentColor;
#endif
	extern bool enterMenu;
	if (enterMenu) {
		dfplayer.playPhysicalTrack(sound);
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
