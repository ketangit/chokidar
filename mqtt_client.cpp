#include <string.h>
#include <mqtt_client.h>

// Original Source: https://github.com/OpenSensorsIO/raspberry-pi-mqtt

/*
 * Internal callback for processing incoming messages.
 * It calls more friendly callback supplied by user.
 */
static void callbackMessageReceived(void * context, const struct mosquitto_message * message) {
    MQTTClient * _this = (MQTTClient *)context;

    if (_this->onMessage != 0) {
        mosquitto_message * destination = new mosquitto_message();
        mosquitto_message_copy(destination, message);
        _this->onMessage(destination->topic, (char*)destination->payload, destination->payloadlen);
    }
}


/*
 * Internal callback for processing disconnect.
 * We reset authenticated flag to force restore server connection next time.
 */
static void callbackDisconnected(void * context) {
    MQTTClient * _this = (MQTTClient *)context;
    _this->resetConnectedState();
}

MQTTClient::MQTTClient(const char* userName, const char* deviceId, const char* devicePassword, const char* serverName, int serverPort, void (*onMessage)(char*,char*,unsigned int)) {
    mosquitto_lib_init();

    this->_data = mosquitto_new(deviceId, this);

    this->_userName = userName;
    this->_devicePassword = devicePassword;
    this->_serverName = serverName;
    this->_serverPort = serverPort;
    this->_authenticatedInServer = false;
    this->onMessage = onMessage;

    mosquitto_message_callback_set(this->_data, callbackMessageReceived);
    mosquitto_disconnect_callback_set(this->_data, callbackDisconnected);
}


/*
 * Free allocated resources.
 */
MQTTClient::~MQTTClient() {
    mosquitto_disconnect(this->_data);
    mosquitto_destroy(this->_data);
    mosquitto_lib_cleanup();
}


/*
 * Perform connection to server (if not already connected) and return TRUE if success.
 */
bool MQTTClient::connectIfNecessary() {
    if (this->_authenticatedInServer) {
        return true;
    }

    // Call it before mosquitto_connect to supply additional user credentials.
    mosquitto_username_pw_set(this->_data, this->_userName, this->_devicePassword);

    int result = mosquitto_connect(this->_data, this->_serverName, this->_serverPort, 60, false);
    this->_authenticatedInServer = result == MOSQ_ERR_SUCCESS;
    return this->_authenticatedInServer;
}


/*
 * Reset connection flag to "not connected".
 */
void MQTTClient::resetConnectedState() {
    this->_authenticatedInServer = false;
}


/*
 * Publish message to topic.
 */
bool MQTTClient::publish(const char* topic, const char* payload) {
    if (!this->connectIfNecessary()) {
        return false;
    }
    int result = mosquitto_publish(this->_data, 0, topic, strlen(payload), (const uint8_t*)payload, 0, false);
    return result == MOSQ_ERR_SUCCESS;
}


/*
 * Subscribe for topic.
 */
bool MQTTClient::subscribe(const char* topic) {
    if (!this->connectIfNecessary()) {
        return false;
    }
    int result = mosquitto_subscribe(this->_data, 0, topic, 0);
    return result == MOSQ_ERR_SUCCESS;
}

/*
 * The main network loop for the client. You must call this frequently in order
 * to keep communications between the client and broker working.
 */
bool MQTTClient::loop() {
    // We use -1 for default 1000ms waiting for network activity.
    int result = mosquitto_loop(this->_data, -1);
    return result == MOSQ_ERR_SUCCESS;
}


/*
 * Disconnect from server. It may be useful to get FALSE from
 * MQTTClient::loop() and break from main communication cycle.
 */
bool MQTTClient::disconnect() {
    int result = mosquitto_disconnect(this->_data);
    return result = MOSQ_ERR_SUCCESS;
}
