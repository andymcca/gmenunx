#ifndef HW_LF1000_H
#define HW_LF1000_H

#include <math.h>

class LF1000 : public Platform {
private:
	volatile uint16_t *memregs;
	int memdev = 0;
	int32_t tickBattery = 0;

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

        batteryStatus = getBatteryStatus(getBatteryLevel(), 0, 0);

		INFO("LF1000 Init Done!");
	}

	void hwDeinit() {
	}

	uint32_t hwCheck(unsigned int interval=0, void *param = NULL) {
               tickBattery++;
                if (tickBattery > 30) { // update battery level every 30 hwChecks
                        tickBattery = 0;
                        batteryStatus = getBatteryStatus(getBatteryLevel(), 0, 0);
                }

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
		int val = -1;
		if (FILE *f = fopen("/sys/devices/platform/lf1000-power/voltage", "r")) {
			fscanf(f, "%i", &val);
			fclose(f);
		}
		INFO("Battery level %d\n", val);
		return val;
	}


	uint8_t getBatteryStatus(int32_t val, int32_t min, int32_t max) {
		if ((val > 10000) || (val < 0)) return 6;
		/* Needs tuning. LF's voltage refs in /sys say:
         "full battery" 8000mv
		 "normal battery" 4600mv
         "low battery" 4200mv  
		 "critical battery" 2000mv
		*/
	    INFO("Battery status %d %d %d\n", val, min, max);
		if (val > 7000) return 5; // 100%
		if (val > 6000) return 4; // 80%
		if (val > 4600) return 3; // 55%
		if (val > 4200) return 2; // 30%
		if (val > 3000) return 1; // 15%
		return 0; // 0% :(
	}

	void setCPU(uint32_t mhz) {
		mhz = constrain(mhz, gmenu2x->confInt["cpuMenu"], gmenu2x->confInt["cpuMax"]);
		if (FILE *f = fopen("/sys/class/backlight/lf1000-pwm-bl/brightness", "w")) {
			fprintf(f, "%d", mhz * 1000000);
			INFO("Set CPU to %d mhz (%d hz)", mhz, mhz * 1000000);
			fclose(f);
		}

	}

	uint8_t getVolumeMode(uint8_t vol) {
		if (!vol) return VOLUME_MODE_MUTE;
		return VOLUME_MODE_NORMAL;
	}

	void setVolume(int val) {
		val = val * (255.0f / 100.0f);
		volumeMode = getVolumeMode(val);

		int hp = 0;
		char cmd[96];
                // For now, use near-100% on Speaker/Headphone specifics, etc
		// TODO: Set mute off/on when calling exec, so that audio isn't running during the menu
		sprintf(cmd, "amixer set Master %d; amixer set Master on; amixer set Headphone 250; amixer set Speaker 250", val);
		system(cmd);
	}

};

#endif
