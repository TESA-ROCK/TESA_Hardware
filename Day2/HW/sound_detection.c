#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>
#include <time.h>
#include <unistd.h>

#define THRESHOLD 100
#define DEVICE "plughw:2,0"
#define PERIOD_SIZE 1024

snd_pcm_t *pcm_handle; // PCM handle for the ALSA device
snd_pcm_hw_params_t *params; // Hardware parameters for the PCM device
unsigned int rate = 48000; // Sampling rate set to 48000 Hz
int dir; // Direction for the rate (0 = nearest)
snd_pcm_uframes_t frames = PERIOD_SIZE; // Number of frames per period

void get_thai_timestamp(char *buffer, size_t buffer_size) {
    time_t rawtime;
    struct tm *timeinfo;
    time(&rawtime);
    rawtime += 7 * 3600; // Add 7 hours for Thai time
    timeinfo = localtime(&rawtime);
    strftime(buffer, buffer_size, "%H:%M:%S", timeinfo);
}

int main() {
    int rc;
    int size;
    char *buffer;
    int sound_detected = 0;

    // Open PCM device for recording (capture)
    rc = snd_pcm_open(&pcm_handle, DEVICE, SND_PCM_STREAM_CAPTURE, 0);
    if (rc < 0) {
        fprintf(stderr, "unable to open pcm device: %s\n", snd_strerror(rc));
        exit(1);
    }

    // Allocate a hardware parameters object
    snd_pcm_hw_params_alloca(&params);

    // Fill it in with default values
    snd_pcm_hw_params_any(pcm_handle, params);

    // Set the desired hardware parameters
    snd_pcm_hw_params_set_access(pcm_handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(pcm_handle, params, SND_PCM_FORMAT_S16_LE);
    snd_pcm_hw_params_set_channels(pcm_handle, params, 1);
    snd_pcm_hw_params_set_rate_near(pcm_handle, params, &rate, &dir);
    snd_pcm_hw_params_set_period_size_near(pcm_handle, params, &frames, &dir);

    // Write the parameters to the driver
    rc = snd_pcm_hw_params(pcm_handle, params);
    if (rc < 0) {
        fprintf(stderr, "unable to set hw parameters: %s\n", snd_strerror(rc));
        exit(1);
    }

    // Use a buffer large enough to hold one period
    snd_pcm_hw_params_get_period_size(params, &frames, &dir);
    size = frames * 2; // 2 bytes/sample, 1 channel
    buffer = (char *) malloc(size);

    while (1) {
        rc = snd_pcm_readi(pcm_handle, buffer, frames);
        if (rc == -EPIPE) {
            // EPIPE means overrun
            fprintf(stderr, "overrun occurred\n");
            snd_pcm_prepare(pcm_handle);
        } else if (rc < 0) {
            fprintf(stderr, "error from read: %s\n", snd_strerror(rc));
        } else if (rc != (int)frames) {
            fprintf(stderr, "short read, read %d frames\n", rc);
        }

        // Compute volume level
        int max_volume = 0;
        for (int i = 0; i < size; i += 2) {
            int16_t sample = buffer[i] | (buffer[i + 1] << 8);
            int volume = abs(sample);
            if (volume > max_volume) {
                max_volume = volume;
            }
        }

        char timestamp[9];
        get_thai_timestamp(timestamp, sizeof(timestamp));

        if (max_volume > THRESHOLD) {
            if (!sound_detected) {
                printf("sound detected at %s\n", timestamp);
                sound_detected = 1;
            }
        } else {
            if (sound_detected) {
                printf("sound silent at %s\n", timestamp);
                sound_detected = 0;
            } else {
                printf(".");
                fflush(stdout);
            }
        }
    }

    snd_pcm_drain(pcm_handle);
    snd_pcm_close(pcm_handle);
    free(buffer);

    return 0;
}