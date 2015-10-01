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
        if(portNumberA > 0) {
            wiringPiI2CWriteReg8(_fileHandle, MCP23017_IODIRA, 0b11111111);  // all input
            wiringPiI2CWriteReg8(_fileHandle, MCP23017_GPPUA,  0b11111111);  // all pull-up
            _pDebouncePortA = new PortDebounce(portNumberA, func);
            syslog(LOG_INFO, "Initialize I2C Port-A deviceId=%d, fileHandle=%d portNumber=%u\n",
                _deviceId, _fileHandle, _pDebouncePortA->getPortNumber());
        } else {
            syslog(LOG_INFO, "Disabled I2C Port-A deviceId=%d, fileHandle=%d\n", _deviceId, _fileHandle);
        }
        if(portNumberB > 0) {
            wiringPiI2CWriteReg8(_fileHandle, MCP23017_IODIRB, 0b11111111);  // all input
            wiringPiI2CWriteReg8(_fileHandle, MCP23017_GPPUB,  0b11111111);  // all pull-up
            _pDebouncePortB = new PortDebounce(portNumberB, func);
            syslog(LOG_INFO, "Initialize I2C Port-B deviceId=%d, fileHandle=%d portNumber=%u\n",
                _deviceId, _fileHandle, _pDebouncePortB->getPortNumber());
        } else {
            syslog(LOG_INFO, "Disabled I2C Port-B deviceId=%d, fileHandle=%d\n", _deviceId, _fileHandle);
        }
    }	
}

void I2CPortDebounce::update(void) {
    if(_pDebouncePortA > 0) {
        _valuePortA = wiringPiI2CReadReg8(_fileHandle, MCP23017_GPIOA);
        _pDebouncePortA->update(_valuePortA);
        //syslog(LOG_INFO, "Received I2C Port-A deviceId=%d, portNumber=%u, portValue=%u\n", _deviceId, _pDebouncePortA->getPortNumber(), _valuePortA);
    }
    if(_pDebouncePortB > 0) {
        _valuePortB = wiringPiI2CReadReg8(_fileHandle, MCP23017_GPIOB);
        _pDebouncePortB->update(_valuePortB);
        //syslog(LOG_INFO, "Received I2C Port-A deviceId=%d, portNumber=%u, portValue=%u\n", _deviceId, _pDebouncePortB->getPortNumber(), _valuePortB);
    }
}

void I2CPortDebounce::report(void) {
    if(_pDebouncePortA > 0) {
        _valuePortA = wiringPiI2CReadReg8(_fileHandle, MCP23017_GPIOA);
        _pDebouncePortA->report(_valuePortA);
        //syslog(LOG_INFO, "Received I2C Port-A deviceId=%d, portNumber=%u, portValue=%u\n", _deviceId, _pDebouncePortA->getPortNumber(), _valuePortA);
    }
    if(_pDebouncePortB > 0) {
        _valuePortB = wiringPiI2CReadReg8(_fileHandle, MCP23017_GPIOB);
        _pDebouncePortB->report(_valuePortB);
        //syslog(LOG_INFO, "Received I2C Port-A deviceId=%d, portNumber=%u, portValue=%u\n", _deviceId, _pDebouncePortB->getPortNumber(), _valuePortB);
    }
}

bool I2CPortDebounce::isAnyPinHigh() {
    bool status = false;
    if(_pDebouncePortA > 0) {
        if(_pDebouncePortA->isAnyPinHigh()) {
            status = true;
        } 
    }
    
    if(_pDebouncePortB > 0) {
        if(_pDebouncePortB->isAnyPinHigh()) {
            status = true;
        } 
    }
    return status;
}
