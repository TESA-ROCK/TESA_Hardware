#include "MQTTClient.h"
#include <string.h>
#include <stdio.h>

#define ADDRESS     "tcp://100.94.191.63:1883"  // Replace with your broker address
#define CLIENTID    "ExampleClientSub"
#define TOPIC       "test/topic"                    // Replace with your topic
#define QOS         1
#define TIMEOUT     10000L
#define USERNAME    "user1"                 // Replace with your username
#define PASSWORD    "admin1234"                 // Replace with your password

volatile MQTTClient_deliveryToken deliveredtoken;

void delivered(void *context, MQTTClient_deliveryToken dt) {
    printf("Message with token value %d delivery confirmed\n", dt);
    deliveredtoken = dt;
}

int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message) {
    printf("Message arrived\n");
    printf("     topic: %s\n", topicName);
    printf("   message: %.*s\n", message->payloadlen, (char*)message->payload);
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}

void connlost(void *context, char *cause) {
    printf("\nConnection lost\n");
    printf("     cause: %s\n", cause);
}

int main(int argc, char* argv[]) {
    MQTTClient client;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    int rc;

    // Create the MQTT client
    MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);

    // Set callback functions
    MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd, delivered);

    // Set connection options
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    conn_opts.username = USERNAME;
    conn_opts.password = PASSWORD;

    // Connect to the broker
    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS) {
        printf("Failed to connect, return code %d\n", rc);
        return -1;
    }

    // Subscribe to the topic
    printf("Subscribing to topic %s for client %s using QoS %d\n\n", TOPIC, CLIENTID, QOS);
    MQTTClient_subscribe(client, TOPIC, QOS);


    // Keep the program running indefinitely
    while(1) {
        // Sleep for a short period to avoid busy-waiting
        sleep(1);
    }
    
    // // Wait for messages
    // printf("Press Q<Enter> to quit\n\n");
    // do {
    //     char c = getchar();
    //     if (c == 'Q' || c == 'q') break;
    // } while(1);

    // // Disconnect
    // MQTTClient_disconnect(client, 10000);
    // MQTTClient_destroy(&client);
    return rc;
}