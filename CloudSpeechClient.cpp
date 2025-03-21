#include <WiFi.h>
#include "CloudSpeechClient.h"
#include "network_param.h"
#include <base64.h>
#include <ArduinoJson.h>
CloudSpeechClient::CloudSpeechClient(Authentication authentication) {
  this->authentication = authentication;
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(1000);
  client.setCACert(root_ca);
  if (!client.connect(server, 443)) {
    Serial.println("❌ ERROR: No se pudo conectar a Google API");
    return;
  }
  Serial.println("✅ Conectado a Google API");
}

String ans;

CloudSpeechClient::~CloudSpeechClient() {
  client.stop();
  WiFi.disconnect();
}

void CloudSpeechClient::PrintHttpBody2(Audio* audio) {
  String enc = base64::encode(audio->paddedHeader, sizeof(audio->paddedHeader));
  enc.replace("\n", "");
  Serial.println("Base64 Header: " + enc.substring(0, 100));  // Solo muestra los primeros 100 caracteres
  client.print(enc);

  char** wavData = audio->wavData;
  for (int j = 0; j < audio->wavDataSize / audio->dividedWavDataSize; ++j) {
    enc = base64::encode((byte*)wavData[j], audio->dividedWavDataSize);
    enc.replace("\n", "");
    Serial.println("Base64 Chunk " + String(j) + ": " + enc.substring(0, 100));  // Solo muestra parte del Base64
    client.print(enc);
  }
}


void CloudSpeechClient::Transcribe(Audio* audio) {
    Serial.println("✅ Enviando solicitud a Google Speech-to-Text...");
    String HttpBody1 = "{\"config\":{\"encoding\":\"LINEAR16\",\"sampleRateHertz\":16000,\"languageCode\":\"es-ES\"},\"audio\":{\"content\":\"";
    String HttpBody3 = "\"}}\r\n\r\n";

    int httpBody2Length = (audio->wavDataSize + sizeof(audio->paddedHeader)) * 4 / 3;
    String ContentLength = String(HttpBody1.length() + httpBody2Length + HttpBody3.length());

    String HttpHeader = "POST /v1/speech:recognize?key=" + ApiKey +
                        " HTTP/1.1\r\nHost: speech.googleapis.com\r\nContent-Type: application/json\r\nContent-Length: " + ContentLength + "\r\n\r\n";

    Serial.println("🔵 Enviando encabezado HTTP...");
    client.print(HttpHeader);
    Serial.println("🔵 Enviando cuerpo HTTP...");
    client.print(HttpBody1);
    PrintHttpBody2(audio);
    client.print(HttpBody3);

    Serial.println("✅ Esperando respuesta de Google...");
    
    unsigned long timeout = millis() + 20000;  // Esperar 10 segundos máximo
    while (!client.available()) {
        if (millis() > timeout) {
            Serial.println("❌ ERROR: Google no respondió en el tiempo esperado.");
            return;
        }
    }

    Serial.println("✅ Google ha respondido. Leyendo datos...");
    String My_Answer = "";
    
    while (client.available()) {
        char temp = client.read();
        My_Answer += temp;
    }

    Serial.println("📌 Respuesta completa:");
    Serial.println(My_Answer);

    int position = My_Answer.indexOf('{');
    if (position != -1) {
        ans = My_Answer.substring(position);
        Serial.println("✅ JSON recibido:");
        Serial.println(ans);
    } else {
        Serial.println("❌ ERROR: No se recibió JSON válido.");
    }
}
