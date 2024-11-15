frameLength = 256;

% Using dsp.AudioFileReader
fileReader = dsp.AudioFileReader('Day2/Day2.wav', 'SamplesPerFrame', frameLength);
Fs = fileReader.SampleRate;

scope = timescope('SampleRate', Fs, 'TimeSpan', 2, 'BufferLength', Fs * 2, ...
                  'YLimits', [0, 20], 'TimeSpanOverrunAction', 'scroll');

bufferLength = round(5.0 * Fs);
audioBuffer = zeros(1, bufferLength); % Filtered signal buffer
audioBufferOriginal = zeros(1, bufferLength); % Original audio buffer (no filter)
bufferPosition = 1;
audioPositionSinceSync = 1;

maxPeak = 0.0;
startThreshold = db2mag(-30);

% Design a low-pass filter with a cutoff frequency of 300 Hz
lowPassFilter = designfilt('lowpassiir', 'FilterOrder', 8, ...
                           'PassbandFrequency', 300, 'PassbandRipple', 0.2, ...
                           'SampleRate', Fs);

% Create a new folder to save segmented audio files
outputFolder = 'SegmentedAudio';
if ~exist(outputFolder, 'dir')
    mkdir(outputFolder);
end

% Wait for SYNC signal (sinusoid with amplitude larger than -30 dBFS)
while ~isDone(fileReader)
    signal = fileReader(); % read a new frame
    if max(signal) > startThreshold
        maxPeak = max(signal);
        break;
    end
end

% STFT parameters
windowSize = 256; % Window size for STFT
overlap = round(windowSize / 2); % 50% overlap
nfft = 512; % Number of FFT points

% Initialize variables to manage cooldown period
threshold = 19;              % Peak threshold for detection
cooldownTime = 1.5;          % Cooldown time in seconds
lastTimestamp = -cooldownTime; % Initialize last timestamp outside cooldown
eventDetectedTimestamp = 0;  % Store timestamp of when the event was detected

while ~isDone(fileReader)
    signal = fileReader(); % read a new frame

    % Apply the low-pass filter to the signal (to keep only frequencies < 300 Hz)
    filteredSignal = filter(lowPassFilter, signal);

    % Adaptive normalization
    signalPeak = max(filteredSignal);
    if signalPeak > maxPeak
        maxPeak = signalPeak;
    end
    filteredSignal = filteredSignal / maxPeak; % scale the signal towards 1.0

    % Handle audio buffer for filtered signal
    frameLength = length(filteredSignal);
    emptyLength = bufferLength - bufferPosition + 1;
    % Make space for the new frame 
    if emptyLength < frameLength
        moveOffset = frameLength - emptyLength;
        audioBuffer(1:bufferPosition-moveOffset-1) = audioBuffer(moveOffset+1:bufferPosition-1);
        bufferPosition = bufferPosition - moveOffset;
    end
    % Fill in the buffer with the new filtered frame
    audioBuffer(bufferPosition:bufferPosition+frameLength-1) = filteredSignal;
    bufferPosition = bufferPosition + frameLength;
    audioPositionSinceSync = audioPositionSinceSync + frameLength;

    % Handle audio buffer for original signal (no filter)
    emptyLengthOriginal = bufferLength - bufferPosition + 1;
    if emptyLengthOriginal < frameLength
        moveOffset = frameLength - emptyLengthOriginal;
        audioBufferOriginal(1:bufferPosition-moveOffset-1) = audioBufferOriginal(moveOffset+1:bufferPosition-1);
        bufferPosition = bufferPosition - moveOffset;
    end
    % Fill in the buffer with the new original frame
    audioBufferOriginal(bufferPosition:bufferPosition+frameLength-1) = signal;

    % Compute STFT
    [S, F, T] = stft(filteredSignal, Fs, 'Window', hamming(windowSize), 'OverlapLength', overlap, 'FFTLength', nfft);

    % Find the indices of the frequencies below 300 Hz
    freqThreshold = 300;
    freqIndices = F <= freqThreshold;

    % Keep only the frequency bins below 300 Hz
    S_filtered = S(freqIndices, :);

    % Calculate the peak magnitude in the selected frequency range
    peakMagnitude = max(abs(S_filtered(:)));

    % Calculate timestamp
    currentTimestamp = audioPositionSinceSync / Fs;

    % Check if the peak magnitude crosses the threshold and print timestamp if outside cooldown
    if peakMagnitude > threshold && (currentTimestamp - lastTimestamp) >= cooldownTime
        fprintf('Threshold crossed at timestamp: %.2f seconds\n', currentTimestamp);
        lastTimestamp = currentTimestamp; % Update last timestamp
        
        % Store the event timestamp for delayed processing
        eventDetectedTimestamp = currentTimestamp;
    end

    % If event was detected, and 2 seconds have passed
    if eventDetectedTimestamp > 0 && (currentTimestamp - eventDetectedTimestamp) >= 2
        % Extract the 3-second segment from the original audio buffer
        segmentStart = max(1, bufferPosition - round(3 * Fs)); % Ensure we don't go out of bounds
        segmentEnd = bufferPosition - 1;
        segment = audioBufferOriginal(segmentStart:segmentEnd);

        % Save the segment to the new folder as a WAV file
        outputFileName = fullfile(outputFolder, sprintf('Segment_%.2f.wav', eventDetectedTimestamp));
        audiowrite(outputFileName, segment, Fs);
        disp(['Segment saved: ', outputFileName]);

        % Reset eventDetectedTimestamp to avoid multiple saving of segments for the same event
        eventDetectedTimestamp = 0;
    end

    % Plot the peak magnitude over time using timescope
    scope(peakMagnitude);  % Show the peak magnitude in the timescope

    % Segment, classify, and report results here based on peak analysis
    % deviceWriter(filteredSignal);
end

release(scope);
release(fileReader);