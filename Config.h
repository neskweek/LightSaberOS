/*
 * Config.h
 *
 * Created on: 6 mars 2016
 * author: 		Sebastien CAPOU (neskweek@gmail.com) and Andras Kun (kun.andras@yahoo.de)
 * Source : 	https://github.com/neskweek/LightSaberOS
 */

#if not defined CONFIG_H_
#define CONFIG_H_



/*!!!!!IMPORTANT IMPORTANT IMPORTANT IMPORTANT IMPORTANT!!!
 *
 * MPU6050 device ORIENTATION
 * Choose which MPU's axis is parallel
 * to your blade axis
 *************************************/
//#define BLADE_X
#define BLADE_Y
//#define BLADE_Z
/************************************/

/*
 * MPU6050 calibrated offset values
 * If defined, calibration values will be retrieved from EEPROM
 * use this option if the MPU6050_calibration sketch wrote the calibrated offsets
 * into the EEPROM (default address is 96)
 * If not defined, you have to note down the calibrated offset values
 * and assign them to the respective variables in the code.
 *************************************/
#define MPUCALOFFSETEEPROM
#ifdef MPUCALOFFSETEEPROM
#define MEMORYBASEMPUCALIBOFFSET 96
#endif
/************************************/


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
//#define LUXEON
//#define NEOPIXEL

/************************************/
/*
 * SABER TYPE
 * currently in v1.3 only the CROSSGUARDSABER
 * will have any effect on the code
 * due to the fire blade effect
 *************************************/
#define SINGLEBLADE  // i.e. Graflex
//#define SABERSTAFF  // i.e. Darth Maul saber with dual blades
//#define CROSSGUARDSABER  // i.e. Kylo Ren saber

/*
 * DEFAULT CONFIG PARAMETERS
 * Will be overriden by EEPROM settings
 * once the first save will be done
 *************************************/
#define VOL          20
#define SOUNDFONT       3
#define SWING         1000
/************************************/

/*
 * DO NOT MODIFY
 * Unless you know what you're doing
 *************************************/
#if defined LEDSTRINGS
#define CONFIG_VERSION     "L01"
#endif
#if defined LUXEON
#define CONFIG_VERSION     "L02"
#endif
#if defined NEOPIXEL
#define CONFIG_VERSION     "L03"
#endif
#define MEMORYBASE       32

/************************************/

#if defined NEOPIXEL
// How many leds in one strip?
#define NUMPIXELS 120  // can go up to 120, could lead to memory problems if further increased

#ifdef CROSSGUARDSABER
// define how many pixels are used for the crossguard and how many for the main blade
#define CG_STRIPE 10
#define MN_STRIPE 50
#endif

#define FIREBLADE

// Number of color defined
#define COLORS 14
static const uint8_t rgbFactor = 255;

// For led chips like NEOPIXELs, which have a data line, ground, and power, you just
// need to define DATA_PIN.  For led chipsets that are SPI based (four wires - data, clock,
// ground, and power), like the LPD8806 define both DATA_PIN and CLOCK_PIN
#define DATA_PIN 			13 //D13
#define STRING1				5
#define STRING2 			6
#define STRING3 			9
#endif







#if defined LUXEON
/*
 * MY_OWN_COLORS
 * If you want to manually specify your own colors
 */
#define MY_OWN_COLORS
//#define FIXED_RANGE_COLORS

static const uint8_t rgbFactor = 100;


# if defined MY_OWN_COLORS
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
 * Default = 100 (39,2%) Max=255 Min=0(Off)
 *
 * WARNING ! A too high value may burn
 * your leds. Please make your maths !
 * BE VERY CAREFULL WITH THIS ONE OR 
 * YOU'LL BURN YOUR BLADE'S LED 
 ************************************/
#define MAX_BRIGHTNESS		150

// How long do the light effect last for the different FX's
#define CLASH_FX_DURATION 200
#define BLASTER_FX_DURATION 300
#define SWING_FX_DURATION 400


#define BLASTER_FLASH_TIME  3
#define CLASH_FLASH_TIME  	1

/* FX DURATIONS AND SUPRESS TIMES
 *  effects cannot be retriggered for the duration
 *  of their respective supress pareameters
 *  HUM_RELAUNCH will tell the state machine to relaunch
 *  hum sound after this time period elapses
 */
#define SWING_SUPPRESS     500
#define CLASH_SUPRESS     400  // do not modify below 400, otherwise interlocking clash sounds can occur
#define BLASTERBLOCK_SUPRESS     400
#define HUM_RELAUNCH     5000

/* BLASTER DEFLECT TYPE
 * Define how a blaser bolt deflect is
 * to be triggered
 * Blaster deflect action is started with
 * a single click on the lockup button.
 * if BLASTERCLICKTRIGGER is defined, a blaster deflect
 * will be triggered once on click.
 * if BLATSTERMOVEMENTTRIGGER is defined,
 * blaser deflect is triggered by ensuing swings/movements.
 *************************************/
#define BLASTERCLICKTRIGGER
#ifndef BLASTERCLICKTRIGGER
#define BLATSTERMOVEMENTTRIGGER
#endif

/* WRIST_MOVEMENTS
 * If you want to enable/disable
 * wrists twists movements
 *************************************/
//#define WRIST_MOVEMENTS


/* DEEP_SLEEP
 * If you want to enable/disable
 * deep sleep capabalities
 * If you a device with a CPU wich is not
 * an Atmega328 : COMMENT THIS
 *************************************/
//#define DEEP_SLEEP
#if defined DEEP_SLEEP
#define SLEEP_TIMER			300000 //5min = 300000 millisecs
#endif

#define VOLTAGEDIVIDER 2,36



/*
 * PINS DEFINITION
 */

#if defined LEDSTRINGS

#define LEDSTRING1 			3 //3
#define LEDSTRING2 			5 //5
#define LEDSTRING3 			6  //6
#define LEDSTRING4 			9  //9
#define LEDSTRING5 			10  //10
#define LEDSTRING6 			11 //11

/*
 * FoCSTRING
 * Enable/disable management of
 * single Flash On Clash ledstring
 *************************************/
//#define FoCSTRING			14
#endif

#ifdef NEOPIXELS

#define LS1       3
#define LS2       5
#define LS3       6
#define LS4       9
#define LS5       10
#define LS6       11

#endif
#if defined LUXEON

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
//#define ACCENT_LED  15 //A1
#if defined ACCENT_LED
/*
 * Soft or Had PWM for Accent
 */
//#define SOFT_ACCENT
#if not defined SOFT_ACCENT
//#define HARD_ACCENT
#endif
#endif //ACCENT_LED




/*
 * MULTICOLOR_ACCENT_LED
 * Enable/disable management of
 * a button accent led
 *************************************/
//#define MULTICOLOR_ACCENT_LED
#if defined MULTICOLOR_ACCENT_LED
#define RED_ACCENT_LED  16 //A2
#define GREEN_ACCENT_LED  17 //A3
#define BLUE_ACCENT_LED  A7  //.... A7 is input only ...
#endif



#define DFPLAYER_RX			8
#define DFPLAYER_TX			7
#define SPK1				20 //A6
#define SPK2				21 //A7


#define MAIN_BUTTON			12
#define LOCKUP_BUTTON		4

#define BUZZMOTOR  17 //A3
#define BUTTONLEDPIN 16 //A2


/*
 * CONFIG MENU PARAMETERS
 */
#define JUKEBOX
#if defined LUXEON
#define CONFIG_BLADE_MAIN_COLOR
#define CONFIG_BLADE_CLASH_COLOR
#endif

#if defined NEOPIXELS
#define CONFIG_BLADE_MAIN_COLOR
#define CONFIG_BLADE_CLASH_COLOR
#define CONFIG_POWERON_EFFECT
#define CONFIG_POWEROFF_EFFECT
#define CONFIG_FLICKER_EFFECT
#endif

#if defined LEDSTRINGS
#define CONFIG_POWERON_EFFECT
#define CONFIG_POWEROFF_EFFECT
#define CONFIG_FLICKER_EFFECT
#endif
/*
 * DEBUG PARAMETERS
 */
/* LS_INFO
 * For daily use I recommend you comment LS_INFO
 * When you plug your device to USB uncomment LS_INFO !
 */
//#define LS_SERIAL  //enable serial communication using Wire library
#if defined LS_SERIAL
//#define LS_INFO
//#define LS_DEBUG
#endif

#if defined LS_DEBUG
#define LS_BUTTON_DEBUG
//#define LS_MOTION_DEBUG
//#define LS_MOTION_HEAVY_DEBUG
//#define LS_RELAUNCH_DEBUG
//#define LS_DEBUG_SLEEP
#endif

#if defined LS_MOTION_DEBUG
#define LS_SWING_DEBUG
#define LS_SWING_HEAVY_DEBUG
//#define LS_CLASH_DEBUG
//#define LS_CLASH_HEAVY_DEBUG
#endif




#endif /* CONFIG_H_ */
