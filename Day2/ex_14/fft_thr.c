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
        }

        
        // Process FFT and store in freq_buf
        for (int i = 0; i < 4096; i++) {
            freq_buf[i] = tmp_buf[i];  // Here, we store the normalized buffer
        }

        // Print 3 frequency values from the spectrum
        for (int i = 0; i < 3; i++) {
            printf("Frequency[%d] = %.5f Hz\n", i, freq_buf[i]);  // This is an example, assuming frequency values
        }

        pthread_mutex_unlock(&data_cond_mutex);
    }
}