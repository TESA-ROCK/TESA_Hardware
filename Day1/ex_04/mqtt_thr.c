#include "iot_app.h"
#include <string.h>
#include <MQTTClient.h>
#include <stdio.h>  // Include for snprintf

// private constants
const char base_topic[] = "tgr2024/team/TGR2024_TESA_ROCK";
const char MQTT_BROKER[] = "tcp://broker.emqx.io:1883";
const char MQTT_CLIENTID[] = "TGR2024_TESA_ROCK";

void *mqtt_thr_fcn(void *ptr) {
    // setup 
    char topic[100];
    int rc;

    printf("Starting MQTT thread\n");
    MQTTClient mqtt_client;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    MQTTClient_deliveryToken token;
    MQTTClient_create(&mqtt_client, MQTT_BROKER, MQTT_CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    if ((rc = MQTTClient_connect(mqtt_client, &conn_opts)) != MQTTCLIENT_SUCCESS) {
        printf("Failed to connect, return code %d\n", rc);
        return NULL;
    }

    // Construct JSON message
    const char *value = "your_value_here";  // Replace with your actual value
    char json_message[100];
    snprintf(json_message, sizeof(json_message), "{\"check\":\"%s\"}", value);

    // Publish the message
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    pubmsg.payload = json_message;
    pubmsg.payloadlen = strlen(json_message);
    pubmsg.qos = 1;
    pubmsg.retained = 0;
    MQTTClient_publishMessage(mqtt_client, base_topic, &pubmsg, &token);
    printf("Waiting for up to 10 seconds for publication of %s\n"
           "on topic %s for client with ClientID: %s\n",
           json_message, base_topic, MQTT_CLIENTID);
    rc = MQTTClient_waitForCompletion(mqtt_client, token, 10000L);
    printf("Message with delivery token %d delivered\n", token);

    // Disconnect
    MQTTClient_disconnect(mqtt_client, 10000);
    MQTTClient_destroy(&mqtt_client);

    return NULL;
}