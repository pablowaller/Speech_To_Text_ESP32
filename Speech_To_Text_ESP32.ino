#include "Audio.h"
#include "CloudSpeechClient.h"

void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("\r\nRecord start!\r\n");
    Audio* audio = new Audio(INMP441);
    audio->Record();
    Serial.println("Recording Completed. Now Processing...");
    CloudSpeechClient* cloudSpeechClient = new CloudSpeechClient(USE_APIKEY);
    cloudSpeechClient->Transcribe(audio);
    delete cloudSpeechClient;
    delete audio;
}

void loop() {
}