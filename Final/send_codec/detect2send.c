#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <curl/curl.h>
#include <alsa/asoundlib.h>
#include <time.h>

#define PERIOD_SIZE 160
#define CHUNK_DURATION 5 // seconds
#define DEVICE "plughw:2,0"
#define FILENAME_FORMAT "s%d.c%d.%s.wav"
#define API_URL "http://100.94.191.63:5000/api/sound_files"
#define TOKEN "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJ1c2VyIjoiYWRtaW4iLCJleHAiOjE3MzQyNDExMjJ9.bl68HX9hZjw412Y3joYwWf3WCKVuDuDt9IArfuZXzXs"
#define THRESHOLD 200

snd_pcm_t *pcm_handle;
pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t queue_cond = PTHREAD_COND_INITIALIZER;
char *chunk_queue[10];
int queue_size = 0;

void get_thai_timestamp(char *buffer, size_t buffer_size) {
    time_t rawtime;
    struct tm *timeinfo;
    time(&rawtime);
    // rawtime += 7 * 3600; // Add 7 hours for Thai time
    timeinfo = localtime(&rawtime);
    strftime(buffer, buffer_size, "%d-%m-%Y_%H:%M:%S", timeinfo);
}

void write_wav_header(FILE *file, int sample_rate, int num_channels, int bits_per_sample, int num_samples) {
    int byte_rate = sample_rate * num_channels * bits_per_sample / 8;
    int block_align = num_channels * bits_per_sample / 8;
    int data_size = num_samples * num_channels * bits_per_sample / 8;
    int chunk_size = 36 + data_size;

    fwrite("RIFF", 1, 4, file);
    fwrite(&chunk_size, 4, 1, file);
    fwrite("WAVE", 1, 4, file);
    fwrite("fmt ", 1, 4, file);

    int subchunk1_size = 16;
    short audio_format = 1;
    fwrite(&subchunk1_size, 4, 1, file);
    fwrite(&audio_format, 2, 1, file);
    fwrite(&num_channels, 2, 1, file);
    fwrite(&sample_rate, 4, 1, file);
    fwrite(&byte_rate, 4, 1, file);
    fwrite(&block_align, 2, 1, file);
    fwrite(&bits_per_sample, 2, 1, file);

    fwrite("data", 1, 4, file);
    fwrite(&data_size, 4, 1, file);
}

int is_sound_detected(const char *buffer, int size) {
    int sum = 0;
    for (int i = 0; i < size; i += 2) {
        short sample = buffer[i] | (buffer[i + 1] << 8);
        sum += abs(sample);
    }
    return (sum / (size / 2)) > THRESHOLD;
}

void *record_chunks(void *arg) {
    int rc;
    int size;
    snd_pcm_uframes_t frames;
    char *buffer;
    int sound_detected = 0;
    int section_count = 0;
    int chunk_count = 0;
    int recording = 0;

    // Open PCM device for recording (capture)
    rc = snd_pcm_open(&pcm_handle, DEVICE, SND_PCM_STREAM_CAPTURE, 0);
    if (rc < 0) {
        fprintf(stderr, "unable to open pcm device: %s\n", snd_strerror(rc));
        exit(1);
    }

    // Allocate a hardware parameters object
    snd_pcm_hw_params_t *params;
    snd_pcm_hw_params_alloca(&params);

    // Fill it in with default values
    snd_pcm_hw_params_any(pcm_handle, params);

    // Set the desired hardware parameters
    snd_pcm_hw_params_set_access(pcm_handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(pcm_handle, params, SND_PCM_FORMAT_S16_LE);
    snd_pcm_hw_params_set_channels(pcm_handle, params, 1);
    unsigned int sample_rate = 48000;
    snd_pcm_hw_params_set_rate_near(pcm_handle, params, &sample_rate, NULL);
    snd_pcm_hw_params_set_period_size_near(pcm_handle, params, &frames, NULL);

    // Write the parameters to the driver
    rc = snd_pcm_hw_params(pcm_handle, params);
    if (rc < 0) {
        fprintf(stderr, "unable to set hw parameters: %s\n", snd_strerror(rc));
        exit(1);
    }

    // Use a buffer large enough to hold one period
    snd_pcm_hw_params_get_period_size(params, &frames, 0);
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

        if (is_sound_detected(buffer, size)) {
            if (!recording) {
                recording = 1;
                chunk_count = 0;
                section_count++;
            }

            // Record sound in chunks
            char timestamp[20];
            get_thai_timestamp(timestamp, sizeof(timestamp));
            char filename[256];
            snprintf(filename, sizeof(filename), FILENAME_FORMAT, section_count, chunk_count, timestamp);
            FILE *file = fopen(filename, "wb");
            if (!file) {
                fprintf(stderr, "unable to open file: %s\n", filename);
                exit(1);
            }

            // Write WAV header with placeholder values
            write_wav_header(file, sample_rate, 1, 16, CHUNK_DURATION * sample_rate);

            int total_frames = 0;
            for (int i = 0; i < CHUNK_DURATION * sample_rate / frames; i++) {
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

                fwrite(buffer, size, 1, file);
                total_frames += frames;
            }

            // Update WAV header with correct values
            fseek(file, 0, SEEK_SET);
            write_wav_header(file, sample_rate, 1, 16, total_frames);

            fclose(file);

            // Add chunk to queue
            pthread_mutex_lock(&queue_mutex);
            chunk_queue[queue_size++] = strdup(filename);
            pthread_cond_signal(&queue_cond);
            pthread_mutex_unlock(&queue_mutex);

            chunk_count++;
        } else {
            if (recording) {
                recording = 0;
            }
        }
    }

    free(buffer);
    snd_pcm_drain(pcm_handle);
    snd_pcm_close(pcm_handle);
    return NULL;
}

void *send_chunks(void *arg) {
    while (1) {
        pthread_mutex_lock(&queue_mutex);
        while (queue_size == 0) {
            pthread_cond_wait(&queue_cond, &queue_mutex);
        }

        char *chunk = chunk_queue[--queue_size];
        pthread_mutex_unlock(&queue_mutex);

        // Send chunk using upload_file function
        CURL *curl;
        CURLcode res;

        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl = curl_easy_init();

        if (curl) {
            struct curl_slist *headers = NULL;

            // Set Authorization header with the JWT token
            headers = curl_slist_append(headers, "Authorization: Bearer " TOKEN);

            // Specify URL
            curl_easy_setopt(curl, CURLOPT_URL, API_URL);

            // Enable verbose output for debugging (optional)
            // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

            // Create a multipart/form-data POST request
            curl_mime *mime;
            curl_mimepart *part;

            mime = curl_mime_init(curl);

            // Add file part
            part = curl_mime_addpart(mime);
            curl_mime_name(part, "file");
            curl_mime_filedata(part, chunk);

            // Attach the mime structure to the handle
            curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

            // Perform the request
            res = curl_easy_perform(curl);
            if (res != CURLE_OK)
                fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

            // Cleanup
            curl_mime_free(mime);
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
        }

        curl_global_cleanup();
        free(chunk);
    }
    return NULL;
}

int main() {
    pthread_t record_thread, send_thread;

    // Create threads
    pthread_create(&record_thread, NULL, record_chunks, NULL);
    pthread_create(&send_thread, NULL, send_chunks, NULL);

    // Wait for threads to finish
    pthread_join(record_thread, NULL);
    pthread_join(send_thread, NULL);

    return 0;
}