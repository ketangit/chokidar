#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <syslog.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <kompex/KompexSQLiteDatabase.h>
#include <kompex/KompexSQLiteStatement.h>

#include "mqtt_client.h"
#include "Timer.h"

//Logging level for the syslog
//Default is INFO-6. Other possible values - ERROR-3, INFO-6, DEBUG-7
#define LOGLEVEL			6
#define MAC_STRING_LENGTH	13

#define PING_TIME         	60000  // 1 minute

#define MCP23017_DEVICE1    0x20
#define MCP23017_DEVICE2    0x21
#define MCP23017_IODIRA     0x00
#define MCP23017_GPPUA      0x0C
#define MCP23017_GPIOA      0x12
#define MCP23017_IODIRB     0x01
#define MCP23017_GPPUB      0x0D
#define MCP23017_GPIOB      0x13

#endif // MAIN_H
