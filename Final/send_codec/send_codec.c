#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#define FILENAME "sample.wav"
#define API_URL "http://192.168.164.139:5000/api/sound_files"
#define TOKEN "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJ1c2VyIjoiYWRtaW4iLCJleHAiOjE3MzE1Nzg1MDV9.KDJ_xVVl0SOWNOPEgMe7xVit0vDHf8ugf0vGrrBaI_E"

// Function to perform the HTTP POST request with multipart/form-data
void upload_file() {
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
        curl_mime_filedata(part, FILENAME);
        
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
}

int main() {
    // Upload the file
    upload_file();
    return 0;
}