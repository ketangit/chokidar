#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#include <mosquitto.h>
#include <syslog.h>

// Original Source: https://github.com/OpenSensorsIO/raspberry-pi-mqtt

class MQTTClient {
private:
    mosquitto* _data;
    const char* _userName;
    const char* _deviceId;
    const char* _devicePassword;
    const char* _serverName;
    int _serverPort;
    bool _authenticatedInServer;
    bool connectIfNecessary();

public:
    MQTTClient(const char * deviceId, const char * userName, const char * password, const char * serverName, int serverPort, void (*callback)(char*,char*,unsigned int));
    ~MQTTClient();
    bool publish(const char * topic, const char * payload);
    bool subscribe(const char * topic);
    bool loop();
    bool disconnect();

    // We don't recommend use these functions directly.
    // They are for internal purposes.
    void (*onMessage)(char*,char*,unsigned int);
    void (*onDisconnect)(void*);
    void resetConnectedState();
};

#endif //MQTT_CLIENT_H
