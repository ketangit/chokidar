#include "I2CPortDebounce.h"
#include <stdio.h>

I2CPortDebounce::I2CPortDebounce() {
    _valuePortA = 0;
    _valuePortB = 0;
    _pDebouncePortA = 0;
    _pDebouncePortB = 0; 
}

void I2CPortDebounce::init(int devId, uint8_t portNumberA, uint8_t portNumberB, void(*func)(bool, uint8_t, uint8_t)) {
    _deviceId = devId;
    _fileHandle = wiringPiI2CSetup(_deviceId);
    if(_fileHandle < 0) {
        syslog(LOG_CRIT, "Error: unable to initialize MCP23017 I2C device at address %d", _deviceId);
    } else {
        wiringPiI2CWriteReg8(_fileHandle, MCP23017_IODIRA, 0b11111111);  // all input
        wiringPiI2CWriteReg8(_fileHandle, MCP23017_GPPUA,  0b11111111);  // all pull-up
        wiringPiI2CWriteReg8(_fileHandle, MCP23017_IODIRB, 0b11111111);  // all input
        wiringPiI2CWriteReg8(_fileHandle, MCP23017_GPPUB,  0b11111111);  // all pull-up
        _pDebouncePortA = new PortDebounce(portNumberA, func);
        _pDebouncePortB = new PortDebounce(portNumberB, func);
        syslog(LOG_INFO, "Initialize I2C deviceId=%d, fileHandle=%d portNumbers=[%u,%u]\n", 
            _deviceId, _fileHandle, _pDebouncePortA->getPortNumber(), _pDebouncePortB->getPortNumber());
    }	
}

void I2CPortDebounce::update(void) {
    _valuePortA = wiringPiI2CReadReg8(_fileHandle, MCP23017_GPIOA);
    _pDebouncePortA->update(_valuePortA);
    //syslog(LOG_INFO, "Received I2C deviceId=%d, portNumber=%u, portValue=%u\n", _deviceId, _pDebouncePortA->getPortNumber(), _valuePortA);
    
    _valuePortB = wiringPiI2CReadReg8(_fileHandle, MCP23017_GPIOB);
    _pDebouncePortB->update(_valuePortB);
    //syslog(LOG_INFO, "Received I2C deviceId=%d, portNumber=%u, portValue=%u\n", _deviceId, _pDebouncePortB->getPortNumber(), _valuePortB);
}
