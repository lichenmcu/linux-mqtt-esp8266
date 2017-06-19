/*******************************************************************************
 * Copyright (c) 2012, 2013 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution. 
 *
 * The Eclipse Public License is available at 
 *   http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at 
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Ian Craggs - initial contribution
 *    Ian Craggs - change delimiter option from char to string
 *    Al Stockdill-Mander - Version using the embedded C client
 *******************************************************************************/

/*
 
 stdout subscriber
 
 compulsory parameters:
 
  topic to subscribe to
 
 defaulted parameters:
 
	--host localhost
	--port 1883
	--qos 2
	--delimiter \n
	--clientid stdout_subscriber
	
	--username none
	--password none

 for example:

    stdoutsub topic/of/interest --host iot.eclipse.org

*/

//#define NUTTX

#ifndef NUTTX
#include "config.h"
#else
#include "nuttx/config.h"
#endif

#include <stdio.h>
#include "MQTTClient.h"

#include <stdio.h>
#include <signal.h>
//#include <memory.h>

#include <sys/time.h>


volatile int toStop = 0;


void usage()
{
	printf("MQTT stdout subscriber\n");
	printf("Usage: stdoutsub topicname <options>, where options are:\n");
	printf("  --host <hostname> (default is localhost)\n");
	printf("  --port <port> (default is 1883)\n");
	printf("  --qos <qos> (default is 2)\n");
	printf("  --delimiter <delim> (default is \\n)\n");
	printf("  --clientid <clientid> (default is hostname+timestamp)\n");
	printf("  --username none\n");
	printf("  --password none\n");
	printf("  --showtopics <on or off> (default is on if the topic has a wildcard, else off)\n");
	exit(-1);
}

#ifndef NUTTX
void cfinish(int sig)
{
	signal(SIGINT, NULL);
	toStop = 1;
}
#endif


struct opts_struct
{
	char* clientid;
	int nodelimiter;
	char* delimiter;
	enum QoS qos;
	char* username;
	char* password;
	char* host;
	int port;
	int showtopics;
} opts =
{
//	(char*)"stdout-subscriber", 0, (char*)"\n", QOS2, NULL, NULL, (char*)"localhost", 1883, 0
	(char*)"clientid", 0, (char*)"\n", QOS0, NULL, NULL, \
        (char*)CONFIG_EXAMPLES_MQTT_SERV_IP, \
               CONFIG_EXAMPLES_MQTT_SERV_PORT, 1
};


void getopts(int argc, char** argv)
{
	int count = 2;
	
	while (count < argc)
	{
		if (strcmp(argv[count], "--qos") == 0)
		{
			if (++count < argc)
			{
				if (strcmp(argv[count], "0") == 0)
					opts.qos = QOS0;
				else if (strcmp(argv[count], "1") == 0)
					opts.qos = QOS1;
				else if (strcmp(argv[count], "2") == 0)
					opts.qos = QOS2;
				else
					usage();
			}
			else
				usage();
		}
		else if (strcmp(argv[count], "--host") == 0)
		{
			if (++count < argc)
				opts.host = argv[count];
			else
				usage();
		}
		else if (strcmp(argv[count], "--port") == 0)
		{
			if (++count < argc)
				opts.port = atoi(argv[count]);
			else
				usage();
		}
		else if (strcmp(argv[count], "--clientid") == 0)
		{
			if (++count < argc)
				opts.clientid = argv[count];
			else
				usage();
		}
		else if (strcmp(argv[count], "--username") == 0)
		{
			if (++count < argc)
				opts.username = argv[count];
			else
				usage();
		}
		else if (strcmp(argv[count], "--password") == 0)
		{
			if (++count < argc)
				opts.password = argv[count];
			else
				usage();
		}
		else if (strcmp(argv[count], "--delimiter") == 0)
		{
			if (++count < argc)
				opts.delimiter = argv[count];
			else
				opts.nodelimiter = 1;
		}
		else if (strcmp(argv[count], "--showtopics") == 0)
		{
			if (++count < argc)
			{
				if (strcmp(argv[count], "on") == 0)
					opts.showtopics = 1;
				else if (strcmp(argv[count], "off") == 0)
					opts.showtopics = 0;
				else
					usage();
			}
			else
				usage();
		}
		count++;
	}
	
}


void messageArrived(MessageData* md)
{
	MQTTMessage* message = md->message;

	if (opts.showtopics)
		printf("%.*s\t", md->topicName->lenstring.len, md->topicName->lenstring.data);
	if (opts.nodelimiter)
		printf("%.*s", (int)message->payloadlen, (char*)message->payload);
	else
		printf("%.*s%s", (int)message->payloadlen, (char*)message->payload, opts.delimiter);
	//fflush(stdout);
}

static int netsetup(void)
{
  int ret;
  int timeout;

  ret = lesp_initialize();
  if (ret != OK)
    {
      perror("ERROR: failed to init network\n");
      return ret;
    }

  ret = lesp_soft_reset();
  if (ret != OK)
    {
      perror("ERROR: failed to reset\n");
      return ret;
    }

  timeout = 5;
  ret = lesp_ap_connect(CONFIG_EXAMPLES_MQTT_SSID, \
                        CONFIG_EXAMPLES_MQTT_PASSWORD, \
                        timeout);
  if (ret != OK)
    {
      perror("ERROR: failed to connect wifi\n");
      return ret;
    }

  return ret;
}


#ifdef CONFIG_BUILD_KERNEL
int main(int argc, FAR char *argv[])
#else
int mqtt_main(int argc, char *argv[])
#endif
{
	int rc = 0;
	unsigned char buf[100];
	unsigned char readbuf[100];
	
	if (argc < 2)
		usage();
	
	char* topic = argv[1];

	if (strchr(topic, '#') || strchr(topic, '+'))
		opts.showtopics = 1;
	if (opts.showtopics)
		printf("topic is %s\n", topic);

	getopts(argc, argv);	

	Network n;
//	Client c;
	MQTTClient c;

#ifndef NUTTX
	signal(SIGINT, cfinish);
	signal(SIGTERM, cfinish);
#endif

#if 0
	NewNetwork(&n);
	ConnectNetwork(&n, opts.host, opts.port);
	MQTTClient(&c, &n, 1000, buf, 100, readbuf, 100);
#else

	rc = netsetup();
	if (rc != OK) {
		exit(1);
	}

	NetworkInit(&n);
	printf("Connecting to %s %d\n", opts.host, opts.port);
	rc = NetworkConnect(&n, opts.host, opts.port);
	if (rc == -1) {
		perror("ERROR: failed to connect mqtt server\n");
		exit(1);
	}
	MQTTClientInit(&c, &n, 3000, buf, 100, readbuf, 100);
#endif

	MQTTPacket_connectData data = MQTTPacket_connectData_initializer;       
	data.willFlag = 0;
	data.MQTTVersion = 4;
	data.clientID.cstring = opts.clientid;
	data.username.cstring = opts.username;
	data.password.cstring = opts.password;

	data.keepAliveInterval = 120;
	data.cleansession = 1;
	
	rc = MQTTConnect(&c, &data);
	if (rc != SUCCESS) {
		printf("MQTT Connect Failed: %d\n", rc);
		NetworkDisconnect(&n);
		exit(1);
	}

	printf("Subscribing to %s\n", topic);
	rc = MQTTSubscribe(&c, topic, opts.qos, messageArrived);
	printf("Subscribed %d\n", rc);

	int count = 0;
	while (!toStop)
	{
		MQTTMessage message;
		char payload[30];

		message.qos = 1;
		message.retained = 0;
		message.payload = payload;
		sprintf(payload, "message number %d", count++);
		message.payloadlen = strlen(payload);

		if ((rc = MQTTPublish(&c, topic, &message)) != 0) {
			printf("Return code from MQTT publish is %d\n", rc);
			break;
		}

		MQTTYield(&c, 3000);	
	}
	
	printf("Stopping\n");

	MQTTDisconnect(&c);

	NetworkDisconnect(&n);
//	n.disconnect(&n);

	return 0;
}


