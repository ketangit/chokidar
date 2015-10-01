#include "PortDebounce.h"

PortDebounce::PortDebounce(uint8_t portNumber, void(*func)(bool, uint8_t, uint8_t)) {
	this->configuredSwitchesNum = 0;
	this->bounceDelay = DEBOUNCE_DEFAULT_DELAY;
	this->portNumber = portNumber;
	for (uint8_t i = 0; i < DEBOUNCE_MAX_CAPACITY; i++) {
		switch_info_t *info = &this->switches[this->configuredSwitchesNum];
		info->pin = i;
		info->func = func;
		info->state = false;		// initial state of pin is LOW by default  digitalRead(pin);
		info->transientState = info->state;
		info->lastChangeTime = 0;
		info->settings = DEBOUNCE_SETTING_NORMAL;
		this->configuredSwitchesNum++;
	}	
}

uint8_t PortDebounce::getPortNumber() {
	return this->portNumber;
}

bool PortDebounce::isAnyPinHigh() {
	switch_info_t *info;
	for (uint8_t i = 0; i < DEBOUNCE_MAX_CAPACITY; i++) {
		info = &this->switches[i];
		if(info->state == true) {
			return true;
		}
	}
	return false;
}

void PortDebounce::update(uint8_t value) {
	uint8_t measuredState;
	uint8_t reportedState;
	switch_info_t *info;
	unsigned long currentTime = millis();
	unsigned long deltaTime;

	for (uint8_t i = 0; i < this->configuredSwitchesNum; i++) {
		info = &this->switches[i];
		measuredState = bitRead(value, info->pin);		// digitalRead(info->pin);
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
						info->func(reportedState, info->pin, this->portNumber);
					}
				}
				else {
					//falling edge
					if (!(info->settings & DEBOUNCE_SETTING_SKIP_FALLING_EDGE)) {
						info->func(reportedState, info->pin, this->portNumber);
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

void PortDebounce::report(uint8_t value) {
	uint8_t measuredState;
	switch_info_t *info;

	for (uint8_t i = 0; i < this->configuredSwitchesNum; i++) {
		info = &this->switches[i];
		measuredState = bitRead(value, info->pin);    // digitalRead(info->pin);
		info->func(measuredState, info->pin, this->portNumber);
	}
}
