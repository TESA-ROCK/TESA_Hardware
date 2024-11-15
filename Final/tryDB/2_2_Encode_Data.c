#include "MQTTClient.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <cjson/cJSON.h>
#include <openssl/bio.h>
#include <openssl/evp.h>

#define ADDRESS     "tcp://100.94.191.63:1883"  // Replace with your broker address
#define CLIENTID    "ExampleClientSub"
#define TOPIC       "test/topic"                // Replace with your topic
#define QOS         0
#define TIMEOUT     10000L
#define USERNAME    "user1"                     // Replace with your username
#define PASSWORD    "admin1234"                 // Replace with your password
#define DB_NAME     "2_2_Encode_data.db"

volatile MQTTClient_deliveryToken deliveredtoken;

// Function to create database table if it doesn't exist
// void create_table(sqlite3 *db) {
//     const char *sql = "CREATE TABLE IF NOT EXISTS mqtt_data ("
//                       "id INTEGER PRIMARY KEY AUTOINCREMENT, "
//                       "timestamp TEXT NOT NULL, "
//                       "outcome TEXT NOT NULL);";  // Column to store base64 data
//     char *errmsg = 0;
//     int rc = sqlite3_exec(db, sql, 0, 0, &errmsg);
//     if (rc != SQLITE_OK) {
//         fprintf(stderr, "SQL error: %s\n", errmsg);
//         sqlite3_free(errmsg);
//     }
// }

void create_table(sqlite3 *db) {
    const char *sql = "CREATE TABLE IF NOT EXISTS mqtt_data ("
                      "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                      "timestamp TEXT NOT NULL, "
                      "outcome TEXT NOT NULL);";  // Added 'outcome' column
    char *errmsg = 0;
    int rc = sqlite3_exec(db, sql, 0, 0, &errmsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", errmsg);
        sqlite3_free(errmsg);
    }
}

// Function to insert data into the SQLite3 database
void insert_data(sqlite3 *db, const char *timestamp, const char *outcome) {
    char *errmsg = 0;
    char sql[1024];  // Increased buffer size to accommodate base64 data
    snprintf(sql, sizeof(sql), 
             "INSERT INTO mqtt_data (timestamp, outcome) VALUES ('%s', '%s');", 
             timestamp, outcome);
    
    int rc = sqlite3_exec(db, sql, 0, 0, &errmsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", errmsg);
        sqlite3_free(errmsg);
    } else {
        printf("Data inserted successfully: timestamp=%s, outcome=%s\n", timestamp, outcome);
    }
}

void delivered(void *context, MQTTClient_deliveryToken dt) {
    printf("Message with token value %d delivery confirmed\n", dt);
    deliveredtoken = dt;
}

int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message) {
    printf("Message arrived\n");
    printf("     topic: %s\n", topicName);
    printf("   message: %.*s\n", message->payloadlen, (char*)message->payload);

    // Parse JSON data
    cJSON *json = cJSON_Parse((char*)message->payload);
    if (json == NULL) {
        printf("Error parsing JSON\n");
    } else {
        // Extract timestamp and outcome
        cJSON *timestamp = cJSON_GetObjectItem(json, "timestamp_2_2");
        cJSON *outcome = cJSON_GetObjectItem(json, "outcome");

        if (timestamp && outcome && cJSON_IsString(timestamp) && cJSON_IsString(outcome)) {
            // Open the SQLite3 database
            sqlite3 *db;
            int rc = sqlite3_open(DB_NAME, &db);
            if (rc) {
                fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
            } else {
                create_table(db);  // Create table if it doesn't exist
                insert_data(db, timestamp->valuestring, outcome->valuestring);  // Insert base64 string
                sqlite3_close(db);
            }
        } else {
            printf("Invalid JSON format\n");
        }
        cJSON_Delete(json);  // Free JSON object
    }

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
    conn_opts.username = USERNAME;
    conn_opts.password = PASSWORD;

    MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd, delivered);

    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS) {
        printf("Failed to connect, return code %d\n", rc);
        return -1;
    }

    printf("Subscribing to topic %s for client %s using QoS%d\n", TOPIC, CLIENTID, QOS);
    MQTTClient_subscribe(client, TOPIC, QOS);

    // Keep the client running to receive messages
    while(1) {
        // You can add a sleep or other logic here if needed
    }

    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);
    return rc;
}

// gcc 2_2_Encode_Data.c -o 2_2_Encode_Data -lpaho-mqtt3c -lssl -lcrypto -lsqlite3 -lcjson -lpthread

// {
//   "timestamp_2_2" : "10:37:00", 
//   "outcome": "c29tZSBkYXRhIGluIGJhc2U2NCBlbmNvZGVkIGZvb3I="  // base64 encoded data
// }
