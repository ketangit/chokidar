#include "Debounce.h"

Debounce::Debounce(char portName, void(*func)(bool, uint8_t, char *portName)) {
	this->configuredSwitchesNum = 0;
	this->bounceDelay = DEBOUNCE_DEFAULT_DELAY;
	this->portName = portName;
	for (uint8_t pin = 0; pin < DEBOUNCE_MAX_CAPACITY; pin++) {
		switch_info_t *info = &this->switches[this->configuredSwitchesNum];
		info->pin = pin;
		info->func = func;
		info->state = false;		// initial state of pin is LOW by default  digitalRead(pin);
		info->transientState = info->state;
		info->lastChangeTime = 0;
		info->settings = settings;
		this->configuredSwitchesNum++;
	}	
}

void Debounce::update(uint8_t value) {
	uint8_t measuredState;
	uint8_t reportedState;
	switch_info_t *info;
	unsigned long currentTime = millis();		//FIXME
	unsigned long deltaTime;

	for (uint8_t i = 0; i < this->configuredSwitchesNum; i++) {
		info = &this->switches[i];
		measuredState = bitRead(value, info->pin);		// digitalRead(info->pin);	//FIXME
		if (!(info->settings & DEBOUNCE_SETTINGS_FAST_CALLBACK) && measuredState != info->transientState) {
			info->transientState = measuredState;
			info->lastChangeTime = currentTime;
		}
		if (measuredState != info->state) {
			deltaTime = currentTime - info->lastChangeTime;
			if (deltaTime > this->bounceDelay) {
				info->state = measuredState;
				if (info->settings & DEBOUNCE_SETTINGS_INVERT) {
					reportedState = !measuredState;
				}
				else {
					reportedState = measuredState;
				}
				if (reportedState) {
					//rising edge
					if (!(info->settings & DEBOUNCE_SETTING_SKIP_RISING_EDGE)) {
						info->func(reportedState, info->pin, &this->portName);
					}
				}
				else {
					//falling edge
					if (!(info->settings & DEBOUNCE_SETTING_SKIP_FALLING_EDGE)) {
						info->func(reportedState, info->pin, &this->portName);
					}
				}
			}
		}
		if (measuredState != info->transientState) {
			info->transientState = measuredState;
			info->lastChangeTime = currentTime;
		}
	}
}