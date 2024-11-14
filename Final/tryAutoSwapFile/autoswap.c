#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <MQTTClient.h>
#include <openssl/bio.h>
#include <openssl/evp.h>

#define ADDRESS     "tcp://broker.hivemq.com:1883" // Replace with your broker address
#define CLIENTID    "RaspberryPiReceiver"
#define TOPIC       "raspberrypi/file"
#define QOS         1
#define TIMEOUT     10000L

// Base64 decoding function using OpenSSL
unsigned char base64_decode(const charinput, int length, int out_length) {
    BIOb64 = BIO_new(BIO_f_base64());
    BIO bio = BIO_new_mem_buf(input, length);
    bio = BIO_push(b64, bio);

    unsigned charbuffer = (unsigned char *)malloc(length);
    out_length = BIO_read(bio, buffer, length);

    BIO_free_all(bio);
    return buffer;
}

// Callback when a message is received
void messageArrived(MessageDatadata) {
    printf("Message received on topic: %s\n", data->topicName->cstring);

    // Decode the base64 data
    int decoded_length;
    unsigned char decoded_data = base64_decode((char)data->message->payload, data->message->payloadlen, &decoded_length);

    if (decoded_data) {
        // Save decoded data to a file
        FILE file = fopen("/path/to/save/received_file.txt", "wb");
        if (file) {
            fwrite(decoded_data, 1, decoded_length, file);
            fclose(file);
            printf("File saved successfully.\n");
        } else {
            printf("Failed to open file for writing.\n");
        }

        free(decoded_data);
    } else {
        printf("Failed to decode base64 data.\n");
    }
}

int main(int argc, char argv[]) {
    MQTTClient client;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    int rc;

    // Initialize MQTT client
    MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;

    // Connect to the broker
    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS) {
        printf("Failed to connect, return code %d\n", rc);
        return -1;
    }

    // Subscribe to the topic
    MQTTClient_subscribe(client, TOPIC, QOS);

    // Assign message callback
    MQTTClient_setCallbacks(client, NULL, NULL, messageArrived, NULL);

    printf("Listening for messages on topic: %s\n", TOPIC);
    while (1) {
        // Infinite loop to keep the program running
    }

    // Disconnect and cleanup (unreachable in this example)
    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);

    return 0;
}