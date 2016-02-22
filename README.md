# LightSaberOS



## What you will need :
* Arduino IDE ___v1.6.5___ (_not 1.6.7_) or/and Arduino Eclipse
* my new DFPlayer library (Included in zip file)
* [I2Cdev and MPU6050 (Included in zip file)](https://codeload.github.com/jrowberg/i2cdevlib/zip/master) 
* [EEPROMex9.1 (Included in zip file)](http://thijs.elenbaas.net/wp-content/uploads/downloads/2013/12/EEPROMEx-9.1.zip)
* [OneButton (Included in zip file)] (https://github.com/mathertel/OneButton)
* [LinkedList (Included in zip file)] (https://github.com/ivanseidel/LinkedList)
* Wire (Included in Arduino IDE) 
  

Note: If you want to explore/tweak the code I __strongly__ recommend you use "Eclipse for Arduino". This source is quite long. I've tried to keep it the most readbale as possible. 



## Device currently supported :

Designed to be used on Arduino Nano (ATmega328 processors)

* IMU (accelerometer + gyroscope) modules :
	* MPU6050
* Soundplayer modules:
	* DFPlayer Mini
* Blade module:
	* homemade LEDstrings
	* RGB LEDs (Luxeon/Cree styled)
	* Single LED

I would be glad to see other modules added. If you're interested to make your device compatible, please contact me.

## What it does :

* Movement detection for swing, wrist twist and clash detection throught interrupts
* Almost gapless playing with basic soundfiles (without hum extensions)
* "Gapless" playing with soundfiles with hum extensions (see "Notes") 
* Blaster shot (short press on lockup button)
* Lockup (long press on lockup button. Plays until release of the button)
* Config Menu (volume, soundfont, RGB Led blade color changes, swing sensitivity, clash sensitivity)
* Soundfont adding supported by manual declaration in Soundfont.h(from DFPlayer spec: virtually up to 254 soundfonts if you don't exceed a total of 65535 sound files on your SDCard. But not tested yet)
* EEPROM load/save of your config 


## Install / Config

#### 1. IMU calibration 
 First, you'll need (if not already done) to calibrate your MPU6050.  
 [I recommend you use the AutoCalibration script you can find here](http://www.i2cdevlib.com/forums/topic/96-arduino-sketch-to-automatically-calculate-mpu6050-offsets/)  
 Note the offset it will give you and replace those you'll find in my code.  


#### 2. Determine IMU orientation
 The way you physically installed the MPU6050 in your hilt will influence how swing detection works.  
 You'll have to determine which IMU's axis is parallel to blade axis and change it accordingly in the code :   
```c++
//#define BLADE_X
#define BLADE_Y
//#define BLADE_Z
```


#### 3. Prepare your SDCard 
Then, put the content of SDCard_without_hum_extensions.zip on your SDCard:  
Erase any directory that would be named like the ones you'll find in this archive.  
Formatting your SDCard would be even better !  

In this archive there's two example of soundfonts :
* 02: contains a Jedi soundfont without extended hum sound files : Obsidian
* 03: contains a Sith soundfont without extended hum sound files : Sith

If you want an example of soundfont with hum extension, download  [this archive](http://https://drive.google.com/file/d/0B3FqB9nvAU0Fd1p4T05zZnRQWnM/view?usp=sharing) and put its content on your SDCard.   Replace SoundFont.h of your project with the one found in this archive.  
This second example contains the following soundonft :  
* 02: contains a Jedi soundfont __WITH__ extended hum sound files : Obsidian
* 03: contains a Jedi soundfont __WITHOUT__ extended hum sound files : Obsidian
* 04: contains a Sith soundfont without extended hum sound files : Sith



#### 4. Check Wirings
Use of original Protonerd's wirings since 1.0RC3  
Don't forget to wire those ones which were added :  
* DFPLAYER TX to D7
* DFPLAYER SPK+ to A0
* DFPLAYER SPK- to A1
Wiring of busy pin is optional since LightSaberOS doesn't use it.  
![Schematics](http://i1073.photobucket.com/albums/w385/cest_bastien1/Lightsaber/AS2_LEDstringSaberArduino_NeskweekRevised_zpsu5k0ljck.png)

#### 5. Tweak your install (optionnal) 

* If you're an RGB led user
comment:
```c++
#define LEDSTRINGS 
``` 

* If you're an Single led use

	1. Wire your LED on pin D3
	
	2. Comment this line :
	
```c++ 
#define LEDSTRINGS 
```        
: : : : : : :iii.Modify the following lines so all variables are set to 0 :    

```c++         
#ifdef LUXEON
		storage.mainColor = 0;
		storage.clashColor = 0;
		storage.soundFontColorPreset[2][0] = 0;
		storage.soundFontColorPreset[2][1] = 0;
		storage.soundFontColorPreset[3][0] = 0;
		storage.soundFontColorPreset[3][1] = 0;
#endif
```

* Tweaks 
```c++
#define CLICK				5    // ms you need to press a button to be a click
#define PRESS_ACTION		200  // ms you need to press a button to be a long press, in action mode
#define PRESS_CONFIG		400  // ms you need to press a button to be a long press, in config mode
/* MAX_BRIGHTNESS
 * Maximum output voltage to apply to LEDS
 * Default = 200 (78,4%) Max=255 Min=0(Off)
 * WARNING ! A too high value may burn your leds. Please make your maths !
 */
#define MAX_BRIGHTNESS		200
```

* To win some hex file size, or when you will want to use your saber in normal day to day use, comment this line: 
```c++
#define LS_INFO
```


#### 6. Upload the sketch to your arduino.
___Will only work with Arduino IDE v1.6.5 !!!___
Arduino v1.6.7 will generate errors.

#### 7. Enjoy


## How it works :

In standby mode (idle) : 

* Short press on main button : activate your saber
* Long press on lockup button : activate config mode


In Action Mode :

* Long press lockup button : Lockup
* Short press lockup button : Clash
* Long press main button : shutdown saber


In config Mode :

* short press main button : Up the value
* short press lockup button : Down the value
* long press main button : Change menu :  
	* volume
	* soundfont 
	* swing sensitivity : adjust swing sensitivity.
	* [ONLY FOR RGB LED USERS] Main color : change the color of your saber
	* [ONLY FOR RGB LED USERS] Clash color: change the color displayed during clash effect 
	* [ONLY FOR RGB LED USERS] Assign colors to current soundfont ? : Allows you to save the colors you just defined to the selected soundfont
	* clash acceleration sensitivity : trigger clash detection algorythm. The higher the value, the less sensible it is
	* clash brake sensitivity : trigger clash sound.The higher the value, the more sensible it is
* Long press lockup button : update config to EEPROM and leave config mode


If you want to add a soundfont, create a new folder (named 004 for instance), put your soundfiles in it (don't put gaps in numbering files) and take a look at SoundFont.h file


## License:

This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-sa/4.0/]http://creativecommons.org/licenses/by-nc-sa/4.0/ .

## Notes :

1. __NOOOO MP3, NOOOO WMA !!!__  Only WAVs !!! : when encoding a sound to those format (MP3 and WMA) the encoder will automaticly put a silence at start. WE DON'T WANT GAPS !!!  
2. With soundfiles with hum extension (you edited a swing file and paste a hum sound repeated for some time), if you put 2 min of hum after your swing sound, if you don't move your saber for 2 min (higly unprobable in real situation) you'll notice a little gap in hum sound at that moment : You've just switch on a pure hum soundfile.
3. Don't put gaps in soundfile numbering ex.:001_Boot.wav,002.wav, 0010_Swing1.wav... The "folder play" command of the DFPlayer will see them as 001, 002, 003... That will generate unpredictable behaviour.
4. You can't put more than 255 folders numbered folder on your SDCard, including O1 (which contains config mode sounds)
5. You can't put more than 255 files in a folder on your SDCard.
6. You won't be able to have more than 65535 sound file on the wole SDCard (including config sounds).
7. Don't put names after folder number (ex.: 001_Config or 002_Sith). Your folder won't be detected (tested :( )
8. Since I've developped it on a breadboard, clash and swings settings may need some more tweaking. Still hopping that will not be the case  :P. I've developped those wanting to obtain "real life" saber feel.
9. Beware  the amount of debug settings you uncoment: they add significant amount of data to the compile.

## TODO :
By priority :

* Better README.md
* Find a way to make hum relaunch unoticable - Possible with our hardware ? it would require to launch a sound _as soon as_ we receive an "END_PLAY" response from DFPlayer :
	* timer ijnterrupts tested with no success.(I might have not use them properly)
	* SoftwareSerial seems to already use interrupts to monitor DFPLAYER_TX we receive(I'm not sure of that). The best solution would be to use this interrupt to capture an "END_PLAY" response but I don't want to modify SoftwareSerial Lib
* Try to reduce compiled hex file to be able to add  more functionnality and/or handling of more modules (IMU/Music players).
* Make powerOn/powerOff ledstring effect sync to soundfont soundfiles play time.
	* Add 2 time-variables to setup in soundfont.h
* Find a use  to:
	* short press lockup button in standby mode 
	* double click on main button in action/config/standby mode
	* double clik on lockup button in action/config/standby mode.


## Videos
* [Quick tour (made under v1.0 RC4)] (https://youtu.be/mc8scn_qyFM)
* [Motion Detection Demo (made under v1.0 RC5)] (https://www.youtube.com/watch?v=wY8BSSEyYLY)


## Known Bug :
* In config mode, when chosing a new SoundFont, on some occasions, LSOS doesn't play the boot sound like it should... I can't find why


## Thanks

Thanks to Protonerd from Arduino Forum for initiating this project.

Thanks to Jakesoft from Arduino Forum for is initial ideau of using arduino device to build a lightsaber.






I hope you'll like it.

Don't hesitate to reports bugs (I've made a lot of test but... hey !), or suggest new functionallity.

If you want to contribute to this project, please contact me via mail or on the [thread of this project] (http://forum.arduino.cc/index.php?topic=361566.0)
