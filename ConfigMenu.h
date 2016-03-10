/*
 * Config.h
 *
 * author: 		Sebastien CAPOU (neskweek@gmail.com)
 * Source : 	https://github.com/neskweek/LightSaberOS
 */

#include <Arduino.h>
#include "DFPlayer.h"
#ifndef CONFIGMENU_H_
#define CONFIGMENU_H_

// ====================================================================================
// ===           	  	 			CONFIG MODE FUNCTIONS	                		===
// ====================================================================================

void confParseValue(uint16_t variable, uint16_t min, uint16_t max,
		short int multiplier, DFPlayer& dfplayer);

void confMenuStart(uint16_t variable, uint16_t sound,  DFPlayer& dfplayer);

#endif /* CONFIG_H_ */
