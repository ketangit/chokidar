#include "main.h"

void setup();
void loop();
void sendMessage(const char* topic, const char* message);
void recieveMessage(char* topic, char* payload, unsigned int length);    
void callbackDeviceStatus();
void callbackPinHigh();
void callbackPinStateChanged(bool state, uint8_t pin, uint8_t portNumber);
void signalHandler(int);

const char* deviceName = "CHOKIDAR";
char szBuffer[100];
char szState[5];
Timer *pTimer = 0;
MQTTClient *pMQTTClient = 0;
I2CPortDebounce *pDevice1 = 0;
I2CPortDebounce *pDevice2 = 0;

int BUZZER = 3;	// GPIO3, pin=22

int main(void) {
    setup();
    loop();
}

void setup() {
    // Setup the syslog logging
    setlogmask(LOG_UPTO(LOGLEVEL));
    openlog(deviceName, LOG_PID | LOG_CONS, LOG_USER);
    sprintf(szBuffer, "**** %s started ****", deviceName);
    syslog(LOG_INFO, szBuffer);

    // Register the signal handler for USR1-user defined signal 1
    if (signal(SIGUSR1, signalHandler) == SIG_ERR) {
        syslog(LOG_CRIT, "Not able to register the signal handler\n");
    }
    if (signal(SIGINT, signalHandler) == SIG_ERR) {
        syslog(LOG_CRIT, "Not able to register the signal handler\n");
    }

    // Create MQTT client for publish/subscribe messages
    pMQTTClient = new MQTTClient(deviceName, "", "", "127.0.0.1", 1883, recieveMessage);
    sprintf(szBuffer, "SENSOR/%s/COMMAND", deviceName);
    if(!pMQTTClient->subscribe(szBuffer)) {
        syslog(LOG_CRIT, "Error: subscribing MQTT messages\n");
    }
    sprintf(szBuffer, "SENSOR/%s/STATUS", deviceName);
    sendMessage(szBuffer, "STARTED");

    // Initialize wiringPi using wiringPi pins
    wiringPiSetup();

    for(int pin=0;pin<8;pin++) {
        pinMode(pin, OUTPUT);
        pullUpDnControl(pin, PUD_UP); // Enable pull-up resistor
        digitalWrite(pin, HIGH);
    }

    // Initialize I2C device input ports  
    pDevice1 = new I2CPortDebounce();
    pDevice1->init(MCP23017_DEVICE1, 1, 2, callbackPinStateChanged);
    pDevice2 = new I2CPortDebounce();
    pDevice2->init(MCP23017_DEVICE2, 3, 0, callbackPinStateChanged);  // port-B is disabled
    
    pTimer = new Timer();
    pTimer->every(PING_TIME, callbackDeviceStatus);
    pTimer->every(CHECK_PIN_TIME, callbackPinHigh);
    pTimer->oscillate(BUZZER, 350, HIGH, 3);        // 350 milli-seconds
}

void loop() {
    while (1) {
        //pMQTTClient->loop();
        pTimer->update();
        pDevice1->update();
        pDevice2->update();
    }
}

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
    sprintf(szBuffer, "SENSOR/%s/COMMAND", deviceName);
    if (strcmp(topic, szBuffer)==0) {
      if (message == "REPORT") {
        pDevice1->report();
        pDevice2->report(); 
      }
    }
    syslog(LOG_INFO, "Recevied: Topic=%s, Message=%s\n", topic, message);
}

void callbackDeviceStatus() {
    sprintf(szBuffer, "SENSOR/%s/STATUS", deviceName);
    sendMessage(szBuffer, "ACTIVE");
}

void callbackPinHigh() {
    if(pDevice1->isAnyPinHigh() || pDevice2->isAnyPinHigh()) {
        syslog(LOG_INFO, "device1 or device2 pins are ON\n");
	pTimer->oscillate(BUZZER, 350, HIGH, 2); 	// 350 milli-seconds
        sprintf(szBuffer, "SENSOR/%s/ALERT", deviceName);
        sendMessage(szBuffer, "YELLOW");
    }
}

void callbackPinStateChanged(bool state, uint8_t pin, uint8_t portNumber) {
    syslog(LOG_INFO, "I2C ports changed: port=%u, pin=%u, state=%s\n", portNumber, pin, (state ? "ON" : "OFF"));
    if(portNumber < 4 && state == true) {
	pTimer->pulseImmediate(BUZZER, 2000, LOW);	// 2000 milli-seconds
	sprintf(szBuffer, "SENSOR/%s/ALERT", deviceName);
        sendMessage(szBuffer, "AMBER");
    }
    sprintf(szBuffer, "SENSOR/%s/PORT/%u%u", deviceName, portNumber, pin);
    sprintf(szState, "%s", (state ? "Open" : "Close"));
    sendMessage(szBuffer, szState);
}

// Signal handler to handle when the user tries to kill this process. Try to close down gracefully
void signalHandler(int signo) {
    syslog(LOG_INFO, "Trying to end the process gracefully. Closing the MQTT connection. \n");
    sprintf(szBuffer, "SENSOR/%s/STATUS", deviceName);
    sendMessage(szBuffer, "DISCONNECT");
    pMQTTClient->disconnect();
    pMQTTClient = 0;
    pTimer = 0;
    pDevice1 = 0;
    pDevice2 = 0;

    sprintf(szBuffer, "**** %s has ended ****", deviceName);
    syslog(LOG_INFO, szBuffer);
    closelog();
    exit(1);
}

