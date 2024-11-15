#include "segment.h"

float audioBuffer[BUFFER_LENGTH];
float audioBufferOriginal[BUFFER_LENGTH];
int bufferPosition = 0;
float maxPeak = 0.0;
float startThreshold = THRESHOLD;
float lastTimestamp = -COOLDOWN_TIME;
float eventDetectedTimestamp = 0.0;

void process_audio_frame(float *frame, int frameLength, float sampleRate) {
    // Apply low-pass filter (simple example, replace with actual filter design)
    for (int i = 0; i < frameLength; i++) {
        frame[i] = frame[i] * (i < FREQ_THRESHOLD ? 1.0 : 0.0);
    }

    // Adaptive normalization
    float signalPeak = 0.0;
    for (int i = 0; i < frameLength; i++) {
        if (fabs(frame[i]) > signalPeak) {
            signalPeak = fabs(frame[i]);
        }
    }
    if (signalPeak > maxPeak) {
        maxPeak = signalPeak;
    }
    for (int i = 0; i < frameLength; i++) {
        frame[i] /= maxPeak;
    }

    // Handle audio buffer for filtered signal
    if (bufferPosition + frameLength > BUFFER_LENGTH) {
        memmove(audioBuffer, audioBuffer + frameLength, (BUFFER_LENGTH - frameLength) * sizeof(float));
        bufferPosition -= frameLength;
    }
    memcpy(audioBuffer + bufferPosition, frame, frameLength * sizeof(float));
    bufferPosition += frameLength;

    // Handle audio buffer for original signal
    if (bufferPosition + frameLength > BUFFER_LENGTH) {
        memmove(audioBufferOriginal, audioBufferOriginal + frameLength, (BUFFER_LENGTH - frameLength) * sizeof(float));
        bufferPosition -= frameLength;
    }
    memcpy(audioBufferOriginal + bufferPosition, frame, frameLength * sizeof(float));

    // Compute STFT (simple example, replace with actual STFT computation)
    float peakMagnitude = 0.0;
    for (int i = 0; i < frameLength; i++) {
        if (fabs(frame[i]) > peakMagnitude) {
            peakMagnitude = fabs(frame[i]);
        }
    }

    // Calculate timestamp
    float currentTimestamp = (float)bufferPosition / sampleRate;

    // Check if the peak magnitude crosses the threshold and print timestamp if outside cooldown
    if (peakMagnitude > THRESHOLD && (currentTimestamp - lastTimestamp) >= COOLDOWN_TIME) {
        printf("Threshold crossed at timestamp: %.2f seconds\n", currentTimestamp);
        lastTimestamp = currentTimestamp;
        eventDetectedTimestamp = currentTimestamp;
    }

    // If event was detected, and 2 seconds have passed
    if (eventDetectedTimestamp > 0 && (currentTimestamp - eventDetectedTimestamp) >= 2) {
        // Extract the 3-second segment from the original audio buffer
        int segmentStart = bufferPosition - (3 * SAMPLE_RATE);
        if (segmentStart < 0) segmentStart = 0;
        int segmentEnd = bufferPosition;
        int segmentLength = segmentEnd - segmentStart;

        // Save the segment to a WAV file
        char outputFileName[256];
        snprintf(outputFileName, sizeof(outputFileName), "Segment_%.2f.wav", eventDetectedTimestamp);
        SF_INFO sfinfo;
        sfinfo.channels = 1;
        sfinfo.samplerate = SAMPLE_RATE;
        sfinfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
        SNDFILE *outfile = sf_open(outputFileName, SFM_WRITE, &sfinfo);
        if (!outfile) {
            fprintf(stderr, "Error opening output file: %s\n", sf_strerror(NULL));
            return;
        }
        sf_write_float(outfile, audioBufferOriginal + segmentStart, segmentLength);
        sf_close(outfile);
        printf("Segment saved: %s\n", outputFileName);

        // Reset eventDetectedTimestamp to avoid multiple saving of segments for the same event
        eventDetectedTimestamp = 0;
    }
}