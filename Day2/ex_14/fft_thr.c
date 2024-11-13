#include "sound_app.h"
#include "sound_freq.h"
#include <limits.h>
#include <fftw3.h>
#include <pthread.h>
#include <stdio.h>
#include <math.h>

extern pthread_mutex_t data_cond_mutex;
extern pthread_cond_t data_cond;
extern short shared_buf[4096];

void *fft_thr_fcn(void *ptr) {
    // setup
    static double tmp_buf[4096];
    static fftw_complex freq_buf[4096];
    fftw_plan plan = fftw_plan_dft_r2c_1d(4096, tmp_buf, freq_buf, FFTW_ESTIMATE);

    while(1) {
        // loop
        pthread_mutex_lock(&data_cond_mutex);
        pthread_cond_wait(&data_cond, &data_cond_mutex);
        printf("Start processing\n");
        for (int i = 0; i < 4096; i++) {
            tmp_buf[i] = (double)shared_buf[i] / SHRT_MAX;
        }

        // Perform FFT
        fftw_execute(plan);

        // Print the magnitudes of the first three frequency values
        for (int i = 0; i < 3; i++) {
            double magnitude = sqrt(freq_buf[i][0] * freq_buf[i][0] + freq_buf[i][1] * freq_buf[i][1]);
            printf("freq_buf[%d] magnitude = %f\n", i, magnitude);
        }

        pthread_mutex_unlock(&data_cond_mutex);
    }

    fftw_destroy_plan(plan);
    return NULL;
}