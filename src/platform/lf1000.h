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

	void setBacklight(int val) {
		if (FILE *f = fopen("/sys/class/graphics/fb0/blank", "w")) {
			fprintf(f, "%d", val <= 0);
			fclose(f);
		}

		if (FILE *f = fopen("/sys/class/backlight/lf1000-pwm-bl/brightness", "w")) {
			fprintf(f, "%d", 400 - (val * 4));
			INFO("Set Backlight %d %d", val, 400 - (val * 4));
			fclose(f);
		}

		return;
	}

	int16_t getBacklight() {
		int val = -1;
		if (FILE *f = fopen("/sys/class/backlight/lf1000-pwm-bl/brightness", "r")) {
			fscanf(f, "%i", &val);
			fclose(f);
		}
		INFO("Get backlight %d %d", val, (400-val)/4);
		return (400-val)/4 ;
	}

	int16_t getBatteryLevel() {
		return 6; // Stub for now
	}

	void setCPU(uint32_t mhz) {
	}
};

#endif
