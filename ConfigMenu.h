/*
 * Config.h
 *
 * author: 		Sebastien CAPOU (neskweek@gmail.com) and Andras Kun (kun.andras@yahoo.de)
 * Source : 	https://github.com/neskweek/LightSaberOS
 */

#include <Arduino.h>
#if not defined CONFIGMENU_H_
#define CONFIGMENU_H_

// ====================================================================================
// ===           	  	 			CONFIG MODE FUNCTIONS	                		===
// ====================================================================================

void confParseValue(uint16_t variable, uint16_t min, uint16_t max,
		short int multiplier);

void confMenuStart(uint16_t variable, uint16_t sound, uint8_t menu);

#endif /* CONFIG_H_ */


