/*
 * Buttons.c
 *
 *  Created on: 6 mars 2016
 * author: 		Sebastien CAPOU (neskweek@gmail.com) and Andras Kun (kun.andras@yahoo.de)
 * Source : 	https://github.com/neskweek/LightSaberOS
 */

#include "Buttons.h"
#include "Config.h"
#include "SoundFont.h"


extern SoundFont soundFont;
enum SaberStateEnum {S_STANDBY, S_SABERON, S_CONFIG, S_SLEEP, S_JUKEBOX};
enum ActionModeSubStatesEnum {AS_HUM, AS_IGNITION, AS_RETRACTION, AS_BLADELOCKUP, AS_BLASTERDEFLECTMOTION, AS_BLASTERDEFLECTPRESS, AS_CLASH, AS_SWING, AS_SPIN, AS_FORCE};
extern SaberStateEnum SaberState;
extern SaberStateEnum PrevSaberState;
extern ActionModeSubStatesEnum ActionModeSubStates;
//extern bool actionMode;
//extern bool configMode;
extern unsigned long sndSuppress;
extern bool hum_playing;
extern int8_t modification;
extern bool play;
#ifdef JUKEBOX
extern bool jukebox_play;
extern uint8_t jb_track;
#endif
//extern bool blasterBlocks;
//extern bool lockup;
extern int8_t blink;
extern bool changeMenu;
extern uint8_t menu;
extern bool enterMenu;
#if defined LEDSTRINGS
//extern uint8_t ledPins[] = { LEDSTRING1, LEDSTRING2, LEDSTRING3, LEDSTRING4,
//LEDSTRING5, LEDSTRING6 };
extern uint8_t blasterPin;
#endif
extern uint8_t blaster;
extern void HumRelaunch();
extern void SinglePlay_Sound(uint8_t track);
extern void LoopPlay_Sound(uint8_t track);
extern void Pause_Sound();
extern void Resume_Sound();
extern void Set_Loop_Playback();
// ====================================================================================
// ===               			BUTTONS CALLBACK FUNCTIONS                 			===
// ====================================================================================

void mainClick() {
#if defined LS_BUTTON_DEBUG
	Serial.println(F("Main button click."));
#endif
	if (SaberState==S_SABERON) {
		if (soundFont.getForce()) {
      ActionModeSubStates=AS_FORCE;
			// Some Soundfont may not have Force sounds
			if (millis() - sndSuppress >= 30) {
				SinglePlay_Sound(soundFont.getForce());
				sndSuppress = millis();
			}
		}
	} else if (SaberState==S_CONFIG) {
		//Button "+"
		modification = 1;
		play = true;
	} else if (SaberState==S_STANDBY) {
		// LightSaber poweron
   SaberState=S_SABERON;
   PrevSaberState=S_STANDBY;
   ActionModeSubStates=AS_IGNITION;
		//actionMode = true;
	}
#ifdef JUKEBOX 
  else if (SaberState==S_JUKEBOX) {
#if defined LS_BUTTON_DEBUG
    Serial.print(F("Next JukeBox sound file "));Serial.print(jb_track);
#endif    // jump to next song and start playing it
    if (jb_track==NR_CONFIGFOLDERFILES+NR_JUKEBOXSONGS) {
      jb_track=NR_CONFIGFOLDERFILES+1;  // fold back to first song in the dir designated for music playback
    }
    else {
      jb_track++;
    }
    SinglePlay_Sound(jb_track);
	}
#endif
} // mainClick

void mainDoubleClick() {
#if defined LS_BUTTON_DEBUG
	Serial.println(F("Main button double click."));
#endif
	if (SaberState==S_SABERON) {
		//ACTION TO DEFINE
	} else if (SaberState==S_CONFIG) {
		// Trigger needs to be hardened with some sort of double click combinaison

		//RESET CONFIG

//		for (unsigned int i = 0; i < EEPROMSizeATmega328; i++) {
//			//			 if (EEPROM.read(i) != 0) {
//			EEPROM.update(i, 0);
//			//			 }
//		}

	} else if (SaberState==S_STANDBY) {
		//ACTION TO DEFINE
	}
} // mainDoubleClick

void mainLongPressStart() {
#if defined LS_BUTTON_DEBUG
	Serial.println(F("Main button longPress start"));
#endif
	if (SaberState==S_SABERON) {
// LightSaber shutdown
  ActionModeSubStates=AS_RETRACTION;
  SaberState=S_STANDBY;
  PrevSaberState=S_SABERON;
		//actionMode = false;
	} else if (SaberState==S_CONFIG) {
// Change Menu
		changeMenu = true;
		enterMenu = true;
		menu++;
#if defined LUXEON
			if (menu==4){menu=0;}  // 3 menu items
#endif
#if defined LEDSTRINGS
			if (menu==2){menu=0;}  // 2 menu items
#endif
#if defined NEOPIXEL
      if (menu==5){menu=0;}  // 4 menu items
#endif
	} else if (SaberState==S_STANDBY) {
		/*
		 * ACTION TO DEFINE
		 */
	}
} // mainLongPressStart

void mainLongPress() {
#if defined LS_BUTTON_DEBUG
	Serial.println(F("Main button longPress..."));
#endif
	if (SaberState==S_SABERON) {
		/*
		 * ACTION TO DEFINE
		 */

	} else if (SaberState==S_CONFIG) {
		/*
		 * ACTION TO DEFINE
		 */
	} else if (SaberState==S_STANDBY) {
		/*
		 * ACTION TO DEFINE
		 */
	}
} // mainLongPress

void mainLongPressStop() {
#if defined LS_BUTTON_DEBUG
	Serial.println(F("Main button longPress stop"));
#endif
	if (SaberState==S_STANDBY) {
		/*
		 * ACTION TO DEFINE
		 */
	}
} // mainLongPressStop

void lockupClick() {
#if defined LS_BUTTON_DEBUG
	Serial.println(F("Lockup button click."));
#endif
	if (SaberState==S_SABERON) {
// Blaster
#if defined LS_BUTTON_DEBUG
  Serial.println(F("Start button activated blaster bolt deflect"));
#endif
    ActionModeSubStates=AS_BLASTERDEFLECTPRESS;
	} else if (SaberState==S_CONFIG) {
// Button "-"
		modification = -1;
		play = true;
	} else if (SaberState==S_STANDBY) {
		/*
		 * ACTION TO DEFINE
		 */
	}
#ifdef JUKEBOX
  else if (SaberState==S_JUKEBOX) {
    if (jukebox_play) {
      // pause the song
#if defined LS_BUTTON_DEBUG
      Serial.println(F("Pause Song"));
#endif
      jukebox_play=false;
      Pause_Sound();
    } else {
      // resume playing the song
#if defined LS_BUTTON_DEBUG
      Serial.println(F("Resume Song"));
#endif
      jukebox_play=true;
      Resume_Sound();
    }
  
  }
#endif

} // lockupClick

void lockupDoubleClick() {
#if defined LS_BUTTON_DEBUG
	Serial.println(F("Lockup button double click."));
#endif
	if (SaberState==S_SABERON) {
#if defined LS_BUTTON_DEBUG
  Serial.println(F("Start motion triggered blaster bolt deflect"));
#endif
    if (ActionModeSubStates!=AS_BLASTERDEFLECTMOTION) { // start motion triggered blaster deflect
      ActionModeSubStates=AS_BLASTERDEFLECTMOTION;
#if defined LS_BUTTON_DEBUG
      Serial.println(F("Start motion triggered blaster bolt deflect"));
#endif
    }
    else { // stop motion triggered blaster deflect
#if defined LS_BUTTON_DEBUG
      Serial.println(F("End motion triggered blaster bolt deflect"));
#endif
      HumRelaunch();
      ActionModeSubStates=AS_HUM;
    }
	} else if (SaberState==S_CONFIG) {
		//ACTION TO DEFINE
	}
#ifdef JUKEBOX 
	else if (SaberState==S_STANDBY) {
#if defined LS_BUTTON_DEBUG
      Serial.println(F("Enter JukeBox"));
#endif
      SaberState=S_JUKEBOX;
      PrevSaberState=S_STANDBY;
		//ACTION TO DEFINE
	} else if (SaberState==S_JUKEBOX) {
//Entering JukeBox mode (MP3 player)
    SaberState=S_STANDBY;
    PrevSaberState=S_JUKEBOX;
// stop/pause track being played
    Pause_Sound();
  }
#endif
} // lockupDoubleClick

void lockupLongPressStart() {
#if defined LS_BUTTON_DEBUG
	Serial.println(F("Lockup button longPress start"));
#endif
	if (SaberState==S_SABERON) {
//Lockup Start
    ActionModeSubStates=AS_BLADELOCKUP;
		blink=0;
		if (soundFont.getLockup()) {
			SinglePlay_Sound(soundFont.getLockup());
			sndSuppress = millis();
			while (millis() - sndSuppress < 50) {
			}
			Set_Loop_Playback();
			sndSuppress = millis();
			while (millis() - sndSuppress < 50) {
			}
		}
	} else if (SaberState==S_CONFIG) {
//Leaving Config Mode
		changeMenu = false;
		//	repeat = true;
    SaberState=S_STANDBY;
    PrevSaberState=S_CONFIG;
		//configMode = false;

	} else if (SaberState==S_STANDBY) {
//Entering Config Mode
    SaberState=S_CONFIG;
    PrevSaberState=S_STANDBY;
		//configMode = true;

	}
} // lockupLongPressStart

void lockupLongPress() {
#if defined LS_BUTTON_DEBUG
	Serial.println(F("Lockup button longPress..."));
#endif
	if (SaberState==S_SABERON) {
		/*
		 * ACTION TO DEFINE
		 */
    ActionModeSubStates=AS_BLADELOCKUP; // needed, otherwise the FSM will change to AS_HUM and the lockup will end prematurely when the hum is relaunched
    sndSuppress = millis();  // trick the hum relaunch by starting the stopper all over again otherwise the hum relaunch will interrupt the lockup
	} else if (SaberState==S_CONFIG) {
		/*
		 * ACTION TO DEFINE
		 */
	} else if (SaberState==S_STANDBY) {
		/*
		 * ACTION TO DEFINE
		 */
	}
} // lockupLongPress

void lockupLongPressStop() {
#if defined LS_BUTTON_DEBUG
	Serial.println(F("Lockup button longPress stop"));
#endif
	if (SaberState==S_SABERON) {
    HumRelaunch();
    ActionModeSubStates=AS_HUM;
	}
} // lockupLongPressStop


