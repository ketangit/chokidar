#ifndef I2C_PORT_DEBOUNCE_H
#define I2C_PORT_DEBOUNCE_H

#include <inttypes.h>
#include <wiringPi.h>
#include "PortDebounce.h"

#define MCP23017_IODIRA     0x00
#define MCP23017_GPPUA      0x0C
#define MCP23017_GPIOA      0x12
#define MCP23017_IODIRB     0x01
#define MCP23017_GPPUB      0x0D
#define MCP23017_GPIOB      0x13

class I2CPortDebounce {
public:
  I2CPortDebounce();
  void init(int devId, char portNameA, char portNameB, void(*func)(bool, uint8_t, char *portName));
  void update(void);
  
  private:
    int _fileHandle;
    int _valuePortA;
    int _valuePortB;
    PortDebounce *_pDebouncePortA;
    PortDebounce *_pDebouncePortB; 
};

#endif // I2C_PORT_DEBOUNCE_H
