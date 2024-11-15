import alsaaudio
import time
from datetime import datetime, timedelta
import numpy as np
import librosa
import tensorflow as tf
from scipy.signal import butter, lfilter
import json
import threading

# Configuration
THRESHOLD = 100  # Adjust this as needed
DEVICE = "plughw:2,0"  # Use the ALSA device identifier from `arecord -l`
PERIOD_SIZE = 1024  # Buffer size for audio capture

# Setup ALSA PCM with the correct device argument
try:
    pcm_input = alsaaudio.PCM(type=alsaaudio.PCM_CAPTURE, mode=alsaaudio.PCM_NORMAL, device=DEVICE,
                              channels=1, rate=48000, format=alsaaudio.PCM_FORMAT_S16_LE, periodsize=PERIOD_SIZE)
except alsaaudio.ALSAAudioError as e:
    print(f"Failed to set up ALSA PCM: {e}")
    exit(1)

# Load the model data from JSON
with open('architecture.json', 'r') as f:
    model_data = json.load(f)

# Create the model architecture based on JSON data
model = tf.keras.Sequential()

# Loop through each layer and add to model
for layer_data in model_data:
    if layer_data['Name'] == 'input':
        input_size = layer_data['InputSize']
        model.add(tf.keras.layers.Input(shape=(input_size,)))
    elif layer_data['Name'] == 'fc1':
        model.add(tf.keras.layers.Dense(
            layer_data['OutputSize'],
            activation='relu',
            name=layer_data['Name']
        ))
    elif layer_data['Name'] == 'fc2':
        model.add(tf.keras.layers.Dense(
            layer_data['OutputSize'],
            activation='softmax',
            name=layer_data['Name']
        ))

# Compile the model
model.compile(optimizer='adam', loss='categorical_crossentropy')

# Set the weights for each layer based on JSON data
for layer_data in model_data:
    if layer_data['Name'] in ['fc1', 'fc2']:  # Only process layers with weights
        weights = np.array(layer_data['Weights']).T  # Transpose weights to match expected shape
        biases = np.array(layer_data['Bias'])
        layer = model.get_layer(name=layer_data['Name'])
        layer.set_weights([weights, biases])

# Low-pass filter design (300 Hz cutoff)
def butter_lowpass_filter(data, cutoff, fs, order=8):
    nyquist = 0.5 * fs
    normal_cutoff = cutoff / nyquist
    b, a = butter(order, normal_cutoff, btype='low', analog=False)
    return lfilter(b, a, data)

# Feature extraction function
def extract_features(audio_segment, sample_rate, num_mfcc, target_num_features):
    mfccs = librosa.feature.mfcc(y=audio_segment, sr=sample_rate, n_mfcc=num_mfcc)
    mean_mfcc = np.mean(mfccs, axis=1)
    std_mfcc = np.std(mfccs, axis=1)
    max_mfcc = np.max(mfccs, axis=1)
    feature_vector = np.hstack([mean_mfcc, std_mfcc, max_mfcc])[:target_num_features]
    return feature_vector.reshape(1, -1)

# Prediction function using the TensorFlow model
def predict(features):
    prediction = model.predict(features)
    return prediction

def get_thai_timestamp():
    return (datetime.now() + timedelta(hours=7)).strftime('%H:%M:%S')

def audio_capture_thread():
    buffer_length = int(5.0 * 48000)
    audio_buffer = np.zeros(buffer_length)
    audio_buffer_original = np.zeros(buffer_length)
    buffer_position = 0
    audio_position_since_sync = 0
    max_peak = 0.0
    last_timestamp = -1.5  # Initialize last timestamp outside cooldown
    event_detected_timestamp = 0  # Initialize to 0
    num_mfcc = 13
    target_num_features = 39
    threshold = 20
    cooldown_time = 1.5

    while True:
        length, data = pcm_input.read()
        if length > 0:
            frame = np.frombuffer(data, dtype='int16')
            if max(frame) > THRESHOLD:
                filtered_signal = butter_lowpass_filter(frame, 300, 48000)
                signal_peak = max(filtered_signal)
                if signal_peak > max_peak:
                    max_peak = signal_peak
                if max_peak > 0:
                    filtered_signal /= max_peak
                else:
                    filtered_signal = np.zeros_like(filtered_signal)
                frame_length = len(filtered_signal)
                if buffer_position + frame_length > buffer_length:
                    move_offset = buffer_position + frame_length - buffer_length
                    audio_buffer[:-move_offset] = audio_buffer[move_offset:]
                    audio_buffer_original[:-move_offset] = audio_buffer_original[move_offset:]
                    buffer_position -= move_offset
                audio_buffer[buffer_position:buffer_position + frame_length] = filtered_signal
                audio_buffer_original[buffer_position:buffer_position + frame_length] = frame
                buffer_position += frame_length
                audio_position_since_sync += frame_length
                S = librosa.stft(filtered_signal, n_fft=1024, hop_length=frame_length // 2, win_length=frame_length)
                peak_magnitude = np.max(np.abs(S))
                current_timestamp = audio_position_since_sync / 48000
                if peak_magnitude > threshold and (current_timestamp - last_timestamp) >= cooldown_time:
                    last_timestamp = current_timestamp
                    event_detected_timestamp = current_timestamp
                if event_detected_timestamp > 0 and (current_timestamp - event_detected_timestamp) >= 1.5:
                    segment_start = max(0, buffer_position - int(3 * 48000))
                    segment_end = buffer_position
                    segment = audio_buffer_original[segment_start:segment_end]
                    features = extract_features(segment, 48000, num_mfcc, target_num_features)
                    prediction = predict(features)
                    predicted_class = np.argmax(prediction, axis=-1)
                    confidence = np.max(prediction)
                    class_name = {0: 'N', 1: 'F', 2: 'X'}.get(predicted_class[0], "Unknown")
                    print(f'| timestamp: {current_timestamp:.2f} s | predict: {class_name} | confidence: {confidence:.2f} |')
                    event_detected_timestamp = 0

def main():
    sound_detected = False
    audio_thread = threading.Thread(target=audio_capture_thread)
    audio_thread.start()
    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        print("Exiting...")
        exit(0)

if __name__ == "__main__":
    main()
