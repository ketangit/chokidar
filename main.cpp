#include <stdio.h>
#include <string.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <signal.h>
#include <mqtt_client.h>

#define STATUS_TIME         60000  // 1 minute

#define MCP23017_DEVICE1    0x20
#define MCP23017_DEVICE2    0x21
#define MCP23017_IODIRA     0x00
#define MCP23017_GPPUA      0x0C
#define MCP23017_GPIOA      0x12
#define MCP23017_IODIRB     0x01
#define MCP23017_GPPUB      0x0D
#define MCP23017_GPIOB      0x13

int fd1 = 0, fd2 = 0;
int valuePortA1 = 0, valuePortB1 = 0, valuePortA2 = 0, valuePortB2 = 0;
int keepRunning = 1;
char szPinNumber[20];

void setup();
void loop();
void ctrlCHandler(int);
void getPinNumber(int);

MQTTClient *mqttClient = 0;
void sendMessage(const char*, const char*);
void recieveMessage(char*, char*, unsigned int);

int main(void) {
    printf("Starting up ...\n");
    setup();
    loop();
    printf("Terminating ...\n");
    sendMessage("SENSOR/CHOKIDAR/STATUS","DISCONNECT");
    mqttClient->disconnect();
    mqttClient = 0;
}

void setup() {
    signal(SIGINT, ctrlCHandler);

    // Create MQTT client for publish/subscribe messages
    mqttClient = new MQTTClient("", "CHOKIDAR", "", "127.0.0.1", 1883, recieveMessage);
    if(!mqttClient->subscribe("SENSOR/CHOKIDAR/COMMAND/#")) {
        fprintf(stderr, "Error: subscribing MQTT messages");
    }
    sendMessage("SENSOR/CHOKIDAR/STATUS","STARTED");

    // Initialize wiringPi using wiringPi pins
    wiringPiSetup();

    for(int pin=0;pin<8;pin++) {
        pinMode(pin, OUTPUT);
        pullUpDnControl(pin, PUD_UP); // Enable pull-up resistor
        digitalWrite(pin, HIGH);
    }

    fd1 = wiringPiI2CSetup(MCP23017_DEVICE1);
    if(fd1 < 0) {
        fprintf(stderr, "Error: unable to initialize MCP23017 device at address %d", MCP23017_DEVICE1);
    } else {
        wiringPiI2CWriteReg8(fd1, MCP23017_IODIRA, 0b11111111);  // all input
        wiringPiI2CWriteReg8(fd1, MCP23017_GPPUA,  0b11111111);  // all pull-up
        wiringPiI2CWriteReg8(fd1, MCP23017_IODIRB, 0b11111111);  // all input
        wiringPiI2CWriteReg8(fd1, MCP23017_GPPUB,  0b11111111);  // all pull-up
    }

    fd2 = wiringPiI2CSetup(MCP23017_DEVICE2);
    if(fd2 < 0) {
        fprintf(stderr, "Error: unable to initialize MCP23017 device at address %d", MCP23017_DEVICE2);
    } else {
        wiringPiI2CWriteReg8(fd2, MCP23017_IODIRA, 0b11111111);  // all input
        wiringPiI2CWriteReg8(fd2, MCP23017_GPPUA,  0b11111111);  // all pull-up
        wiringPiI2CWriteReg8(fd2, MCP23017_IODIRB, 0b11111111);  // all input
        wiringPiI2CWriteReg8(fd2, MCP23017_GPPUB,  0b11111111);  // all pull-up
    }
}

void loop() {
    char szBuffer[50];
    char szTmp[20];
    unsigned long last_time=0; 
    while (keepRunning == 1) {
        mqttClient->loop();
	if(millis() - last_time > STATUS_TIME) {
	    last_time = millis();
	    sendMessage("SENSOR/CHOKIDAR/STATUS","ACTIVE");
	}
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
            if( (valuePortA2 & 0b10000000) == 1 ||
                (valuePortA2 & 0b01000000) == 1 ||
                (valuePortA2 & 0b00100000) == 1 ||
                (valuePortA2 & 0b00010000) == 1 ) {
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
            printf(" tx: %s\n", szBuffer);
            sendMessage("SENSOR/CHOKIDAR/PORTS",szBuffer);
        }
    }
}

void ctrlCHandler(int dummy) {
    keepRunning = 0;
}

void getPinNumber(int portValue) {
    strcpy(szPinNumber, "");
    if( (portValue & 0b10000000) == 1) strcat(szPinNumber, "-7");
    if( (portValue & 0b01000000) == 1) strcat(szPinNumber, "-6");
    if( (portValue & 0b00100000) == 1) strcat(szPinNumber, "-5");
    if( (portValue & 0b00010000) == 1) strcat(szPinNumber, "-4");
    if( (portValue & 0b00001000) == 1) strcat(szPinNumber, "-3");
    if( (portValue & 0b00000100) == 1) strcat(szPinNumber, "-2");
    if( (portValue & 0b00000010) == 1) strcat(szPinNumber, "-1");
    if( (portValue & 0b00000001) == 1) strcat(szPinNumber, "-0");

    if(strlen(szPinNumber) > 1) {
    	szPinNumber[0] = '=';
    }
}

// send MQTT message
void sendMessage(const char* topic, const char* message) {
    if(!mqttClient->publish(topic,message)) {
        fprintf(stderr, "Error: publishing MQTT message %s", message);
    }
}

// recieve MQTT message
void recieveMessage(char* topic, char* payload, unsigned int length) {
    char* message = new char[length + 1];
    memset(message, 0, length + 1);
    memcpy(message, payload, length);

    printf(" rx: Topic=%s, Message=%s\n", topic, message);
}

