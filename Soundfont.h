/*
 * Soundfont.h
 *
 * Created on: 	21 Octber 2016
 * author: 		Sebastien CAPOU (neskweek@gmail.com) and Andras Kun (kun.andras@yahoo.de)
 * Source : 	https://github.com/neskweek/LightSaberOS
 * Description:	Soundfont Config file for LightSaberOS
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-sa/4.0/.
 */
#include <LinkedList.h>

#ifndef SOUNDFONT_H_
#define SOUNDFONT_H_

class SoundFont {

#define SOUNDFONT_QUANTITY 5
#define NR_CONFIGFOLDERFILES 14
#define NR_JUKEBOXSONGS 5
#define NR_SF1_FILES 30
#define NR_SF2_FILES 30
#define NR_SF3_FILES 30
#define NR_SF4_FILES 30
#define NR_SF5_FILES 30

public:

	SoundFont() {

    boot = LinkedList<uint16_t>();
		powerOn = LinkedList<uint16_t>();
		powerOff = LinkedList<uint16_t>();
		hum = LinkedList<uint16_t>();
		swing = LinkedList<uint16_t>();
		clash = LinkedList<uint16_t>();
		lockup = LinkedList<uint16_t>();
		blaster = LinkedList<uint16_t>();
    menu = LinkedList<uint16_t>();
		ID=0;
		powerOnTime =500;
		powerOffTime = 500;
    powerOnEffect=0; //0: movie-like; 1: inverted
    powerOffEffect=0; //0: movie-like; 1: inverted
    flickerEffect=0; //0: standard; 1: pulse; 2: anarchic
    swingthreshold=300;

	}
	;
	~SoundFont() {
		/*
		 delete ID;
		 delete boot;
		 delete powerOn;
		 delete powerOff;
		 delete hum;
		 delete swing;
		 delete clash;
		 delete lockup;
		 delete blaster;
		 */
	}
	;

	void setID(uint16_t id) {
    uint16_t boot[2];
		uint16_t powerOn[2];
		uint16_t powerOff[2];
		uint16_t hum[2];
		uint16_t swing[2];
		uint16_t clash[2];
		uint16_t lockup[2];
		uint16_t blaster[2];
    uint16_t menu[2];

		this->ID = id;

		switch (id) {
		/*
		 case EXAMPLE:
		 // soundFont directory XX :
     menu[0] = 1;     // first menu sound file
     menu[1] = 1;     // last  menu sound file
     boot[0] = 1;  		// first boot sound file
		 boot[1] = 1;  		// last  boot sound file
		 powerOn[0] = 2;	// first powerOn sound file
		 powerOn[1] = 2;	// last  powerOn sound file
		 powerOff[0] = 3;	// first powerOff sound file
		 powerOff[1] = 3;	// last  powerOff sound file
		 hum[0] = 4;		// first hum sound file
		 hum[1] = 4;		// last  hum sound file
		 swing[0] = 6;		// first swing sound file
		 swing[1] = 17;		// last  swing sound file
		 clash[0] = 18;		// first clash sound file
		 clash[1] = 30;		// last  clash sound file
		 lockup[0] = 5;		// first lockup sound file
		 lockup[1] = 5;		// last  lockup sound file
		 blaster[0] = 0;	// first blaster sound file
		 blaster[1] = 0;	// last  blaster sound file
		 break;
		 */
    case 1:
      // soundFont directory 01 is reserved for config menu sounds
      break;
    default:
      // If you specify a ID number not defined here you will end up
      // on the first defined soundfont
    case 2:
      // soundFont directory 02 :
      this->powerOnTime = 800;
      this->powerOffTime = 800;
      this->powerOnEffect=0;
      this->powerOffEffect=0;
      this->flickerEffect=0;
      this->swingthreshold=700;
      boot[0] = NR_CONFIGFOLDERFILES + NR_JUKEBOXSONGS + 1; // 1 boot sound (1)
      boot[1] = boot[0];
      powerOn[0] = boot[1]+1; // 4 power-on sounds (2-5)
      powerOn[1] = powerOn[0]+3;
      powerOff[0] = powerOn[1]+1; // 2 power-off sounds (6-7)
      powerOff[1] = powerOff[0]+1;
      swing[0] = powerOff[1]+1; // 8 swing sounds (8-15)
      swing[1] = swing[0]+7;
      clash[0] = swing[1]+1; // 8 clash sounds (16-23)
      clash[1] = clash[0]+7;
      lockup[0] = clash[1]+1; // 1 lockup sound (24)
      lockup[1] = lockup[0];
      blaster[0] = lockup[1]+1; // 4 blaster deflect sound (25-28)
      blaster[1] = blaster[0]+3;
      menu[0] = blaster[1]+1; // 1 menu sound file (29)
      menu[1] = menu[0];
      // hum must be the last file in the sound font for proper hum relaunch
      hum[0] = menu[1]+1; // 1 hum relaunch sound (30)
      hum[1] = hum[0];
      break;
    case 3:
      this->powerOnTime = 1050;
      this->powerOffTime = 900;
      this->powerOnEffect=0;
      this->powerOffEffect=0;
      this->flickerEffect=1;
      this->swingthreshold=900;
      boot[0] = NR_CONFIGFOLDERFILES + NR_JUKEBOXSONGS + NR_SF1_FILES + 1;
      boot[1] = boot[0];
      powerOn[0] = boot[1]+1; // 4 power-on sounds (2-5)
      powerOn[1] = powerOn[0]+3;
      powerOff[0] = powerOn[1]+1; // 2 power-off sounds (6-7)
      powerOff[1] = powerOff[0]+1;
      swing[0] = powerOff[1]+1; // 8 swing sounds (8-15)
      swing[1] = swing[0]+7;
      clash[0] = swing[1]+1; // 8 clash sounds (16-23)
      clash[1] = clash[0]+7;
      lockup[0] = clash[1]+1; // 1 lockup sound (24)
      lockup[1] = lockup[0];
      blaster[0] = lockup[1]+1; // 4 blaster deflect sound (25-28)
      blaster[1] = blaster[0]+3;
      menu[0] = blaster[1]+1; // 1 menu sound file (29)
      menu[1] = menu[0];
      // hum must be the last file in the sound font for proper hum relaunch
      hum[0] = menu[1]+1; // 1 hum relaunch sound (30)
      hum[1] = hum[0];
      break;
   case 4:
      this->powerOnTime = 1200;
      this->powerOffTime = 1200;
      this->powerOnEffect=1;
      this->powerOffEffect=1;
      this->flickerEffect=2;
      this->swingthreshold=1100;
      boot[0] = NR_CONFIGFOLDERFILES + NR_JUKEBOXSONGS + NR_SF1_FILES + NR_SF2_FILES + 1;
      boot[1] = boot[0];
      powerOn[0] = boot[1]+1; // 4 power-on sounds (2-5)
      powerOn[1] = powerOn[0]+3;
      powerOff[0] = powerOn[1]+1; // 2 power-off sounds (6-7)
      powerOff[1] = powerOff[0]+1;
      swing[0] = powerOff[1]+1; // 8 swing sounds (8-15)
      swing[1] = swing[0]+7;
      clash[0] = swing[1]+1; // 8 clash sounds (16-23)
      clash[1] = clash[0]+7;
      lockup[0] = clash[1]+1; // 1 lockup sound (24)
      lockup[1] = lockup[0];
      blaster[0] = lockup[1]+1; // 4 blaster deflect sound (25-28)
      blaster[1] = blaster[0]+3;
      menu[0] = blaster[1]+1; // 1 menu sound file (29)
      menu[1] = menu[0];
      // hum must be the last file in the sound font for proper hum relaunch
      hum[0] = menu[1]+1; // 1 hum relaunch sound (30)
      hum[1] = hum[0];
      break;
    case 5:
      this->powerOnTime = 2500;
      this->powerOffTime = 2000;
      this->powerOnEffect=0;
      this->powerOffEffect=0;
      this->flickerEffect=2;
      this->swingthreshold=1300;
      boot[0] = NR_CONFIGFOLDERFILES + NR_JUKEBOXSONGS + NR_SF1_FILES + NR_SF2_FILES + NR_SF3_FILES + 1;
      boot[1] = boot[0];
      powerOn[0] = boot[1]+1; // 4 power-on sounds (2-5)
      powerOn[1] = powerOn[0]+3;
      powerOff[0] = powerOn[1]+1; // 2 power-off sounds (6-7)
      powerOff[1] = powerOff[0]+1;
      swing[0] = powerOff[1]+1; // 8 swing sounds (8-15)
      swing[1] = swing[0]+7;
      clash[0] = swing[1]+1; // 8 clash sounds (16-23)
      clash[1] = clash[0]+7;
      lockup[0] = clash[1]+1; // 1 lockup sound (24)
      lockup[1] = lockup[0];
      blaster[0] = lockup[1]+1; // 4 blaster deflect sound (25-28)
      blaster[1] = blaster[0]+3;
      menu[0] = blaster[1]+1; // 1 menu sound file (29)
      menu[1] = menu[0];
      // hum must be the last file in the sound font for proper hum relaunch
      hum[0] = menu[1]+1; // 1 hum relaunch sound (30)
      hum[1] = hum[0];
      break;
    case 6:
      this->powerOnTime = 700;
      this->powerOffTime = 1500;
      this->powerOnEffect=1;
      this->powerOffEffect=1;
      this->flickerEffect=2;
      this->swingthreshold=1500;
      boot[0] = NR_CONFIGFOLDERFILES + NR_JUKEBOXSONGS + NR_SF1_FILES + NR_SF2_FILES + NR_SF3_FILES + NR_SF4_FILES + 1;
      boot[1] = boot[0];
      powerOn[0] = boot[1]+1; // 4 power-on sounds (2-5)
      powerOn[1] = powerOn[0]+3;
      powerOff[0] = powerOn[1]+1; // 2 power-off sounds (6-7)
      powerOff[1] = powerOff[0]+1;
      swing[0] = powerOff[1]+1; // 8 swing sounds (8-15)
      swing[1] = swing[0]+7;
      clash[0] = swing[1]+1; // 8 clash sounds (16-23)
      clash[1] = clash[0]+7;
      lockup[0] = clash[1]+1; // 1 lockup sound (24)
      lockup[1] = lockup[0];
      blaster[0] = lockup[1]+1; // 4 blaster deflect sound (25-28)
      blaster[1] = blaster[0]+3;
      menu[0] = blaster[1]+1; // 1 menu sound file (29)
      menu[1] = menu[0];
      // hum must be the last file in the sound font for proper hum relaunch
      hum[0] = menu[1]+1; // 1 hum relaunch sound (30)
      hum[1] = hum[0];
      break;
    }
    this->boot.clear();
    this->powerOn.clear();
    this->powerOff.clear();
    this->swing.clear();
    this->clash.clear();
    this->lockup.clear();
    this->blaster.clear();
    this->menu.clear();
    this->hum.clear();
    fill(&this->boot, boot);
    fill(&this->powerOn, powerOn);
    fill(&this->powerOff, powerOff);
    fill(&this->swing, swing);
    fill(&this->clash, clash);
    fill(&this->lockup, lockup);
    fill(&this->blaster, blaster);
    fill(&this->menu, menu);
    fill(&this->hum, hum);
  }

	uint16_t getID() const {
		return this->ID;
	}

	const uint16_t getBlaster() {
		return this->blaster.get(random(0, this->blaster.size()));
	}

	const uint16_t getBoot() {
		return this->boot.get(random(0, this->boot.size()));
	}

	const uint16_t getClash() {
		return this->clash.get(random(0, this->clash.size()));
	}

	const uint16_t getHum() {
		return this->hum.get(random(0, this->hum.size()));
	}

	const uint16_t getLockup() {
		return this->lockup.get(random(0, this->lockup.size()));
	}

	const uint16_t getPowerOff() {
		return this->powerOff.get(random(0, this->powerOff.size()));
	}

	const uint16_t getPowerOn() {
		return this->powerOn.get(random(0, this->powerOn.size()));
	}

	const uint16_t getSwing() {
		return this->swing.get(random(0, this->swing.size()));
	}

  const uint16_t getMenu() {
    return this->menu.get(random(0, this->menu.size()));
  }
  
	uint16_t getPowerOffTime() const {
		return powerOffTime;
	}

	uint16_t getPowerOnTime() const {
		return powerOnTime;
	}

  uint16_t getPowerOnEffect() const {
    return powerOnEffect;
  }
  
  uint16_t getPowerOffEffect() const {
    return powerOffEffect;
  }
  
  uint16_t getFlickerEffect() const {
    return flickerEffect;
  }
  
  uint16_t getSwingThreshold() const {
    return swingthreshold;
  }
  

private:
	uint16_t ID;
	LinkedList<uint16_t> boot;
	LinkedList<uint16_t> powerOn;
	uint16_t powerOnTime;
	LinkedList<uint16_t> powerOff;
	uint16_t powerOffTime;
	LinkedList<uint16_t> hum;
	LinkedList<uint16_t> swing;
	LinkedList<uint16_t> clash;
	LinkedList<uint16_t> lockup;
	LinkedList<uint16_t> blaster;
  LinkedList<uint16_t> menu;
  uint16_t powerOnEffect;
  uint16_t powerOffEffect;
  uint16_t flickerEffect;
  uint16_t swingthreshold;


	void fill(LinkedList<uint16_t>* list, uint16_t array[]) {
		for (uint16_t i = array[0]; i <= array[1]; i++) {
			list->add(i);
		}
	}
};

#endif /* SOUNDFONT_H_ */
