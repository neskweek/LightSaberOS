/*
 * Light.h
 *
 *  Created on: 6 mars 2016
 * author: 		Sebastien CAPOU (neskweek@gmail.com)
 * Source : 	https://github.com/neskweek/LightSaberOS
 */

#ifndef LIGHT_H_
#define LIGHT_H_

#include <Arduino.h>



// ====================================================================================
// ===              	    			LED FUNCTIONS		                		===
// ====================================================================================

#ifdef LEDSTRINGS

void lightOn(uint8_t ledPins[], int8_t segment = -1);
void lightOff(uint8_t ledPins[]);

void lightIgnition(uint8_t ledPins[], uint16_t time, uint8_t type);
void lightRetract(uint8_t ledPins[], uint16_t time, uint8_t type);

void FoCOn (uint8_t pin);
void FoCOff (uint8_t pin);

void lightFlicker(uint8_t ledPins[], uint8_t type, uint8_t value = 0);

#endif
#ifdef LUXEON

void lightOn(uint8_t ledPins[], uint8_t color[]);
void lightOff(uint8_t ledPins[]);

void lightIgnition(uint8_t ledPins[], uint8_t color[], uint16_t time);
void lightRetract(uint8_t ledPins[], uint8_t color[], uint16_t time);

void lightFlicker(uint8_t ledPins[], uint8_t color[], uint8_t value = 0);


#ifdef MY_OWN_COLORS
void getColor(uint8_t color[], uint8_t colorID); //getColor
#endif
#ifdef FIXED_RANGE_COLORS
void getColor(uint8_t color[], uint16_t colorID); //getColor
#endif
#endif

#endif /* LIGHT_H_ */
