#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <syslog.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include "MQTTAsync.h"
#include "main.h"

char configFile[50] = "/etc/chokidar/app.conf";

char clientId[MAXBUF];
char publishTopic[MAXBUF] = "iot-2/evt/status/fmt/json";
char subscribeTopic[MAXBUF] = "iot-2/cmd/reboot/fmt/json";
MQTTAsync client;

void setup();
void loop();
void sig_handler(int);
char *getmac(char *);
int reconnect_delay(int);

int main(int argc, char **argv) {
    setup();
    loop();
}

void setup() {
    //setup the syslog logging
    setlogmask(LOG_UPTO(LOGLEVEL));
    openlog("chokidar", LOG_PID | LOG_CONS, LOG_USER);
    syslog(LOG_INFO, "**** Chokidar started ****");

    // register the signal handler for USR1-user defined signal 1
    if (signal(SIGUSR1, sig_handler) == SIG_ERR) {
        syslog(LOG_CRIT, "Not able to register the signal handler\n");
    }
    if (signal(SIGINT, sig_handler) == SIG_ERR) {
        syslog(LOG_CRIT, "Not able to register the signal handler\n");
    }

    char* clientId = getmac("eth0");
    printf("mac address = %s\n", clientId);
    //the timeout between the connection retry
    int connDelayTimeout = 1;	// default sleep for 1 sec
    int retryAttempt = 0;
    char *mqttUrl = "tcp://127.0.0.1:1883";
    int isRegistered = 0; // no ssl
    char *passwd = "";
    char *username = "";
    
    // initialize the MQTT connection
    init_mqtt_connection(&client, mqttUrl, isRegistered, clientId, username, passwd);
    // Wait till we get a successful connection to IoT MQTT server
    while (!MQTTAsync_isConnected(client)) {
		connDelayTimeout = 1; // add extra delay(3,60,600) only when reconnecting
		if (connected == -1) {
			connDelayTimeout = reconnect_delay(++retryAttempt);	//Try to reconnect after the retry delay
			syslog(LOG_ERR, "Failed connection attempt #%d. Will try to reconnect in %d seconds\n", retryAttempt, connDelayTimeout);
			connected = 0;
			init_mqtt_connection(&client, mqttUrl, isRegistered, clientId, username, passwd);
		}
		fflush(stdout);
		sleep(connDelayTimeout);
    }
    // resetting the counters
    connDelayTimeout = 1;
    retryAttempt = 0;
}

void loop() {
    while(1) {


    }
}


// Signal handler to handle when the user tries to kill this process. Try to close down gracefully
void sig_handler(int signo) {
    syslog(LOG_INFO, "Received the signal to terminate the chokidar process. \n");
    syslog(LOG_INFO, "Trying to end the process gracefully. Closing the MQTT connection. \n");
    int res = disconnect_mqtt_client(&client);
    syslog(LOG_INFO, "Disconnect finished with result code : %d\n", res);
    syslog(LOG_INFO, "Shutdown of the chokidar process is complete. \n");
    syslog(LOG_INFO, "**** Chokidar has ended ****");
    closelog();
    exit(1);
}

char *getmac(char *iface)
{
  char *ret = malloc(MAC_STRING_LENGTH);
  struct ifreq s;

  int fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);

  strcpy(s.ifr_name, iface);
  if (fd >= 0 && ret && 0 == ioctl(fd, SIOCGIFHWADDR, &s))
  {
    int i;
    for (i = 0; i < 6; ++i)
      snprintf(ret+i*2,MAC_STRING_LENGTH-i*2,"%02x",(unsigned char) s.ifr_addr.sa_data[i]);
  }
  else
  {
    perror("malloc/socket/ioctl failed");
    exit(1);
  }
  return(ret);
}


// Reconnect delay time depends on the number of failed attempts
int reconnect_delay(int i) {
	if (i < 10) {
		return 3; // first 10 attempts try within 3 seconds
	}
	if (i < 20)
		return 60; // next 10 attempts retry after every 1 minute

	return 600;	// after 20 attempts, retry every 10 minutes
}
