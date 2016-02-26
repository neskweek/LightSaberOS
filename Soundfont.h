/*
 * Soundfont.h
 *
 * Created on: 	19 feb 2016
 * author: 		Sebastien CAPOU (neskweek@gmail.com)
 * Source : 	https://github.com/neskweek/LightSaberOS
 * Description:	Soundfont Config file for LightSaberOS
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-sa/4.0/.
 */

#ifndef SOUNDFONT_H_
#define SOUNDFONT_H_

struct wav {
	uint8_t id;    //Wav id (ex :001 <= 001_Boot.wav)
	uint16_t time; //how long this sound last in milliseconds
};

class SoundFont {

#define SOUNDFONT_QUANTITY 2

public:

	SoundFont() {

		boot = LinkedList<wav*>();
		powerOn = LinkedList<wav*>();
		powerOff = LinkedList<wav*>();
		hum = LinkedList<wav*>();
		swing = LinkedList<wav*>();
		clash = LinkedList<wav*>();
		lockup = LinkedList<wav*>();
		blaster = LinkedList<wav*>();
		wrist = LinkedList<wav*>();
		force = LinkedList<wav*>();

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

	void setFolder(uint8_t folder) {
		uint8_t boot[2];
		uint8_t powerOn[2];
		uint8_t powerOff[2];
		uint8_t hum[2];
		uint8_t swing[2];
		uint8_t clash[2];
		uint8_t lockup[2];
		uint8_t blaster[2];
		uint8_t wrist[2];
		uint8_t force[2];

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
			clash[1] = 29;
			lockup[0] = 5;
			lockup[1] = 5;
			blaster[0] = 30;
			blaster[1] = 30;
			wrist[0] = 32;
			wrist[1] = 32;
			force[0] = 31;
			force[1] = 31;
			break;
		case 3:
			this->boot.add(new wav( { 1, 3508 }));
			this->powerOn.add(new wav( { 2, 1739 }));
			this->powerOff.add(new wav( { 3, 1697 }));
			this->hum.add(new wav( { 4, 4218 }));
			this->lockup.add(new wav( { 5, 9258 }));
			this->swing.add(new wav( { 6, 610 }));
			this->swing.add(new wav( { 7, 610 }));
			this->swing.add(new wav( { 8, 522 }));
			this->swing.add(new wav( { 9, 485 }));
			this->swing.add(new wav( { 10, 421 }));
			this->swing.add(new wav( { 11, 520 }));
			this->swing.add(new wav( { 12, 610 }));
			this->swing.add(new wav( { 13, 648 }));
			this->clash.add(new wav( { 14, 307 }));
			this->clash.add(new wav( { 15, 570 }));
			this->clash.add(new wav( { 16, 570 }));
			this->clash.add(new wav( { 17, 715 }));
			this->clash.add(new wav( { 18, 640 }));
			this->clash.add(new wav( { 19, 570 }));
			this->clash.add(new wav( { 20, 845 }));
			this->clash.add(new wav( { 21, 845 }));
			this->blaster.add(new wav( { 22, 1535 }));
			this->wrist.add(new wav( { 23, 14171 }));
			this->force.add(new wav( { 24, 609 }));
			/*
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
			 wrist[0] = 24;
			 wrist[1] = 24;
			 force[0] = 23;
			 force[1] = 23;
			 */
			break;
		}
		/*
		if (this->folder != 3) {
			this->boot.clear();
			this->powerOn.clear();
			this->powerOff.clear();
			this->hum.clear();
			this->swing.clear();
			this->clash.clear();
			this->lockup.clear();
			this->blaster.clear();
			this->wrist.clear();
			this->force.clear();

			fill(&this->boot, boot);
			fill(&this->powerOn, powerOn);
			fill(&this->powerOff, powerOff);
			fill(&this->hum, hum);
			fill(&this->swing, swing);
			fill(&this->clash, clash);
			fill(&this->lockup, lockup);
			fill(&this->blaster, blaster);
			fill(&this->wrist, wrist);
			fill(&this->force, force);
		}
		*/
	}

	uint8_t getFolder() const {
		return folder;
	}

	wav* getBlaster() {
		return this->blaster.get(random(0, this->blaster.size()));
	}

	wav* getBoot() {
		return this->boot.get(random(0, this->boot.size()));
	}

	wav* getClash() {
		return this->clash.get(random(0, this->clash.size()));
	}

	wav* getHum() {
		return this->hum.get(random(0, this->hum.size()));
	}

	wav* getLockup() {
		return this->lockup.get(random(0, this->lockup.size()));
	}

	wav* getPowerOff() {
		return this->powerOff.get(random(0, this->powerOff.size()));
	}

	wav* getPowerOn() {
		return this->powerOn.get(random(this->powerOn.size()));
	}

	wav* getSwing() {
		return this->swing.get(random(0, this->swing.size()));
	}

	wav* getForce() {
		return this->force.get(random(0, this->force.size()));
	}
	wav* getWrist() {
		return this->wrist.get(random(this->wrist.size()));
	}

private:
	uint8_t folder;
	LinkedList<wav*> boot;
	LinkedList<wav*> powerOn;
	LinkedList<wav*> powerOff;
	LinkedList<wav*> hum;
	LinkedList<wav*> swing;
	LinkedList<wav*> clash;
	LinkedList<wav*> lockup;
	LinkedList<wav*> blaster;
	LinkedList<wav*> wrist;
	LinkedList<wav*> force;

	void fill(LinkedList<wav*>* list, uint8_t array[]) {
		for (uint8_t i = array[0]; i <= array[1]; i++) {
			list->add(new wav( { i, 0 }));
		}
	}

};

#endif /* SOUNDFONT_H_ */
