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



class SoundFont {

#define SOUNDFONT_QUANTITY 1

public:

	SoundFont() {

		boot = LinkedList<uint8_t>();
		powerOn = LinkedList<uint8_t>();
		powerOff = LinkedList<uint8_t>();
		hum = LinkedList<uint8_t>();
		swing = LinkedList<uint8_t>();
		clash = LinkedList<uint8_t>();
		lockup = LinkedList<uint8_t>();
		blaster = LinkedList<uint8_t>();
		wrist = LinkedList<uint8_t>();
		force = LinkedList<uint8_t>();

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

			boot[0] = 20;
			boot[1] = 20;
			powerOn[0] = 1;
			powerOn[1] = 1;
			powerOff[0] = 2;
			powerOff[1] = 2;
			hum[0] = 3;
			hum[1] = 3;
			swing[0] = 4;
			swing[1] = 11;
			clash[0] = 12;
			clash[1] = 14;
			lockup[0] = 18;
			lockup[1] = 18;
			blaster[0] = 15;
			blaster[1] = 17;
			wrist[0] = 19;
			wrist[1] = 19;
			force[0] = 21;
			force[1] = 21;
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

	uint8_t getFolder() const {
		return folder;
	}

	const uint8_t getBlaster() {
		return this->blaster.get(random(0, this->blaster.size()));
	}

	const uint8_t getBoot() {
		return this->boot.get(random(0, this->boot.size()));
	}

	const uint8_t getClash() {
		return this->clash.get(random(0, this->clash.size()));
	}

	const uint8_t getHum() {
		return this->hum.get(random(0, this->hum.size()));
	}

	const uint8_t getLockup() {
		return this->lockup.get(random(0, this->lockup.size()));
	}

	const uint8_t getPowerOff() {
		return this->powerOff.get(random(0, this->powerOff.size()));
	}

	const uint8_t getPowerOn() {
		return this->powerOn.get(random(0, this->powerOn.size()));
	}

	const uint8_t getSwing() {
		return this->swing.get(random(0, this->swing.size()));
	}

	const uint8_t getForce() {
		return this->force.get(random(0, this->force.size()));
	}
	const uint8_t getWrist() {
		return this->wrist.get(random(0, this->wrist.size()));
	}

private:
	uint8_t folder;
	LinkedList<uint8_t> boot;
	LinkedList<uint8_t> powerOn;
	LinkedList<uint8_t> powerOff;
	LinkedList<uint8_t> hum;
	LinkedList<uint8_t> swing;
	LinkedList<uint8_t> clash;
	LinkedList<uint8_t> lockup;
	LinkedList<uint8_t> blaster;
	LinkedList<uint8_t> wrist;
	LinkedList<uint8_t> force;

	void fill(LinkedList<uint8_t>* list, uint8_t array[]) {
		for (uint8_t i = array[0]; i <= array[1]; i++) {
			list->add(i);
		}
	}

};

#endif /* SOUNDFONT_H_ */
