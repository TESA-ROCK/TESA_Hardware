frameLength = 256;

% Using dsp.AudioFileReader
fileReader = dsp.AudioFileReader('Day2/Faulty2.wav', 'SamplesPerFrame', frameLength);
Fs = fileReader.SampleRate;

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

% Load the trained neural network model
loadedNet = load('net.mat');
net = loadedNet.net;  % Assuming 'net' is the variable name within 'net.mat'

% Parameters for feature extraction
numMFCC = 13;  % Number of MFCC features
targetNumFeatures = 39;  % Expected input size for the model

% Wait for SYNC signal (sinusoid with amplitude larger than -30 dBFS)
while ~isDone(fileReader)
    signal = fileReader(); % Read a new frame
    if max(signal) > startThreshold
        maxPeak = max(signal);
        break;
    end
end

% STFT parameters
windowSize = 256; % Window size for STFT
overlap = round(windowSize / 2); % 50% overlap
nfft = 512; % Number of FFT points

% Initialize variables for cooldown period
threshold = 20;              % Peak threshold for detection
cooldownTime = 1.5;          % Cooldown time in seconds
lastTimestamp = -cooldownTime; % Initialize last timestamp outside cooldown
eventDetectedTimestamp = 0;  % Store timestamp of when the event was detected

while ~isDone(fileReader)
    signal = fileReader(); % Read a new frame

    % Apply the low-pass filter to the signal (to keep only frequencies < 300 Hz)
    filteredSignal = filter(lowPassFilter, signal);

    % Adaptive normalization
    signalPeak = max(filteredSignal);
    if signalPeak > maxPeak
        maxPeak = signalPeak;
    end
    filteredSignal = filteredSignal / maxPeak; % Scale the signal towards 1.0

    % Update audio buffers and manage buffer position
    frameLength = length(filteredSignal);
    if bufferPosition + frameLength - 1 > bufferLength
        moveOffset = bufferPosition + frameLength - 1 - bufferLength;
        audioBuffer(1:end-moveOffset) = audioBuffer(moveOffset+1:end);
        audioBufferOriginal(1:end-moveOffset) = audioBufferOriginal(moveOffset+1:end);
        bufferPosition = bufferPosition - moveOffset;
    end
    audioBuffer(bufferPosition:bufferPosition+frameLength-1) = filteredSignal;
    audioBufferOriginal(bufferPosition:bufferPosition+frameLength-1) = signal;
    bufferPosition = bufferPosition + frameLength;
    audioPositionSinceSync = audioPositionSinceSync + frameLength;

    % Compute STFT for filtered signal
    [S, F, ~] = stft(filteredSignal, Fs, 'Window', hamming(windowSize), ...
                     'OverlapLength', overlap, 'FFTLength', nfft);

    % Keep only the frequency bins below 300 Hz
    peakMagnitude = max(abs(S(:)));

    % Calculate timestamp and check if peak magnitude crosses threshold
    currentTimestamp = audioPositionSinceSync / Fs;
    if peakMagnitude > threshold && (currentTimestamp - lastTimestamp) >= cooldownTime
        lastTimestamp = currentTimestamp; % Update last timestamp
        eventDetectedTimestamp = currentTimestamp;
    end

    % Perform prediction with neural network if 2 seconds have passed since event detection
    if eventDetectedTimestamp > 0 && (currentTimestamp - eventDetectedTimestamp) >= 1.5
        segmentStart = max(1, bufferPosition - round(3 * Fs)); % Ensure we don't go out of bounds
        segmentEnd = bufferPosition - 1;
        segment = audioBufferOriginal(segmentStart:segmentEnd);
       
        % Extract features for the unknown sample
        mfccFeatures = mfcc(segment', Fs, 'NumCoeffs', numMFCC);
            
        % Calculate the same statistics used in training
        meanMFCC = mean(mfccFeatures, 1);
        stdMFCC = std(mfccFeatures, 1);
        maxMFCC = max(mfccFeatures, [], 1);
            
        % Form a feature vector with exactly 39 elements
        featureVector = [meanMFCC, stdMFCC, maxMFCC];
        featureVector = featureVector(1:targetNumFeatures);  % Ensure correct size
            
        % Reshape featureVector to a 1x39 matrix for prediction
        featureVector = reshape(featureVector, [1, targetNumFeatures]);
        
        % Get the prediction class
        prediction = classify(net, featureVector);
        
        % Calculate confidence scores if the model allows it
        scores = predict(net, featureVector);
        confidence = max(scores); % Confidence for the predicted class
        
        fprintf('| timestamp: %.2f s | predict: %c | confidence: %.2f |\n', currentTimestamp, prediction, confidence);

        eventDetectedTimestamp = 0;
    end
end

release(scope);
release(fileReader);
