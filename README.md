# TESA_Hardware

## Final
This folder is the final challenge problem in this competition. In this folder, it stored many folder of coder I writed, I named each folder shortly by specifying to the main tasks of the foder.

---------------------------------------------------------------------------------------------------------------------------------
## sound_detect
### sound_detection.c
- read sound from jack adapter which connect with USB of Raspberry Pi
- print "sound is detected at <timestamp>" in terminal when sound that is higher than specified Threshold was detected and then print "sound is silent at <timestamp>" in terminal when the detected sound was lower than Threshold
---------------------------------------------------------------------------------------------------------------------------------
## send_codec
### send_codec.c
- set API_URL with target server url to select Target server
- set Token to be server's token
- send .wav file that we select to target servers

### sound_detection.c
work as multi-thread
  1) record_chucks
     - set the desired hardware parameters
     - read sound from jack adapter which connect with USB of Raspberry Pi continuously, specify hardware with `pcm_handle` variable and pointer to memory that the sound data will be stored with `buffer` variable, and using `rc = snd_pcm_readi(pcm_handle, buffer, frames)` that using function `snd_pcm_readi` will return value, stored in the variable rc,indicates the number of frames actually read.
     - Write .wav file header using `write_wav_header` function and Add each chunk to `chuck_queue` for send it to server by `send_chunk` thread
  2)  send_chunk
    - Add file part for send to server using this command `curl_mime_filedata(part, chunk)` which chunk use `char *chunk = chunk_queue[--queue_size];` command that retrieves an element from a queue and assigns it to a pointer variable named `chunk`.
    - Send chunk using upload_file function `curl_easy_perform` using this command `res = curl_easy_perform(curl)`, is a line of C code that uses the libcurl library to perform a file transfer operation. `curl_easy_perform` is a function provided by libcurl, a popular library for transferring data with URLs.
