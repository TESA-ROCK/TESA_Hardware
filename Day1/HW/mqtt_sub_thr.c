#include "mqtt_sub_app.h"
#include <string.h>
#include <MQTTClient.h>

// Private constants
const char base_topic[] = "tgr2024/team/%s";
const char MQTT_BROKER[] = "tcp://broker.emqx.io:1883";
const char MQTT_CLIENTID[] = "TGR2024_SUB_CLIENT";

void *mqtt_sub_thr_fcn(void *ptr) {
    char topic[100];
    int rc;
    MQTTClient mqtt_client;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;

    printf("Starting MQTT subscriber thread\n");

    // Initialize MQTT client
    MQTTClient_create(&mqtt_client, MQTT_BROKER, MQTT_CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;

    // Connect to the MQTT broker
    if ((rc = MQTTClient_connect(mqtt_client, &conn_opts)) != MQTTCLIENT_SUCCESS) {
        printf("Failed to connect to MQTT broker, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }
    sprintf(topic, base_topic, (char*)ptr);
    printf("MQTT TOPIC: %s\n", topic);

    // Subscribe to the topic
    if ((rc = MQTTClient_subscribe(mqtt_client, topic, 0)) != MQTTCLIENT_SUCCESS) {
        printf("Failed to subscribe to topic %s, return code %d\n", topic, rc);
        MQTTClient_destroy(&mqtt_client);
        exit(EXIT_FAILURE);
    }
    printf("Subscribed to topic: %s\n", topic);

    // Message delivery callback
    MQTTClient_message *message = NULL;
    int topic_len;
    char *received_payload;

    while (1) {
        char mem_value[100];  // Buffer for Mem??? value
        rc = MQTTClient_receive(mqtt_client, &topic, &topic_len, &message, 1000);

        if (rc == MQTTCLIENT_SUCCESS && message != NULL) {
            received_payload = (char *)message->payload;

            printf("Received message: %s\n", received_payload);

            // Parse the JSON payload
            cJSON *json = cJSON_Parse(received_payload);
            if (json != NULL) {
                cJSON *check_item = cJSON_GetObjectItem(json, "check");
                cJSON *mem_item = cJSON_GetObjectItem(json, "Mem???");
                if (cJSON_IsString(mem_item) && mem_item->valuestring != NULL) {
                    strncpy(mem_value, mem_item->valuestring, sizeof(mem_value) - 1);
                    mem_value[sizeof(mem_value) - 1] = '\0'; // Null-terminate the string

                    // Update the shared variable
                    pthread_mutex_lock(&data_cond_mutex);
                    shared_value = atoi(mem_value);  // Convert Mem??? to an integer (if needed)
                    pthread_mutex_unlock(&data_cond_mutex);

                    printf("Updated shared value: %d\n", shared_value);
                } else {
                    printf("Invalid Mem??? field in JSON\n");
                }
                cJSON_Delete(json);
            } else {
                printf("Invalid JSON format\n");
            }

            MQTTClient_freeMessage(&message);
            MQTTClient_free(topic);
        } else if (rc != MQTTCLIENT_SUCCESS) {
            printf("MQTT receive failed, return code %d\n", rc);
        }

        // Sleep for a short time before checking again
        usleep(100000);  // 100ms
    }

    // Cleanup
    MQTTClient_disconnect(mqtt_client, 1000);
    MQTTClient_destroy(&mqtt_client);
    return NULL;
}
