/*
 * Soundfont.h
 *
 * Created on: 	27 feb 2016
 * author: 		Sebastien CAPOU (neskweek@gmail.com)
 * Source : 	https://github.com/neskweek/LightSaberOS
 * Description:	Soundfont Config file for LightSaberOS
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-sa/4.0/.
 */
#include <LinkedList.h>

#if not defined SOUNDFONT_H_
#define SOUNDFONT_H_

class SoundFont {

#define SOUNDFONT_QUANTITY 2

public:

	SoundFont() {

		boot = LinkedList<uint16_t>();
		powerOn = LinkedList<uint16_t>();
		powerOff = LinkedList<uint16_t>();
		hum = LinkedList<uint16_t>();
		swing = LinkedList<uint16_t>();
		spin = LinkedList<uint16_t>();
		clash = LinkedList<uint16_t>();
		lockup = LinkedList<uint16_t>();
		blaster = LinkedList<uint16_t>();
		wrist = LinkedList<uint16_t>();
		force = LinkedList<uint16_t>();
		ID=0;
		powerOnTime =500;
		powerOffTime = 500;

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
		uint16_t spin[2];
		uint16_t clash[2];
		uint16_t lockup[2];
		uint16_t blaster[2];
		uint16_t wrist[2];
		uint16_t force[2];

		this->ID = id;

		switch (id) {
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
			// If you specify a ID number not defined here you will end up
			// on the first defined soundfont
		case 2:
			// soundFont directory 02 :
			this->powerOnTime = 400;
			this->powerOffTime = 400;
			boot[0] = 20;
			boot[1] = 20;
			powerOn[0] = 21;
			powerOn[1] = 21;
			powerOff[0] = 22;
			powerOff[1] = 22;
			hum[0] = 23;
			hum[1] = 23;
			swing[0] = 24;
			swing[1] = 31;
			spin[0] = 24;
			spin[1] = 24;
			clash[0] = 32;
			clash[1] = 34;
			lockup[0] = 38;
			lockup[1] = 38;
			blaster[0] = 35;
			blaster[1] = 37;
			wrist[0] = 39;
			wrist[1] = 39;
			force[0] = 0;
			force[1] = 0;
			break;
		case 3:
			this->powerOnTime = 400;
			this->powerOffTime = 400;
			boot[0] = 40;
			boot[1] = 40;
			powerOn[0] = 41;
			powerOn[1] = 41;
			powerOff[0] = 42;
			powerOff[1] = 42;
			hum[0] = 43;
			hum[1] = 43;
			swing[0] = 45;
			swing[1] = 56;
			clash[0] = 57;
			clash[1] = 68;
			lockup[0] = 44;
			lockup[1] = 44;
			blaster[0] = 69;
			blaster[1] = 69;
			wrist[0] = 71;
			wrist[1] = 71;
			force[0] = 70;
			force[1] = 70;
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
		fill(&this->spin, spin);
		fill(&this->clash, clash);
		fill(&this->lockup, lockup);
		fill(&this->blaster, blaster);
		fill(&this->wrist, wrist);
		fill(&this->force, force);
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

	const uint16_t getSpin() {
			return this->spin.get(random(0, this->spin.size()));
		}

	const uint16_t getForce() {
		return this->force.get(random(0, this->force.size()));
	}
	const uint16_t getWrist() {
		return this->wrist.get(random(0, this->wrist.size()));
	}

	uint16_t getPowerOffTime() const {
		return powerOffTime;
	}

	uint16_t getPowerOnTime() const {
		return powerOnTime;
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
	LinkedList<uint16_t> spin;
	LinkedList<uint16_t> clash;
	LinkedList<uint16_t> lockup;
	LinkedList<uint16_t> blaster;
	LinkedList<uint16_t> wrist;
	LinkedList<uint16_t> force;

	void fill(LinkedList<uint16_t>* list, uint16_t array[]) {
		for (uint16_t i = array[0]; i <= array[1]; i++) {
			list->add(i);
		}
	}
};

#endif /* SOUNDFONT_H_ */
