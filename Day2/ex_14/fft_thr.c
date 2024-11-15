#include "sound_app.h"
#include "sound_freq.h"
#include <limits.h>

void *fft_thr_fcn(void *ptr) {
    // setup
    static double tmp_buf[4096];
    static double freq_buf[4096];

    while(1) {
        // loop
        pthread_mutex_lock(&data_cond_mutex);
        pthread_cond_wait(&data_cond, &data_cond_mutex);
        printf("Start processing\n");
        for (int i=0; i < 4096; i++) {
            tmp_buf[i] = (double)shared_buf[i]/SHRT_MAX;
            printf("tmp_buf[%d] = %f\n", i, tmp_buf[i]);
        }
        
        pthread_mutex_unlock(&data_cond_mutex);
    }
}