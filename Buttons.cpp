/*
 * Buttons.c
 *
 *  Created on: 6 mars 2016
 * author: 		Sebastien CAPOU (neskweek@gmail.com)
 * Source : 	https://github.com/neskweek/LightSaberOS
 */
#include <DFPlayer.h>
#include "Buttons.h"
#include "Config.h"
#include "SoundFont.h"

extern DFPlayer dfplayer;
extern SoundFont soundFont;
extern bool actionMode;
extern bool configMode;
extern unsigned long sndSuppress;
extern int8_t modification;
extern bool play;
extern bool blasterBlocks;
extern bool lockup;
extern int8_t blink;
extern bool changeMenu;
extern uint8_t menu;
extern bool enterMenu;

// ====================================================================================
// ===               			BUTTONS CALLBACK FUNCTIONS                 			===
// ====================================================================================

void mainClick() {
#ifdef LS_BUTTON_DEBUG
	Serial.println(F("Main button click."));
#endif
	if (actionMode) {
		if (soundFont.getForce()) {

			// Some Soundfont may not have Force sounds
			if (millis() - sndSuppress >= 30) {
				dfplayer.playPhysicalTrack(soundFont.getForce());
				sndSuppress = millis();
			}
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

void mainDoubleClick() {
#ifdef LS_BUTTON_DEBUG
	Serial.println(F("Main button double click."));
#endif
	if (actionMode) {
		//ACTION TO DEFINE
	} else if (configMode) {
		// Trigger needs to be hardened with some sort of double click combinaison

		//RESET CONFIG

//		for (unsigned int i = 0; i < EEPROMSizeATmega328; i++) {
//			//			 if (EEPROM.read(i) != 0) {
//			EEPROM.update(i, 0);
//			//			 }
//		}

	} else if (!configMode && !actionMode) {
		//ACTION TO DEFINE
	}
} // mainDoubleClick

void mainLongPressStart() {
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
	} else if (!configMode && !actionMode) {
		/*
		 * ACTION TO DEFINE
		 */
	}
} // mainLongPressStart

void mainLongPress() {
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

void mainLongPressStop() {
#ifdef LS_BUTTON_DEBUG
	Serial.println(F("Main button longPress stop"));
#endif
	if (!configMode && !actionMode) {
		/*
		 * ACTION TO DEFINE
		 */
	}
} // mainLongPressStop

void lockupClick() {
#ifdef LS_BUTTON_DEBUG
	Serial.println(F("Lockup button click."));
#endif
	if (actionMode) {
// Blaster

		blasterBlocks = !blasterBlocks;

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
	Serial.println(F("Lockup button double click."));
#endif
	if (actionMode) {
		//ACTION TO DEFINE
	} else if (configMode) {
		//ACTION TO DEFINE
	} else if (!configMode && !actionMode) {
		//ACTION TO DEFINE
	}
} // lockupDoubleClick

void lockupLongPressStart() {
#ifdef LS_BUTTON_DEBUG
	Serial.println(F("Lockup button longPress start"));
#endif
	if (actionMode) {
//Lockup Start
		lockup = true;
		blink=0;
//		Serial.println(soundFont.getLockup());
		if (soundFont.getLockup()) {
			dfplayer.playPhysicalTrack(soundFont.getLockup());
			sndSuppress = millis();
			while (millis() - sndSuppress < 50) {
			}
			dfplayer.setSingleLoop(true);
			sndSuppress = millis();
			while (millis() - sndSuppress < 50) {
			}
		}
	} else if (configMode) {
//Leaving Config Mode
		changeMenu = false;
		//	repeat = true;
		configMode = false;

	} else if (!configMode && !actionMode) {
//Entering Config Mode
		configMode = true;

	}
} // lockupLongPressStart

void lockupLongPress() {
#ifdef LS_BUTTON_DEBUG
	Serial.println(F("Lockup button longPress..."));
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
} // lockupLongPress

void lockupLongPressStop() {
#ifdef LS_BUTTON_DEBUG
	Serial.println(F("Lockup button longPress stop"));
#endif
	if (actionMode) {
//Lockup Stop
		lockup = false;
		dfplayer.playPhysicalTrack(soundFont.getHum());
		delay(70);
		dfplayer.setSingleLoop(true);
		sndSuppress = millis();
	}
} // lockupLongPressStop

