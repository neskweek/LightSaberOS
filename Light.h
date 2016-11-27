/*
 * Light.h
 *
 *  Created on: 21 Octber 2016
 * author: 		Sebastien CAPOU (neskweek@gmail.com) and Andras Kun (kun.andras@yahoo.de)
 * Source : 	https://github.com/neskweek/LightSaberOS
 */

#if not defined LIGHT_H_
#define LIGHT_H_

#include <Arduino.h>
#include <WS2812.h>
#include "Config.h"


enum AccentLedAction_En {AL_PULSE, AL_ON, AL_OFF};

#if defined ACCENT_LED
#if defined SOFT_ACCENT

struct softPWM {
  uint8_t dutyCycle; // in percent
  bool revertCycle;
  uint8_t state;
  uint16_t tick;
} pwmPin = { 100, false, LOW, 0 };
#endif
#endif

// ====================================================================================
// ===              	    			LED FUNCTIONS		                		===
// ====================================================================================

#if defined LEDSTRINGS

void lightOn(uint8_t ledPins[], int8_t segment = -1);
void lightOff();

void lightIgnition(uint8_t ledPins[], uint16_t time, uint8_t type);
void lightRetract(uint8_t ledPins[], uint16_t time, uint8_t type);

void FoCOn (uint8_t pin);
void FoCOff (uint8_t pin);

void lightFlicker(uint8_t ledPins[], uint8_t type, uint8_t value = 0,uint8_t AState=0);
  #ifdef JUKEBOX
    void JukeBox_Stroboscope(uint8_t ledPins[]);
  #endif
#endif
#if defined LUXEON

void lightOn(uint8_t ledPins[], uint8_t color[]);
void lightOff(uint8_t ledPins[]);

void lightIgnition(uint8_t ledPins[], uint8_t color[], uint16_t time);
void lightRetract(uint8_t ledPins[], uint8_t color[], uint16_t time);

void lightFlicker(uint8_t ledPins[], uint8_t color[], uint8_t value = 0);

void getColor(uint8_t color[], uint8_t colorID); //getColor

#ifdef JUKEBOX
    void JukeBox_Stroboscope();
#endif
#endif

#if defined NEOPIXEL

void neopixels_stripeKillKey_Enable();
void neopixels_stripeKillKey_Disable();

void lightOn(cRGB color,int16_t pixel = -1);
void lightOff();

void lightIgnition(cRGB color, uint16_t time, uint8_t type);
void lightRetract( uint16_t time, uint8_t type);

void lightBlasterEffect( uint8_t pixel, uint8_t range, uint8_t SndFnt_MainColor);
void lightFlicker( uint8_t value = 0,uint8_t AState=0);

void getColor(uint8_t colorID); //getColor
void RampNeoPixel(uint16_t RampDuration, bool DirectionUpDown);

#ifdef FIREBLADE
void FireBlade();
cRGB HeatColor( uint8_t temperature);
uint8_t scale8_video( uint8_t i, uint8_t scale);
#endif

#ifdef JUKEBOX
void JukeBox_Stroboscope(cRGB color);
#endif

#endif

void accentLEDControl(AccentLedAction_En AccentLedAction);
void PWM();
#endif /* LIGHT_H_ */


