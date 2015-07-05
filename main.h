#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <syslog.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <kompex/KompexSQLiteDatabase.h>
#include <kompex/KompexSQLiteStatement.h>

#include "mqtt_client.h"
#include "Timer.h"
#include "I2CPortDebounce.h"

//Logging level for the syslog
//Default is INFO-6. Other possible values - ERROR-3, INFO-6, DEBUG-7
#define LOGLEVEL			6
#define MAC_STRING_LENGTH	13

#define PING_TIME         	60000	// 1 minute
#define CHECK_PIN_TIME		240000	// 4 minutes	

#define MCP23017_DEVICE1    0x20
#define MCP23017_DEVICE2    0x21

#endif // MAIN_H
