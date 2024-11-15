import alsaaudio
import time
from datetime import datetime, timedelta

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
    
def get_thai_timestamp():
    return (datetime.now() + timedelta(hours=7)).strftime('%H:%M:%S')

def main():
    sound_detected = False
    try:
        while True:
            # Read audio data
            try:
                length, data = pcm_input.read()
            except alsaaudio.ALSAAudioError as e:
                print(f"Error reading audio data: {e}")
                time.sleep(0.01)
                continue

            if length <= 0:
                print(f"Error reading audio data: {length}")
                continue

            # Compute volume level
            audio_data = [abs(int.from_bytes(data[i:i+2], byteorder='little', signed=True))
                          for i in range(0, len(data), 2)]
            volume = max(audio_data)

            if volume > THRESHOLD:
                if not sound_detected:
                    print(f"sound detected at {get_thai_timestamp()}")
                    sound_detected = True
            else:
                if sound_detected:
                    print(f"sound silent at {get_thai_timestamp()}")
                    sound_detected = False
                else:
                    print(".", end='', flush=True)
    except KeyboardInterrupt:
        print("\nProgram terminated")

if __name__ == "__main__":
    main()
