#include "mqtt_sub_app.h"

pthread_mutex_t data_cond_mutex;
pthread_cond_t data_cond;
int shared_value = 0;

int main(int argc, char *argv[]) {
    pthread_t mqtt_sub_thr;

    // Prepare IPC objects
    pthread_mutex_init(&data_cond_mutex, NULL);

    // Initialize MQTT subscription thread
    pthread_create(&mqtt_sub_thr, NULL, mqtt_sub_thr_fcn, (void *)argv[1]);

    // Wait for thread termination
    pthread_join(mqtt_sub_thr, NULL);
}