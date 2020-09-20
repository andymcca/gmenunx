#ifndef HW_LF1000_H
#define HW_LF1000_H

#include <math.h>

class LF1000 : public Platform {
private:
	volatile uint16_t *memregs;
	int memdev = 0;

	typedef struct {
		uint16_t batt;
		uint16_t remocon;
	} MMSP2ADC;

public:
	LF1000(GMenu2X *gmenu2x) : Platform(gmenu2x) {
				INFO("LF1000");
	};

	void hwInit() {
		setenv("SDL_NOMOUSE", "1", 1);
		w = 320;
		h = 240;

		INFO("LF1000 Init Done!");
	}

	void hwDeinit() {
	}

	void ledOn() {
	}

	void ledOff() {
	}

	int16_t getBatteryLevel() {
		return 6; // Stub for now
	}

	void setCPU(uint32_t mhz) {
	}
};

#endif