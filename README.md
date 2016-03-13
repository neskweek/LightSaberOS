# LightSaberOS

________________________________________________________________________________   

## VIDEOS

[![DIYino demo of Neopixel lightsaber blade with LSOS] (http://img.youtube.com/vi/lyk8riXgIzM/0.jpg)](http://www.youtube.com/watch?v=lyk8riXgIzM)[![DIYino prototype demo with LSOS by neskweek - Swings] (http://img.youtube.com/vi/tU3GZzV9I6E/0.jpg)](http://www.youtube.com/watch?v=tU3GZzV9I6E)[![Motion Detection Demo (made under v1.0 RC5)] (http://img.youtube.com/vi/wY8BSSEyYLY/0.jpg)](http://www.youtube.com/watch?v=wY8BSSEyYLY)[![Quick tour (made under v1.0 RC4)] (http://img.youtube.com/vi/mc8scn_qyFM/0.jpg)](http://www.youtube.com/watch?v=mc8scn_qyFM)

________________________________________________________________________________   

### FEATURES :

* Simple modular systems (you can choose to remove some systems from your final compilation)
* Swing detection
* Blade Clash detection  
* Wrist movement detection 
* Full gapless sound playing with "Hum Extended" soundfiles (see below)
* Blaster Shot deflect 
* Blade Lockup (long press on Aux switch. Plays until release of the button)
* Accent LED
* Flash-on-Clash LEDString
* Multiple ignition/retractation/flickering effects
* Config Menu to modify some features without wiring the device to your PC/Mac 
* EEPROM load/save of your config preferences
* Soundfont adding supported (not automatic, you'll have little work to do ;) )
* Low battery level warnings
* Shutdown on critical battery level 
* Automatic Powersaving mode when idle

________________________________________________________________________________   

### WHAT YOU WILL NEED :
* [Java Runtime Environment 8](http://www.oracle.com/technetwork/java/javase/downloads/jre8-downloads-2133155.html) Download the right file for your architecture
* [Arduino Eclipse v.2016-03-11_03-29-38](http://www.baeyens.it/eclipse/download.php) Again, download the right file for your architecture
* my new DFPlayer library (Included in zip file)
* [I2Cdev and MPU6050 (Included in zip file)](https://codeload.github.com/jrowberg/i2cdevlib/zip/master) 
* [EEPROMex9.1 (Included in zip file)](http://thijs.elenbaas.net/wp-content/uploads/downloads/2013/12/EEPROMEx-9.1.zip)
* [OneButton (Included in zip file)] (https://github.com/mathertel/OneButton)
* [LinkedList (Included in zip file)] (https://github.com/ivanseidel/LinkedList)
* Wire (Included in Arduino Eclipse) 

________________________________________________________________________________   

### DEVICE CURRENTLY SUPPORTED :

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
   
________________________________________________________________________________   

## SET UP YOUR PC:
1. Download and install [Java Runtime Environment 8](http://www.oracle.com/technetwork/java/javase/downloads/jre8-downloads-2133155.html)   
Download the right file for your OS   
2. Download [Arduino Eclipse v.2016-03-11_03-29-38](http://www.baeyens.it/eclipse/download.php)      
Again, download the right file for your OS   
3. Uncompress this archive inside c:\Program Files   
4. Start C:\Program Files\eclipseArduino\eclipseArduinoIDE.exe    
On first start up you will be ask where to put your prject workspace.   
I advise you to put it there (off course replace "neskw" by your username) :   
![Set workspace destination](https://raw.githubusercontent.com/neskweek/LightSaberOS/master/README/eclipse1.PNG)   
On first startup it will download a bunch of stuff related to Arduino Libraries.   
Wait until it finishes   
5. Go to Windows > Preferences      
6. Select Arduino and set "Build before upload ?" to "Yes" and press OK   
7. Plug in your Arduino device    
It would be best that you always use the same USB port for future use    
8. Go to File > New > Arduino sketch     
![Project Name](https://raw.githubusercontent.com/neskweek/LightSaberOS/master/README/eclipse2.PNG)   
Then press next   
![Fill in your board Info](https://raw.githubusercontent.com/neskweek/LightSaberOS/master/README/eclipse3.PNG)   
COM port may be different on your PC. It depends on USB port you plugged it in.   
Then press next   
![Optional Check AVaRICE](https://raw.githubusercontent.com/neskweek/LightSaberOS/master/README/eclipse4.PNG)   
Here Checking AVaRICE is optional   
Then press Finish   
9. Delete LightSaberOS.ino file   

________________________________________________________________________________    

## IMPORT GITHUB FILES INSIDE ECLIPSE PROJECT:

1. Uncompress LightSaberOS-master.zip archive
2. If not already done create this directory : C:\Users\neskw\Arduino   
3. Copy and Paste LightSaberOS-master\Libraries directory inside C:\Users\neskw\Arduino   
4. Inside Eclipse Right click on LightSaberOS project  > Import    
![Import source files](https://raw.githubusercontent.com/neskweek/LightSaberOS/master/README/eclipse5.PNG)   
Then press Next  
![Select source directory](https://raw.githubusercontent.com/neskweek/LightSaberOS/master/README/eclipse6.PNG)   
Then press Next   
In "Eclipse Project explorer" open the new created LightSaberOS-master and select those files :   
![Select source directory](https://raw.githubusercontent.com/neskweek/LightSaberOS/master/README/eclipse7.PNG)   
Then drag and drop them to the root structure of the project   
5. Inside Eclipse Right click on LightSaberOS project  > Import   
![Import Libraries](https://raw.githubusercontent.com/neskweek/LightSaberOS/master/README/eclipse8.PNG)   
Then press Next    
Select those libraries :     
![Select Libraries](https://raw.githubusercontent.com/neskweek/LightSaberOS/master/README/eclipse9.PNG)   
Then press Finish    
6. Delete LightSaberOS-master directory from Eclipse Project explorer    
    
   
You should end up with this Project explorer structure :   
![Project structure](https://raw.githubusercontent.com/neskweek/LightSaberOS/master/README/eclipse10.PNG)   

If so, Then you're ready to rock !   

________________________________________________________________________________    
   
   
 
### SETUP PROJECT    

#### 1. IMU calibration 
 First, you'll need (if not already done) to calibrate your MPU6050.   
 [I recommend you use the AutoCalibration script you can find here](http://www.i2cdevlib.com/forums/topic/96-arduino-sketch-to-automatically-calculate-mpu6050-offsets/)  
 Note the offset values it will give you and replace those you'll find inside setup() function  in __Lightsaber.ino__ : 
```c++
	/*
	 * Those offsets are specific to each MPU6050 device.
	 * they are found via calibration process.
	 * See this script http://www.i2cdevlib.com/forums/index.php?app=core&module=attach&section=attach&attach_id=27
	 */
	mpu.setXAccelOffset(-2645);
	mpu.setYAccelOffset(-5491);
	mpu.setZAccelOffset(3881);
	mpu.setXGyroOffset(27);
	mpu.setYGyroOffset(-135);
	mpu.setZGyroOffset(-38);
```

#### 2. Determine IMU orientation
 The way you physically installed the MPU6050 in your hilt will influence how swing detection works.  
 You'll have to determine which IMU's axis is parallel to blade axis and change it accordingly in __Config.h__ :   
```c++
//#define BLADE_X
#define BLADE_Y
//#define BLADE_Z
```


#### 3. Prepare your SDCard
Then, put the content of _SDCard.7z_ on your SDCard:  

1. Format your SDCard. __I insist !__  (go read __HOW TO MANAGE YOUR SDRCARD__ section for further explanation)
2. Unzip SDCard.7z to a folder
3. Select all the files from this folder and __"Drag and Drop"__ them to your SDCard. __NO COPY AND PASTE !!!__ :
We need to have this file copied in the same order as their filename order. On Microsoft Windows, Copy/paste produce an anarchic copy order, but Drag and Drop produce an ordered copy...  


#### 4. Check Wirings
_If you use DIYno board, don't mind that paragraph._   

Using  **Protonerd's** wirings.    
Don't forget to wire those ones which were added :  
* DFPLAYER TX to D7
* DFPLAYER SPK+ to A0
* DFPLAYER SPK- to A1     
![Schematics](http://i1073.photobucket.com/albums/w385/cest_bastien1/Lightsaber/AS2_LEDstringSaberArduino_NeskweekRevised_zpsu5k0ljck.png)    
Wiring of busy pin is optional since LightSaberOS doesn't use it.  



 











#### 5. Select your emitter type


###### A.LEDSTRINGS 
In __Config.h__  be sure this line is uncommented :
```c++
#define LEDSTRINGS 
```   

###### B. RGB LEDs 
In __Config.h__  comment this line :
```c++
#define LEDSTRINGS 
``` 

###### C. Single LED
* a) Wire your LED on pin D3
	
* b) In __Config.h__  comment this line :
	
```c++ 
#define LEDSTRINGS 
```        
* c) Modify the following lines  in __LightSaber.ino__ so all variables are set to 0 :    

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

#### 6. Compile and Upload the sketch to your Arduino device

#### 7. Enjoy

________________________________________________________________________________   

## HOW IT WORKS :

###### _In Standby Mode (idle)_ :  
* Short press on Main switch : activate your saber (Action Mode)
* Long press on Aux switch : activate Config Mode
* if idle for more than 5 min : PowerSaving mode. 

###### _In PowerSaving Mode (idle)_ :  
* Short press on Main switch or Aux switch : leave PowerSaving Mode (go back to Standby Mode)


###### _In Action Mode_ :
* Move your saber around : Swing effect
* Hit the hilt/blade : Clash effect
* Short press Aux switch : Enable/disable Blaster block modes. Move your saber to produce Blaster effects.
* Long press Aux switch : Blade Lockup effect
* Long press Main switch : Shutdown saber




###### _In Config Mode_ :  
* short press Main switch : Up the value
* short press Aux switch : Down the value
* long press Main switch : Change menu :  
	* Volume
	* SoundFont 
	* [ONLY FOR LEDSTRING USERS] Power On effect: change the type of ignition for the current SoundFont
	* [ONLY FOR LEDSTRING USERS] Power Off effect: change the type of retractation for the current SoundFont
	* [ONLY FOR LEDSTRING USERS] Flicker effect: change the type of flickering for the current SoundFont
	* [ONLY FOR RGB LED USERS] Main color : change the color of your saber
	* [ONLY FOR RGB LED USERS] Clash color: change the color displayed during clash effect 
	* [ONLY FOR RGB LED USERS] Assign colors to current soundfont ? : Allows you to save the colors you just defined to the current SoundFont
	* Swing sensitivity : adjust swing sensitivity.

* Long press Aux switch : update config to EEPROM and leave Config Mode

________________________________________________________________________________  

### HOW TO MANAGE YOUR SDCRAD
#### 1. For DFPlayer Mini

While there are several ways to make DFPlayer Mini plays your sound files from your SDard, only one can provide full gapless sound "enqueuing" : we need to call each sound file by their copy order number.    

Let's say you have a fresh formatted SDcard.   
You copy one file on it : its copy order is 1.   
You copy another one: its copy order is 2. 
You copy another one: its copy order is 3.   

Now the main "problem" to be aware of is : you remove file 2 (because you maybe want to replace it) : file 3 keep is order number.
You copy another one: its copy order is 4.    

In that case the only proper way to manage your SDCard I found is he following :    
1. You create a folder called _sdcard_ on your computer    
2. Using a batch renamer utility (I personnaly use [Bulk Rename Utility](http://www.bulkrenameutility.co.uk/Main_Intro.php)) you rename and copy your sound files to that folder in the order you planned     
3. You format your SDCard   
4. You select all your files from _sdcard_ directory and __DRAG AND DROP__ them to your SDCard.   

Drag and drop on windows genereate a ordered copy following file name sorting.    
Copy and paste (Crtl+C/Ctrl+V) generate an anarchic ordered copy. Don't use it !  

Each time you will want to modify one file you'll have to do it that way. Adding files on he other hand can be done without formatting.   

Files MUST be named that way :   
_XXXX[-myfileName].wav_   
where :   
* XXXX is the order number planned (ex.: 0001, 0002, 2568,...)    
* [-myfileName] is OPTIONNAL. But I encourage you to put one to ease up organisation.    
__YOU MUST ABSOLUTLY AVOID ALL GAPS IN THE ORDER NUMBER YOU PLANNED !!!__    

You can put files in different folders if you like. You don't necessarily need to let them on SDCard root.     
__BUT STILL BEWARE OF THOSE NUMBERS !!!!__   
Here are different examples of organisation you can use :

* All in SDCard root (like SDCard.7z example provided)   
* 1 folder for each SoundFonts. For example :    
	* **_CONFIG_** folder containing files numbered 0001 to 0020
	* **_Barlow's_** folder containing files numbered 0021 to 0053
	* **_MySoundfont_** folder containing files numbered 0079 to 0124
	* **_ZZZZZ_** folder containing files numbered 0054 to 0078
In this example files in ZZZZZ have been copied BEFORE those in MySoundfont
* an other example of organisation :
	* **_CONFIG_** folder containing files numbered 0001 to 0020
	* **_Barlow's_** folder containing files  0021-PowerOn.wav, 0022-PowerOff.wav,0023-Hum.wav,0024-Boot.wav
	* **_Swing_** folder containing only swing sound files numbered 0025 to 1075
	* **_Clash_** folder containing only clash sound files numbered 1076 to 1078
	* **_Blaster_** folder containing only blaster sound files numbered 1079 to 2001
	* **_Lockup_** folder containing only lockup sound files numbered 2002 to 2024   
	* **_MySoundFont_** folder containing files numbered 2025-Hum.wav, 2026-Boot.wav 2027-PowerOff.wav,2028-PowerOn.wav      
In that case, the user wants to be able to use all of his swing/clash/blaster/lockup/etc. sounds with every of its soundfont.   
He can also configure some soundfont to use just a part like for example Swings 0100 to 0153.   

________________________________________________________________________________ 

### HOW TO PERSONNALIZE CONFIG MENU SOUNDS

Let's say you want to change the lady voice or ladies usres may want their saber with a male voice or you may want her to talk in Dutch or French or Italian or Spanish (name your langage here).    


I used [this online free Text-To-Speech generator](http://www.fromtexttospeech.com/) to produce those sounds.   
You can produce your own sentences with it or use any other Text-To-Speech program.  
For example :    
"Hello Master! You just entered into Config Mode !"    
Then rename the file it produces : 0001-CONFIG-ConfigMode.mp3. Remove old 0001-CONFIG-ConfigMode.wav file.    


**_N.B.:_** _Only in Config Mode case_, mp3 are OK to use. We don't care in this mode if there's sound gap.

________________________________________________________________________________ 

### HOW TO MAKE YOUR OWN SOUNDFONT

[In this video](https://www.youtube.com/watch?v=Q_6VITJT0-w), you'll learn how to create the differents lightsabers sounds-effects.   

You can experiment with different device to produce some unique sounds.   

Then use Audacity to mix them.    

Final sounds **MUST BE** in WAV format (**_NO MP3 ! NO WMA!_**)!   
When encoding a sound to those format (MP3 or WMA) the encoder will automaticly put a silence at start.
________________________________________________________________________________ 

## License:

This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.  
To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-sa/4.0/]http://creativecommons.org/licenses/by-nc-sa/4.0/ .

________________________________________________________________________________   

## TODO :
By priority :
* NEOPIXEL code integration
* Better README.md (Work in progress)
* Try to reduce compiled hex file 
* Rewrite to Object Oriented form, using JakeSoft Library
* Find a use to:
	* short press Aux switch in standby mode 
	* double click on Main switch in action/config/standby mode
	* double clik on Aux switch in action/config/standby mode.
* Rewrite some function in Assembler
* Pitch shifting on movements (don't know if it's feasable)

________________________________________________________________________________   

## THANK YOU !!!

Thanks to Andras Kun (__Protonerd__ from Arduino Forum) for initiating this project and his big contributions to LSOS Code    
Thanks to __Jakesoft__ from Arduino Forum for :    
* his initial idea of using Arduino device to build a lightsaber.   
* his idea of using Aux Switch as a triger mode for Blaster Blocks.   
and many more source of inspirations.

Thanks to [__Joe Barlow__](https://www.freesound.org/people/joe93barlow/) for his excellent opensource soundfont that I did remix for our needs.    
Thanks to __Scott Daniels__ for his [code that allows to monitor the battery charge] (http://provideyourown.com/2012/secret-arduino-voltmeter-measure-battery-voltage/)    
Thanks to __you__ for using it ;)

________________________________________________________________________________   

## WITH YOUR HELP...
... a real life  lightsaber which cut through stuffs, like a pizza, could be made...    
But first things first : I need the pizza !   
[![paypal](https://www.paypalobjects.com/en_US/i/btn/btn_donateCC_LG.gif)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=GX6M3586YU8JL)   
;)   
________________________________________________________________________________   

## FINAL THOUGHTS

I hope you'll like it.

Don't hesitate to reports bugs (I've made a lot of test but... hey !), or suggest new functionallity.  
If you want to contribute to this project, please contact me via mail or on the [thread of this project] (http://forum.arduino.cc/index.php?topic=361566.0)  
________________________________________________________________________________    
________________________________________________________________________________    
________________________________________________________________________________    
________________________________________________________________________________    
________________________________________________________________________________    
   
________________________________________________________________________________    
________________________________________________________________________________   
   

##12/03/2016 : The following might not be accurate since release of version 1.0  
## Modification of this Readme is a "work in progress"
## More info regarding LSOS will be added
## Thank you for your patience
________________________________________________________________________________    
________________________________________________________________________________    
________________________________________________________________________________    
________________________________________________________________________________    
________________________________________________________________________________    
   
________________________________________________________________________________    
________________________________________________________________________________   

### MODULES

________________________________________________________________________________ 

### HOW TO ADD A SOUNDFONT

Edit SoundFont.h

________________________________________________________________________________ 


### NOTES :

1. __Only WAVs file !!!__ _NOOOO MP3, NOOOO WMA !!!_   :     
When encoding a sound to those format (MP3 and WMA) the encoder will automaticly put a silence at start.    
___WE DON'T WANT GAPS !!!___  
2. With soundfiles with hum extension (you edited a swing file and paste a hum sound repeated for some time), if you put 2 min of hum after your swing sound, if you don't move your saber for 2 min (higly unprobable in real situation) you'll notice a little gap in hum sound at that moment : You've just switch on a pure hum soundfile.
3. Don't put gaps in soundfile numbering ex.:001_Boot.wav,002.wav, 0010_Swing1.wav...    
The "folder play" command of the DFPlayer will see them as 001, 002, 003...    
That will generate unpredictable behaviour.
4. You can't put more than 255 folders numbered folder on your SDCard, including O1 (which contains config mode sounds)
5. You can't put more than 255 files in a folder on your SDCard.
6. You won't be able to have more than 65535 sound file on the wole SDCard (including config sounds).
7. Don't put names after folder number (ex.: 001_Config or 002_Sith). Your folder won't be detected (tested :( )
8. Since I've developped it on a breadboard, clash and swings settings may need some more tweaking.    
Still hopping that will not be the case  :P. I've developped those wanting to obtain "real life" saber feel.
9. Beware  the amount of debug settings you uncoment: they add significant amount of data to the compile.   
I've made them modular for this reason, so take advantage of it.

________________________________________________________________________________ 

### LECTURES / SOURCES /INSPIRATIONS

  





