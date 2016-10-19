/*
 * Buttons.h
 *
 *  Created on: 6 mars 2016
 *  author: 		Sebastien CAPOU (neskweek@gmail.com) and Andras Kun (kun.andras@yahoo.de)
 *  Source : 	https://github.com/neskweek/LightSaberOS
 */
#include <Arduino.h>
#if not defined BUTTONS_H_
#define BUTTONS_H_


/*
 * BUTTONS PARAMETERS
 ************************************/
#define CLICK				200 //5    // ms you need to press a button to be a click
#define PRESS_ACTION		600 //200  // ms you need to press a button to be a long press, in action mode
#define PRESS_CONFIG		600 //400  // ms you need to press a button to be a long press, in config mode
/************************************/




// ====================================================================================
// ===               			BUTTONS CALLBACK FUNCTIONS                 			===
// ====================================================================================

void mainClick();
void mainDoubleClick();
void mainLongPressStart();
void mainLongPress();
void mainLongPressStop();

void lockupClick();
void lockupDoubleClick();
void lockupLongPressStart();
void lockupLongPress();
void lockupLongPressStop();

#endif /* BUTTONS_H_ */


