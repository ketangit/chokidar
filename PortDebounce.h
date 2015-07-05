#ifndef PORT_DEBOUNCE_H
#define PORT_DEBOUNCE_H

#include <inttypes.h>
#include <wiringPi.h>

// Original Source: // https://github.com/arnebech/Debounce
// Modfied to work with Raspberry PI

// Number of inputs we can monitor per port
#define DEBOUNCE_MAX_CAPACITY 8

// Default bounce delay in ms
#define DEBOUNCE_DEFAULT_DELAY 100

#define DEBOUNCE_SETTING_NORMAL 0x00
#define DEBOUNCE_SETTING_SKIP_RISING_EDGE 0x01
#define DEBOUNCE_SETTING_SKIP_FALLING_EDGE 0x02
#define DEBOUNCE_SETTINGS_INVERT 0x04
#define DEBOUNCE_SETTINGS_FAST_CALLBACK 0x08

#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
//#define bitSet(value, bit) ((value) |= (1UL << (bit)))
//#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
//#define bitWrite(value, bit, bitvalue) (bitvalue ? bitSet(value, bit) : bitClear(value, bit))

class PortDebounce {
    
public:
    PortDebounce(uint8_t portNumber, void(*func)(bool, uint8_t, uint8_t));
    void update(uint8_t value);
    uint8_t getPortNumber();
private:
    typedef struct config_struct {
        uint8_t pin;
        void(*func)(bool, uint8_t, uint8_t);
        uint8_t state;
        uint8_t transientState;
        unsigned long lastChangeTime;
        uint8_t settings;
        
    } switch_info_t;
    switch_info_t switches[DEBOUNCE_MAX_CAPACITY];
    uint8_t configuredSwitchesNum;
    uint16_t bounceDelay;
    uint8_t portNumber;
};

#endif // PORT_DEBOUNCE_H
