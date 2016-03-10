/*
 * Config.h
 *
 * Created on: 6 mars 2016
 * author: 		Sebastien CAPOU (neskweek@gmail.com)
 * Source : 	https://github.com/neskweek/LightSaberOS
 */

#ifndef CONFIG_H_
#define CONFIG_H_


/*!!!!!IMPORTANT IMPORTANT IMPORTANT IMPORTANT IMPORTANT!!!
 *
 * MPU6050 device ORIENTATION
 * Choose which MPU's axis is parallel
 * to your blade axis
 *************************************/
//#define BLADE_X
//#define BLADE_Y
#define BLADE_Z
/************************************/

/*
 * BLADE TYPE
 *
 * RGB LED OR NEOPIXEL users:
 * Comment the following line will
 * disable and remove all LEDSTRINGS
 * blocks from compile
 *************************************/
#define LEDSTRINGS
#ifndef LEDSTRINGS
#define LUXEON
#endif


#ifdef LUXEON
/*
 * MY_OWN_COLORS
 * If you want to manually specify your own colors
 */
#define MY_OWN_COLORS
//#define FIXED_RANGE_COLORS

static const uint8_t rgbFactor = 100;


# ifdef MY_OWN_COLORS
/* COLORS
 * Number of colors YOU defined in getColor function
 */
#define COLORS		 		3
#else
/* COLORS
 * Number of colors to chose from
 * Range : 6<->600
 * Default: 48
 */
#define COLORS		 		48
#endif
#endif
/************************************/ // BLADE TYPE





/* MAX_BRIGHTNESS
 *
 * Maximum output voltage to apply to LEDS
 * Default = 200 (78,4%) Max=255 Min=0(Off)
 *
 * WARNING ! A too high value may burn
 * your leds. Please make your maths !
 ************************************/
#define MAX_BRIGHTNESS		200



#define BLASTER_FLASH_TIME  3
#define CLASH_FLASH_TIME  	1



/* WRIST_MOVEMENTS
 * If you want to enable/disable
 * wrists twists movements
 *************************************/
//#define WRIST_MOVEMENTS


/* DEEP_SLEEP
 * If you want to enable/disable
 * deep sleep capabalities
 *************************************/
#define DEEP_SLEEP
#ifdef DEEP_SLEEP
#define SLEEP_TIMER			300000 //5min = 300000 millisecs
#endif




/*
 * PINS DEFINITION
 */

#ifdef LEDSTRINGS

#define LEDSTRING1 			3
#define LEDSTRING2 			5
#define LEDSTRING3 			6
#define LEDSTRING4 			9
#define LEDSTRING5 			10
#define LEDSTRING6 			11

/*
 * FoCSTRING
 * Enable/disable management of
 * single Flash On Clash ledstring
 *************************************/
//
#define FoCSTRING			13
#endif

#ifdef LUXEON

#define LED_RED 			3
#define LED_GREEN 			5
#define LED_BLUE 			6
#endif

/*
 * ACCENT_LED
 * Enable/disable management of
 * a button accent led
 *
 * If you define ACCENT_LED beware on which
 * kind of pin you defined it :
 * D3,D5,D6,D9,D10,D11 are hardware PWM
 * others must use software PWM
 *
 * RGB LEDS user should choose tu plu their
 * Accent leds on Hardware PWM pin
 *
 * LEDSTRINGS users have no choice :
 * your forced to use Software Accent LED
 *************************************/
#define ACCENT_LED  A2
#ifdef ACCENT_LED
/*
 * Soft or Had PWM for Accent
 */
#define SOFT_ACCENT
#ifndef SOFT_ACCENT
#define HARD_ACCENT
#endif
#endif //ACCENT_LED




/*
 * MULTICOLOR_ACCENT_LED
 * Enable/disable management of
 * a button accent led
 *************************************/
//#define MULTICOLOR_ACCENT_LED
#ifdef MULTICOLOR_ACCENT_LED
#define RED_ACCENT_LED  A2
#define GREEN_ACCENT_LED  A3
#define BLUE_ACCENT_LED  A7  //.... A7 is input only ...
#endif



#define DFPLAYER_RX			8
#define DFPLAYER_TX			7
#define SPK1				A0
#define SPK2				A1


#define MAIN_BUTTON			12
#define LOCKUP_BUTTON		4


/*
 * DEBUG PARAMETERS
 */
/* LS_INFO
 * For daily use I recommend you comment LS_INFO
 * When you plug your device to USB uncomment LS_INFO !
 */
#define LS_INFO
#ifndef LS_INFO
//#define LS_DEBUG
#endif

#ifdef LS_DEBUG
//#define LS_BUTTON_DEBUG
#define LS_MOTION_DEBUG
//#define LS_MOTION_HEAVY_DEBUG
//#define LS_RELAUNCH_DEBUG
//#define LS_DEBUG_SLEEP
#endif

#ifdef LS_MOTION_DEBUG
#define LS_SWING_DEBUG
//#define LS_SWING_HEAVY_DEBUG
//#define LS_CLASH_DEBUG
//#define LS_CLASH_HEAVY_DEBUG
#endif




#endif /* CONFIG_H_ */
