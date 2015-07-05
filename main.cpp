#include "main.h"

//int fd1 = 0, fd2 = 0;
//int valuePortA1 = 0, valuePortB1 = 0, valuePortA2 = 0, valuePortB2 = 0;
//int keepRunning = 1;
//char szPinNumber[20];

void setup();
void loop();
//void getPinNumber(int signo);
void sendMessage(const char* topic, const char* message);
void recieveMessage(char* topic, char* payload, unsigned int length);    
void pingMQTTMessage();
void callbackPort(bool state, uint8_t pin, char *portName);
void signalHandler(int);

Timer *pTimer = 0;
MQTTClient *pMQTTClient = 0;
I2CPortDebounce *pDevice1 = 0;
I2CPortDebounce *pDevice2 = 0;

int main(void) {
    setup();
    loop();
}

void setup() {
    // Setup the syslog logging
    setlogmask(LOG_UPTO(LOGLEVEL));
    openlog("chokidar", LOG_PID | LOG_CONS, LOG_USER);
    syslog(LOG_INFO, "**** Chokidar started ****");

    // Register the signal handler for USR1-user defined signal 1
    if (signal(SIGUSR1, signalHandler) == SIG_ERR) {
        syslog(LOG_CRIT, "Not able to register the signal handler\n");
    }
    if (signal(SIGINT, signalHandler) == SIG_ERR) {
        syslog(LOG_CRIT, "Not able to register the signal handler\n");
    }    

    // Create MQTT client for publish/subscribe messages
    pMQTTClient = new MQTTClient("chokidar", "", "", "127.0.0.1", 1883, recieveMessage);
    if(!pMQTTClient->subscribe("SENSOR/CHOKIDAR/COMMAND/#")) {
        syslog(LOG_CRIT, "Error: subscribing MQTT messages\n");
    }
    sendMessage("SENSOR/CHOKIDAR/STATUS", "STARTED");

    // Initialize wiringPi using wiringPi pins
    wiringPiSetup();

    for(int pin=0;pin<8;pin++) {
        pinMode(pin, OUTPUT);
        pullUpDnControl(pin, PUD_UP); // Enable pull-up resistor
        digitalWrite(pin, HIGH);
    }

    // Initialize I2C device input ports  
    pDevice1 = new I2CPortDebounce();
    pDevice1->init(MCP23017_DEVICE1, 'A', 'B', callbackPort);
    pDevice2 = new I2CPortDebounce();
    pDevice2->init(MCP23017_DEVICE2, 'C', 'D', callbackPort);
    
    /*
    fd1 = wiringPiI2CSetup(MCP23017_DEVICE1);
    if(fd1 < 0) {
        syslog(LOG_CRIT, "Error: unable to initialize MCP23017 device at address %d", MCP23017_DEVICE1);
    } else {
        wiringPiI2CWriteReg8(fd1, MCP23017_IODIRA, 0b11111111);  // all input
        wiringPiI2CWriteReg8(fd1, MCP23017_GPPUA,  0b11111111);  // all pull-up
        wiringPiI2CWriteReg8(fd1, MCP23017_IODIRB, 0b11111111);  // all input
        wiringPiI2CWriteReg8(fd1, MCP23017_GPPUB,  0b11111111);  // all pull-up
    }

    fd2 = wiringPiI2CSetup(MCP23017_DEVICE2);
    if(fd2 < 0) {
        syslog(LOG_CRIT, "Error: unable to initialize MCP23017 device at address %d", MCP23017_DEVICE2);
    } else {
        wiringPiI2CWriteReg8(fd2, MCP23017_IODIRA, 0b11111111);  // all input
        wiringPiI2CWriteReg8(fd2, MCP23017_GPPUA,  0b11111111);  // all pull-up
        wiringPiI2CWriteReg8(fd2, MCP23017_IODIRB, 0b11111111);  // all input
        wiringPiI2CWriteReg8(fd2, MCP23017_GPPUB,  0b11111111);  // all pull-up
    }
    */
    pTimer = new Timer();
    pTimer->every(PING_TIME, pingMQTTMessage);
}

void loop() {
    //char szBuffer[50];
    //char szTmp[20];
    while (1) {
        pMQTTClient->loop();
        pTimer->update();
        pDevice1->update();
        pDevice2->update();
/*
        int val = 0;
        strcpy(szBuffer, "");
        val = wiringPiI2CReadReg8(fd1, MCP23017_GPIOA);
        if (valuePortA1 != val) {
            valuePortA1 = val;
            getPinNumber(valuePortA1);
            sprintf(szTmp, " C1=%d%s", valuePortA1, szPinNumber);
            strcat(szBuffer, szTmp);
        }
        delay(50);
        val = wiringPiI2CReadReg8(fd1, MCP23017_GPIOB);
        if (valuePortB1 != val) {
            valuePortB1 = val;
            getPinNumber(valuePortB1);
            sprintf(szTmp, " C2=%d%s", valuePortB1, szPinNumber);
            strcat(szBuffer, szTmp);
        }
        delay(50);
        val = wiringPiI2CReadReg8(fd2, MCP23017_GPIOA);
        if(valuePortA2 != val) {
            valuePortA2 = val;
            getPinNumber(valuePortA2);
            sprintf(szTmp, " C3=%d%s", valuePortA2, szPinNumber);
            strcat(szBuffer, szTmp);
            if( (valuePortA2 ^ 0b10000000) == 0 ||
                (valuePortA2 ^ 0b01000000) == 0 ||
                (valuePortA2 ^ 0b00100000) == 0 ||
                (valuePortA2 ^ 0b00010000) == 0 ) {
                    sendMessage("SENSOR/ALERT","AMBER");
                }
        }
        delay(50);
        val = wiringPiI2CReadReg8(fd2, MCP23017_GPIOB);
        if(valuePortB2 != val) {
            valuePortB2 = val;
            getPinNumber(valuePortB2);
            sprintf(szTmp, " C4=%d%s", valuePortB2, szPinNumber);
            strcat(szBuffer, szTmp);
        }
        delay(50);

        if(strlen(szBuffer) != 0) {
            sendMessage("SENSOR/CHOKIDAR/PORTS",szBuffer);
        }
*/
    }
}

/*
void getPinNumber(int portValue) {
    strcpy(szPinNumber, "");
    if( (portValue ^ 0b10000000) == 0) strcat(szPinNumber, "-7");
    if( (portValue ^ 0b01000000) == 0) strcat(szPinNumber, "-6");
    if( (portValue ^ 0b00100000) == 0) strcat(szPinNumber, "-5");
    if( (portValue ^ 0b00010000) == 0) strcat(szPinNumber, "-4");
    if( (portValue ^ 0b00001000) == 0) strcat(szPinNumber, "-3");
    if( (portValue ^ 0b00000100) == 0) strcat(szPinNumber, "-2");
    if( (portValue ^ 0b00000010) == 0) strcat(szPinNumber, "-1");
    if( (portValue ^ 0b00000001) == 0) strcat(szPinNumber, "-0");
    if(strlen(szPinNumber) > 1) {
    	szPinNumber[0] = '=';
    }
}
*/

// send MQTT message
void sendMessage(const char* topic, const char* message) {
    if(!pMQTTClient->publish(topic,message)) {
        syslog(LOG_CRIT, "Error: publishing MQTT message %s\n", message);
    }
}

// recieve MQTT message
void recieveMessage(char* topic, char* payload, unsigned int length) {
    char* message = new char[length + 1];
    memset(message, 0, length + 1);
    memcpy(message, payload, length);
    syslog(LOG_INFO, "Recevied: Topic=%s, Message=%s\n", topic, message);
}

void pingMQTTMessage() {
    sendMessage("SENSOR/CHOKIDAR/STATUS", "ACTIVE");
}

void callbackPort(bool state, uint8_t pin, char *portName) {
    syslog(LOG_INFO, "I2C ports changed: portName=%s, pinNumber=%u, state=%s\n", portName, pin, (state ? "H" : "L"));
    sendMessage("SENSOR/ALERT", "AMBER");
    /*
	if (strncmp(portName, "A", 1) == 0) {
		//printf("port=%s pin=%u state=%s\n", portName, pin, (state ? "H" : "L"));
		char payload[40];
		sprintf_s(payload, "port=%s pin=%u state=%s", portName, pin, (state ? "H" : "L"));
		//client.publish("TEST/MSG", payload);
	}*/
}

// Signal handler to handle when the user tries to kill this process. Try to close down gracefully
void signalHandler(int signo) {
    syslog(LOG_INFO, "Received the signal to terminate the chokidar process. \n");
    syslog(LOG_INFO, "Trying to end the process gracefully. Closing the MQTT connection. \n");
    
    sendMessage("SENSOR/CHOKIDAR/STATUS", "DISCONNECT");
    pMQTTClient->disconnect();
    pMQTTClient = 0;
    pTimer = 0;
    pDevice1 = 0;
    pDevice2 = 0;

    syslog(LOG_INFO, "Shutdown of the chokidar process is complete. \n");
    syslog(LOG_INFO, "**** Chokidar has ended ****");
    closelog();
    exit(1);
}
