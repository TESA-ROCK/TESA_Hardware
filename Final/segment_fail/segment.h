#ifndef SEGMENT_H
#define SEGMENT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include <curl/curl.h>
#include <alsa/asoundlib.h>
#include <time.h>
#include <sndfile.h>

// Shared constants
#define FRAME_LENGTH 256
#define SAMPLE_RATE 48000
#define BUFFER_LENGTH (5 * SAMPLE_RATE)
#define THRESHOLD 0.0316 // -30 dBFS in linear scale
#define COOLDOWN_TIME 1.5
#define WINDOW_SIZE 256
#define OVERLAP (WINDOW_SIZE / 2)
#define NFFT 512
#define FREQ_THRESHOLD 300
#define PERIOD_SIZE 160
#define CHUNK_DURATION 5 // seconds
#define DEVICE "default"
#define FILENAME_FORMAT "s%d.c%d.%s.wav"
#define API_URL "http://192.168.164.139:5000/api/sound_files"
#define TOKEN "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJ1c2VyIjoiYWRtaW4iLCJleHAiOjE3MzQxOTM3NDJ9.2Jb3jwWmEV4eqQf_l3N7PsKX3-cjbWiSjZHL8PBajLs"
#define DETECTION_THRESHOLD 100

// Shared variables
extern float audioBuffer[BUFFER_LENGTH];
extern float audioBufferOriginal[BUFFER_LENGTH];
extern int bufferPosition;
extern float maxPeak;
extern float startThreshold;
extern float lastTimestamp;
extern float eventDetectedTimestamp;

// Function declarations
void process_audio_frame(float *frame, int frameLength, float sampleRate);
void get_thai_timestamp(char *buffer, size_t buffer_size);
void write_wav_header(FILE *file, int sample_rate, int num_channels, int bits_per_sample, int num_samples);
int is_sound_detected(const char *buffer, int size);
void *record_chunks(void *arg);
void *send_chunks(void *arg);

#endif // SEGMENT_H