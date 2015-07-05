#include "I2CPortDebounce.h"

I2CPortDebounce::I2CPortDebounce() {
    _valuePortA = 0;
    _valuePortB = 0;
    _pDebouncePortA = 0;
    _pDebouncePortB = 0; 
}

void I2CPortDebounce::init(int devId, char portNameA, char portNameB, void(*func)(bool, uint8_t, char *portName)) {
    _fileHandle = wiringPiI2CSetup(devId);
    if(_fileHandle < 0) {
        syslog(LOG_CRIT, "Error: unable to initialize MCP23017 I2C device at address %d", devId);
    } else {
        wiringPiI2CWriteReg8(_fileHandle, MCP23017_IODIRA, 0b11111111);  // all input
        wiringPiI2CWriteReg8(_fileHandle, MCP23017_GPPUA,  0b11111111);  // all pull-up
        wiringPiI2CWriteReg8(_fileHandle, MCP23017_IODIRB, 0b11111111);  // all input
        wiringPiI2CWriteReg8(_fileHandle, MCP23017_GPPUB,  0b11111111);  // all pull-up
        _pDebouncePortA = new PortDebounce(portNameA, func);
        _pDebouncePortB = new PortDebounce(portNameB, func);
    }	
}

void I2CPortDebounce::update(void) {
    int valA = wiringPiI2CReadReg8(_fileHandle, MCP23017_GPIOA);
    if (_valuePortA != valA) {
        _valuePortA = valA;
        _pDebouncePortA->update(_valuePortA);
    }
    
    int valB = wiringPiI2CReadReg8(_fileHandle, MCP23017_GPIOB);
    if (_valuePortB != valB) {
        _valuePortB = valB;
        _pDebouncePortB->update(_valuePortB);
    }
}
