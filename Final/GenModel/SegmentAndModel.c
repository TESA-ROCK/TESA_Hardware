#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <sndfile.h>
#include <fftw3.h>

#include "matrix.h"
#include "mat.h"
#include "engine.h"




#define FRAME_LENGTH 256
#define BUFFER_LENGTH 5 * 44100 // Assuming 5 seconds buffer length
#define THRESHOLD 20
#define COOLDOWN_TIME 1.5
#define SAMPLE_RATE 44100

// Function prototypes
void apply_low_pass_filter(float* signal, int length, float cutoff_freq, int sample_rate);
void stft(float* signal, int length, int window_size, int overlap, int nfft, float* magnitude);
void extract_mfcc(float* signal, int length, int sample_rate, float* mfcc_features);
// void load_neural_network(const char* filename, void** net);
// void classify(void* net, float* feature_vector, int num_features, char* prediction);

void load_neural_network(const char* filename, mxArray** net);
void classify(mxArray* net, float* feature_vector, int num_features, char* prediction);




int main() {
    SNDFILE *file;
    SF_INFO sfinfo;
    float buffer[FRAME_LENGTH];
    float audio_buffer[BUFFER_LENGTH] = {0};
    float audio_buffer_original[BUFFER_LENGTH] = {0};
    int buffer_position = 0;
    int audio_position_since_sync = 0;
    float max_peak = 0.0;
    float start_threshold = pow(10, -30 / 20.0);
    float last_timestamp = -COOLDOWN_TIME;
    float event_detected_timestamp = 0;
    void* net;

    // Open the audio file
    file = sf_open("Day2/Day2.wav", SFM_READ, &sfinfo);
    if (!file) {
        fprintf(stderr, "Could not open audio file\n");
        return 1;
    }

    // Load the neural network
    load_neural_network("net.mat", &net);

    // Main loop for processing audio frames
    while (sf_read_float(file, buffer, FRAME_LENGTH) == FRAME_LENGTH) {
        // Apply low-pass filter
        apply_low_pass_filter(buffer, FRAME_LENGTH, 300, SAMPLE_RATE);

        // Adaptive normalization
        float signal_peak = 0.0;
        for (int i = 0; i < FRAME_LENGTH; i++) {
            if (fabs(buffer[i]) > signal_peak) {
                signal_peak = fabs(buffer[i]);
            }
        }
        if (signal_peak > max_peak) {
            max_peak = signal_peak;
        }
        for (int i = 0; i < FRAME_LENGTH; i++) {
            buffer[i] /= max_peak;
        }

        // Update the audio buffer
        if (buffer_position + FRAME_LENGTH > BUFFER_LENGTH) {
            memmove(audio_buffer, audio_buffer + FRAME_LENGTH, (BUFFER_LENGTH - FRAME_LENGTH) * sizeof(float));
            memmove(audio_buffer_original, audio_buffer_original + FRAME_LENGTH, (BUFFER_LENGTH - FRAME_LENGTH) * sizeof(float));
            buffer_position -= FRAME_LENGTH;
        }
        memcpy(audio_buffer + buffer_position, buffer, FRAME_LENGTH * sizeof(float));
        memcpy(audio_buffer_original + buffer_position, buffer, FRAME_LENGTH * sizeof(float));
        buffer_position += FRAME_LENGTH;
        audio_position_since_sync += FRAME_LENGTH;

        // Perform STFT and peak detection
        float magnitude[FRAME_LENGTH / 2 + 1];
        stft(buffer, FRAME_LENGTH, 256, 128, 512, magnitude);
        float peak_magnitude = 0.0;
        for (int i = 0; i < 300; i++) {
            if (magnitude[i] > peak_magnitude) {
                peak_magnitude = magnitude[i];
            }
        }

        // Check for event detection
        float current_timestamp = (float)audio_position_since_sync / SAMPLE_RATE;
        if (peak_magnitude > THRESHOLD && (current_timestamp - last_timestamp) >= COOLDOWN_TIME) {
            printf("Threshold crossed at timestamp: %.2f seconds\n", current_timestamp);
            last_timestamp = current_timestamp;
            event_detected_timestamp = current_timestamp;
        }

        // Perform prediction
        if (event_detected_timestamp > 0 && (current_timestamp - event_detected_timestamp) >= 2) {
            int segment_start = buffer_position - 3 * SAMPLE_RATE;
            if (segment_start < 0) segment_start = 0;
            int segment_end = buffer_position - 1;
            float segment[segment_end - segment_start + 1];
            memcpy(segment, audio_buffer_original + segment_start, (segment_end - segment_start + 1) * sizeof(float));

            // Extract MFCC features
            float mfcc_features[39];
            extract_mfcc(segment, segment_end - segment_start + 1, SAMPLE_RATE, mfcc_features);

            // Predict the class using the neural network
            char prediction[50];
            classify(net, mfcc_features, 39, prediction);
            printf("Prediction: %s\n", prediction);

            event_detected_timestamp = 0;
        }
    }

    // Close the audio file
    sf_close(file);

    return 0;
}



// Implement the function prototypes here

void apply_low_pass_filter(float* signal, int length, float cutoff_freq, int sample_rate) {
    // Simple low-pass filter implementation using a single-pole IIR filter
    float rc = 1.0 / (cutoff_freq * 2 * M_PI);
    float dt = 1.0 / sample_rate;
    float alpha = dt / (rc + dt);
    float previous = signal[0];
    for (int i = 1; i < length; i++) {
        signal[i] = previous + (alpha * (signal[i] - previous));
        previous = signal[i];
    }
}

void stft(float* signal, int length, int window_size, int overlap, int nfft, float* magnitude) {
    // Perform Short-Time Fourier Transform (STFT)
    int hop_size = window_size - overlap;
    int num_frames = (length - window_size) / hop_size + 1;
    fftw_complex* out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * nfft);
    fftw_plan p = fftw_plan_dft_r2c_1d(nfft, signal, out, FFTW_ESTIMATE);
    for (int i = 0; i < num_frames; i++) {
        int start = i * hop_size;
        fftw_execute_dft_r2c(p, signal + start, out);
        for (int j = 0; j < nfft / 2 + 1; j++) {
            magnitude[j] = sqrt(out[j][0] * out[j][0] + out[j][1] * out[j][1]);
        }
    }
    fftw_destroy_plan(p);
    fftw_free(out);
}

void extract_mfcc(float* signal, int length, int sample_rate, float* mfcc_features) {
    // Placeholder for MFCC extraction
    // You can use a library like libmfcc or implement your own MFCC extraction
    // For simplicity, we'll just fill the mfcc_features array with dummy values
    for (int i = 0; i < 39; i++) {
        mfcc_features[i] = (float)rand() / RAND_MAX;
    }
}

// void load_neural_network(const char* filename, void** net) {
//     // Placeholder for loading a neural network
//     // You can use a library like TensorFlow Lite or ONNX Runtime
//     // For simplicity, we'll just set net to NULL
//     *net = NULL;
// }

// void classify(void* net, float* feature_vector, int num_features, char* prediction) {
//     // Placeholder for classification
//     // You can use a library like TensorFlow Lite or ONNX Runtime
//     // For simplicity, we'll just set a dummy prediction
//     strcpy(prediction, "dummy_prediction");
// }




// Function prototypes
void load_neural_network(const char* filename, mxArray** net);
void classify(mxArray* net, float* feature_vector, int num_features, char* prediction);

void load_neural_network(const char* filename, mxArray** net) {
    MATFile* pmatFile;
    mxArray* pArray;

    // Open MAT-file
    pmatFile = matOpen(filename, "r");
    if (pmatFile == NULL) {
        fprintf(stderr, "Error opening MAT file %s\n", filename);
        exit(1);
    }

    // Read the variable 'net' from the MAT-file
    pArray = matGetVariable(pmatFile, "net");
    if (pArray == NULL) {
        fprintf(stderr, "Error reading variable 'net' from MAT file %s\n", filename);
        matClose(pmatFile);
        exit(1);
    }

    // Assign the loaded network to the output parameter
    *net = pArray;

    // Close MAT-file
    matClose(pmatFile);
}

void classify(mxArray* net, float* feature_vector, int num_features, char* prediction) {
    Engine* ep;
    mxArray* featureArray;
    mxArray* result;
    char buffer[256];

    // Start MATLAB engine
    if (!(ep = engOpen(NULL))) {
        fprintf(stderr, "\nCan't start MATLAB engine\n");
        exit(1);
    }

    // Create MATLAB array for feature vector
    featureArray = mxCreateDoubleMatrix(1, num_features, mxREAL);
    memcpy(mxGetPr(featureArray), feature_vector, num_features * sizeof(float));

    // Put the feature vector and network into the MATLAB workspace
    engPutVariable(ep, "feature_vector", featureArray);
    engPutVariable(ep, "net", net);

    // Run the classification command in MATLAB
    engEvalString(ep, "prediction = classify(net, feature_vector);");

    // Get the result from MATLAB
    result = engGetVariable(ep, "prediction");
    if (result == NULL) {
        fprintf(stderr, "Error getting prediction from MATLAB\n");
        engClose(ep);
        exit(1);
    }

    // Copy the result to the prediction output parameter
    mxGetString(result, prediction, 50);

    // Clean up
    mxDestroyArray(featureArray);
    mxDestroyArray(result);
    engClose(ep);
}

int main() {
    // Example usage
    mxArray* net;
    load_neural_network("net.mat", &net);

    float feature_vector[39] = {0}; // Example feature vector
    char prediction[50];
    classify(net, feature_vector, 39, prediction);

    printf("Prediction: %s\n", prediction);

    // Clean up
    mxDestroyArray(net);

    return 0;
}