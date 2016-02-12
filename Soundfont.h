/*
 * Soundfont.h
 *
 *  Created on: 10 févr. 2016
 *      Author: neskw
 */

#ifndef SOUNDFONT_H_
#define SOUNDFONT_H_

/*
 * LightSaberOS V1.0RC1
 * author: 		Sébastien CAPOU (neskweek@gmail.com)
 * Source : 	https://github.com/neskweek/LightSaberOS.
 * Date: 		2016-02-11
 * Description:	Operating System for Arduino based LightSaber
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-sa/4.0/.
 */
#include <LinkedList.h>

class SoundFont {

#define SOUNDFONT_QUANTITY 2

public:

	SoundFont() {

		boot = LinkedList<int>();
		powerOn = LinkedList<int>();
		powerOff = LinkedList<int>();
		hum = LinkedList<int>();
		swing = LinkedList<int>();
		clash = LinkedList<int>();
		lockup = LinkedList<int>();
		blaster = LinkedList<int>();

	}
	;
	~SoundFont() {
		/*
		 delete folder;
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

	void setFolder(int folder) {
		int boot[2];
		int powerOn[2];
		int powerOff[2];
		int hum[2];
		int swing[2];
		int clash[2];
		int lockup[2];
		int blaster[2];

		this->folder = folder;

		switch (folder) {
		/*
		 case EXAMPLE:
		 // soundFont directory XX :
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
			// If you specify a folder number not defined here you will end up
			// on the first defined soundfont
		case 2:
			// soundFont directory 02 :

			boot[0] = 1;
			boot[1] = 1;
			powerOn[0] = 2;
			powerOn[1] = 2;
			powerOff[0] = 3;
			powerOff[1] = 3;
			hum[0] = 4;
			hum[1] = 4;
			swing[0] = 6;
			swing[1] = 17;
			clash[0] = 18;
			clash[1] = 30;
			lockup[0] = 5;
			lockup[1] = 5;
			blaster[0] = 30;
			blaster[1] = 30;
			break;
		case 3:
			boot[0] = 1;
			boot[1] = 1;
			powerOn[0] = 2;
			powerOn[1] = 2;
			powerOff[0] = 3;
			powerOff[1] = 3;
			hum[0] = 4;
			hum[1] = 4;
			swing[0] = 6;
			swing[1] = 13;
			clash[0] = 14;
			clash[1] = 21;
			lockup[0] = 5;
			lockup[1] = 5;
			blaster[0] = 22;
			blaster[1] = 22;
			break;
		}
		this->boot.clear();
		this->powerOn.clear();
		this->powerOff.clear();
		this->hum.clear();
		this->swing.clear();
		this->clash.clear();
		this->lockup.clear();
		this->blaster.clear();
		fill(&this->boot, boot);
		fill(&this->powerOn, powerOn);
		fill(&this->powerOff, powerOff);
		fill(&this->hum, hum);
		fill(&this->swing, swing);
		fill(&this->clash, clash);
		fill(&this->lockup, lockup);
		fill(&this->blaster, blaster);

	}

	int getFolder() const {
		return folder;
	}

	const int getBlaster() {
		return this->blaster.get(random(0, this->blaster.size()));
	}

	const int getBoot() {
		return this->boot.get(random(0, this->boot.size()));
	}

	const int getClash() {
		return this->clash.get(random(0, this->clash.size()));
	}

	const int getHum() {
		return this->hum.get(random(0, this->hum.size()));
	}

	const int getLockup() {
		return this->lockup.get(random(0, this->lockup.size()));
	}

	const int getPowerOff() {
		return this->powerOff.get(random(0, this->powerOff.size()));
	}

	const int getPowerOn() {
		return this->powerOn.get(random(0, this->powerOn.size()));
	}

	const int getSwing() {
		return this->swing.get(random(0, this->swing.size()));
	}

private:
	int folder;
	LinkedList<int> boot;
	LinkedList<int> powerOn;
	LinkedList<int> powerOff;
	LinkedList<int> hum;
	LinkedList<int> swing;
	LinkedList<int> clash;
	LinkedList<int> lockup;
	LinkedList<int> blaster;

	void fill(LinkedList<int>* list, int array[]) {
		for (int i = array[0]; i <= array[1]; i++) {
			list->add(i);
		}
	}

};

#endif /* SOUNDFONT_H_ */
