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
#ifdef LUXEON
	extern uint8_t currentColor[];
#endif
	extern bool enterMenu;
	if (enterMenu) {
		dfplayer.playPhysicalTrack(sound);
		delay(100);

		switch (sound) {
		case 4:
#ifdef LS_INFO
			Serial.print(F("VOL\nCur:"));
#endif
#ifdef LEDSTRINGS
			lightOff(ledPins);
			lightOn(ledPins, 0);
#endif
			break;
		case 5:
#ifdef LS_INFO
			Serial.print(F("SNDFT\nCur:"));
#endif
#ifdef LEDSTRINGS
			lightOff(ledPins);
			lightOn(ledPins, 1);
#endif
			break;
		case 6:
#ifdef LS_INFO
			Serial.print(F("SWING\nCur:"));
#endif
#ifdef LEDSTRINGS
			lightOff(ledPins);
			lightOn(ledPins, 2);
#endif
			break;
#ifdef LUXEON
			case 9:
			lightOff(ledPins);
#ifdef LS_INFO
			Serial.print(F("COLOR1\nCur:"));
#endif
			getColor(currentColor, variable);
			lightOn(ledPins, currentColor);
			break;
			case 10:
			lightOff(ledPins);
#ifdef LS_INFO
			Serial.print(F("COLOR2\nCur:"));
#endif
			getColor(currentColor, variable);
			lightOn(ledPins, currentColor);
			break;
			case 11:
			lightOff(ledPins);
#ifdef LS_INFO
			Serial.println(F("SAVE?\n"));
#endif
			break;
#endif
#ifdef LEDSTRINGS
		case 17:
#ifdef LS_INFO
			Serial.print(F("PWRON\nCur:"));
#endif
			lightOff(ledPins);
			lightOn(ledPins, 3);
			break;
		case 18:
#ifdef LS_INFO
			Serial.print(F("PWROFF\nCur:"));
#endif
			lightOff(ledPins);
			lightOn(ledPins, 4);
			break;
		case 19:


#ifdef LS_INFO
			Serial.print(F("FLICK\nCur:"));
#endif
			lightOff(ledPins);
			lightOn(ledPins, 5);
			break;
#endif
		}

#ifdef LS_INFO
		Serial.println(variable);
#endif
		value = variable;
		enterMenu = false;
		delay(100);
	}
} //confMenuStart
