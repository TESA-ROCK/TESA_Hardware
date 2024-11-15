#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <MQTTClient.h>
#include <json-c/json.h>

#define ADDRESS     "tcp://100.94.191.63:1883"
#define CLIENTID    "ExampleClientSub"
#define TOPIC       "test/topic"
#define QOS         1
#define TIMEOUT     10000L
#define DB_NAME     "1_2_AudioStream.db"

volatile MQTTClient_deliveryToken deliveredtoken;

void delivered(void *context, MQTTClient_deliveryToken dt) {
    printf("Message with token value %d delivery confirmed\n", dt);
    deliveredtoken = dt;
}

int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message) {
    sqlite3 *db;
    char *err_msg = 0;
    int rc;
    struct json_object *parsed_json;
    struct json_object *timestamp;
    struct json_object *datasize;

    char *payload = message->payload;
    parsed_json = json_tokener_parse(payload);
    json_object_object_get_ex(parsed_json, "timestamp_1_2", &timestamp);
    json_object_object_get_ex(parsed_json, "datasize", &datasize);

    rc = sqlite3_open(DB_NAME, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        return 1;
    }

    char sql[256];
    snprintf(sql, sizeof(sql), "INSERT INTO AudioStream (timestamp, datasize) VALUES ('%s', %s);",
             json_object_get_string(timestamp), json_object_get_string(datasize));

    rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
    }

    sqlite3_close(db);

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

    MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;

    MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd, delivered);

    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS) {
        printf("Failed to connect, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }

    MQTTClient_subscribe(client, TOPIC, QOS);

    printf("Press Enter to exit...\n");
    getchar();

    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);
    return rc;
}