#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "MQTTAsync.h"
#include <syslog.h>

#define QOS 0
#define TRUSTSTORE "/opt/iot/IoTFoundation.pem"

/* this maintains the status of connection
 *  0 - Not connected
 *  1 - Connected
 * -1 - Connection Failed, retry
 */
int connected = 0;
volatile MQTTAsync_token deliveredtoken;

// This function is called when the message is successfully published
void onSend(void* context, MQTTAsync_successData* response) {
	syslog(LOG_DEBUG, "Event with token value %d delivery confirmed\n", response->token);
}

// This function is called when the subscription succeeds
void onSubscription(void* context, MQTTAsync_successData* response) {
	syslog(LOG_INFO, "Subscription succeeded\n");
}

// Called when the connection is successful. Update the connected variable
void onConnectSuccess(void* context, MQTTAsync_successData* response) {
	syslog(LOG_INFO, "Connection was successful\n");
	// The connection is successful. update it to 1
	connected = 1;
}

// After sending the message disconnect from IoT/mqtt server
int disconnect_mqtt_client(MQTTAsync* client) {
	MQTTAsync_disconnectOptions opts = MQTTAsync_disconnectOptions_initializer;
	int rc = MQTTASYNC_SUCCESS;

	opts.context = client;

	if ((rc = MQTTAsync_disconnect(client, &opts)) != MQTTASYNC_SUCCESS) {
		syslog(LOG_ERR, "Failed to start sendMessage, return code %d\n", rc);
	}
	MQTTAsync_destroy(client);
	return rc;
}

// On failure of connection to server, print the response code 
void onConnectFailure(void* context, MQTTAsync_failureData* response) {
	syslog(LOG_ERR, "Connect failed ");
	if (response) {
		syslog(LOG_ERR, "with response code : %d and with message : %s", response->code, response->message);
	}
	connected = -1; // connection has failed
}

// Function to process the subscribed messages
int subscribeMessage(void *context, char *topicName, int topicLen, MQTTAsync_message *message) {
	int i;
	char* payloadptr;
	char* command;
	int time_delay = 0;

	payloadptr = message->payload;

	MQTTAsync_freeMessage(&message);
	MQTTAsync_free(topicName);
	return 1;
}

// Try to reconnect if the connection is lost
void connlost(void *context, char *cause) {
	MQTTAsync client = (MQTTAsync)context;
	int rc;
	syslog(LOG_ERR, "Connection lost\n");
	syslog(LOG_ERR, " cause: %s\n", cause);

	syslog(LOG_INFO, "Retrying the connection\n");
		MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;

		conn_opts.keepAliveInterval = 20;
		conn_opts.cleansession = 1;
		conn_opts.onSuccess = onConnectSuccess;
		conn_opts.onFailure = onConnectFailure;
		conn_opts.context = &client;
		syslog(LOG_INFO, "Retrying the connection -1 \n");
		if ((rc = MQTTAsync_connect(client, &conn_opts)) != MQTTASYNC_SUCCESS) {
			syslog(LOG_ERR, "Failed to start connect from connlost, return code %d\n", rc);
		}
}

// Function is used to initialize the MQTT connection handle "client"
int init_mqtt_connection(MQTTAsync* client, char *address, int isRegistered, char* client_id, char* username, char* passwd) {

	MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;

	MQTTAsync_SSLOptions sslopts = MQTTAsync_SSLOptions_initializer;

	int rc = MQTTASYNC_SUCCESS;
	MQTTAsync_create(client, address, client_id, MQTTCLIENT_PERSISTENCE_NONE, NULL);

	MQTTAsync_setCallbacks(*client, NULL, NULL, subscribeMessage, NULL);
	syslog(LOG_INFO, "Connecting to %s with client Id: %s \n", address, client_id);

	conn_opts.keepAliveInterval = 20;
	conn_opts.cleansession = 1;
	conn_opts.onSuccess = onConnectSuccess;
	conn_opts.onFailure = onConnectFailure;
	conn_opts.context = client;
	//only when in registered mode, set the username/passwd and enable TLS
	if (isRegistered) {
		//currently only supported mech is token based. Need to change this in future.
		conn_opts.username = username;
		conn_opts.password = passwd;
		sslopts.trustStore = TRUSTSTORE;
		sslopts.enableServerCertAuth = 1;

		conn_opts.ssl = &sslopts;
	}

	if ((rc = MQTTAsync_connect(*client, &conn_opts)) != MQTTASYNC_SUCCESS) {
		syslog(LOG_ERR, "Failed to start connect, return code %d\n", rc);
	}
	return rc;
}

int reconnect(MQTTAsync* client, int isRegistered, char* username, char* passwd) {
	syslog(LOG_INFO, "Retrying the connection\n");
	MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
	MQTTAsync_SSLOptions sslopts = MQTTAsync_SSLOptions_initializer;

	conn_opts.keepAliveInterval = 20;
	conn_opts.cleansession = 1;
	conn_opts.onSuccess = onConnectSuccess;
	conn_opts.onFailure = onConnectFailure;
	conn_opts.context = client;
	int rc;

	//only when in registered mode, set the username/passwd and enable TLS
	if (isRegistered) {
			//currently only supported mech is token based. Need to change this in future.
			syslog(LOG_INFO, "with SSL properties\n");
			conn_opts.username = username;
			conn_opts.password = passwd;
			sslopts.trustStore = TRUSTSTORE;
			sslopts.enableServerCertAuth = 0;

			conn_opts.ssl = &sslopts;
	}

	if ((rc = MQTTAsync_connect(*client, &conn_opts)) != MQTTASYNC_SUCCESS) {
		syslog(LOG_ERR, "Failed to start connect, return code %d\n", rc);
	}
	return rc;
}

int subscribe(MQTTAsync* client, char *topic) {
	MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
	int rc = MQTTASYNC_SUCCESS;

	opts.onSuccess = onSubscription;
	opts.context = *client;

	if ((rc = MQTTAsync_subscribe(*client, topic, QOS, &opts)) != MQTTASYNC_SUCCESS) {
		syslog(LOG_ERR, "Failed to subscribe, return code %d\n", rc);
		return rc;
	}
	syslog(LOG_DEBUG, "Waiting for subscription on topic %s\n", topic);
	return rc;
}

// Function is used to publish events to the IoT MQTT reciver. This reuses the "client"  
int publishMQTTMessage(MQTTAsync* client, char *topic, char *payload) {
	MQTTAsync_message pubmsg = MQTTAsync_message_initializer;
	MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
	int rc = MQTTASYNC_SUCCESS;

	opts.onSuccess = onSend;
	opts.context = *client;

	pubmsg.payload = payload;
	pubmsg.payloadlen = strlen(payload);
	pubmsg.qos = QOS;
	pubmsg.retained = 0;
	deliveredtoken = 0;

	if ((rc = MQTTAsync_sendMessage(*client, topic, &pubmsg, &opts)) != MQTTASYNC_SUCCESS) {
		syslog(LOG_ERR, "Failed to start sendMessage, return code %d\n", rc);
		return rc;
	}

	syslog(LOG_DEBUG, "Waiting for publication of %s on topic %s\n", payload, topic);
	return rc;
}
