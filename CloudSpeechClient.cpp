#include <WiFi.h>
#include "CloudSpeechClient.h"
#include "network_param.h"
#include <base64.h>
#include <ArduinoJson.h>

CloudSpeechClient::CloudSpeechClient(Authentication authentication) {
  this->authentication = authentication;

  // Conectar a Wi-Fi
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Waiting for WiFi connection...");
  }
  Serial.println("Connected to WiFi");

  // Configurar el certificado raíz
  client.setCACert(root_ca);

  // Conectar al servidor de Google Cloud
  Serial.println("Connecting to Google Cloud...");
  if (!client.connect(server, 443)) {
    Serial.println("Connection to Google Cloud failed!");
  } else {
    Serial.println("Connected to Google Cloud");
  }
}

String ans;

CloudSpeechClient::~CloudSpeechClient() {
  client.stop();
  WiFi.disconnect();
}

void CloudSpeechClient::PrintHttpBody2(Audio* audio) {
  String enc = base64::encode(audio->paddedHeader, sizeof(audio->paddedHeader));
  enc.replace("\n", "");  // delete last "\n"
  client.print(enc);      // HttpBody2
  char** wavData = audio->wavData;
  for (int j = 0; j < audio->wavDataSize / audio->dividedWavDataSize; ++j) {
    enc = base64::encode((byte*)wavData[j], audio->dividedWavDataSize);
    enc.replace("\n", "");  // delete last "\n"
    Serial.println("Encoded audio chunk: " + enc);
    client.print(enc);      // HttpBody2
  }
}

void CloudSpeechClient::Transcribe(Audio* audio) {
  String HttpBody1 = "{\"config\":{\"encoding\":\"LINEAR16\",\"sampleRateHertz\":16000,\"languageCode\":\"es-ES\"},\"audio\":{\"content\":\"";
  String HttpBody3 = "\"}}\r\n\r\n";
  int httpBody2Length = (audio->wavDataSize + sizeof(audio->paddedHeader)) * 4 / 3;  // 4/3 is from base64 encoding
  String ContentLength = String(HttpBody1.length() + httpBody2Length + HttpBody3.length());
  String HttpHeader;
  // if (authentication == USE_APIKEY)
  HttpHeader = String("POST /v1/speech:recognize?key=") + ApiKey
               + String(" HTTP/1.1\r\nHost: speech.googleapis.com\r\nContent-Type: application/json\r\nContent-Length: ") + ContentLength + String("\r\n\r\n");
  //  else if (authentication == USE_ACCESSTOKEN)
  //    HttpHeader = String("POST /v1beta1/speech:syncrecognize HTTP/1.1\r\nHost: speech.googleapis.com\r\nContent-Type: application/json\r\nAuthorization: Bearer ")
  //   + AccessToken + String("\r\nContent-Length: ") + ContentLength + String("\r\n\r\n");
  client.print(HttpHeader);
  client.print(HttpBody1);
  PrintHttpBody2(audio);
  client.print(HttpBody3);
  String My_Answer = "";
  while (!client.available())
    ;

  while (client.available()) {
    char temp = client.read();
    My_Answer = My_Answer + temp;
    // Serial.write(client.read());

    Serial.println("Waiting for response...");
    while (!client.available())
      ;  // Esperar hasta que haya datos disponibles

    Serial.println("Response from Google Cloud:");
    String responseBody = "";
    while (client.connected() || client.available()) {
      if (client.available()) {
        String line = client.readStringUntil('\n');  // Leer línea por línea
        responseBody += line;                        // Acumular la respuesta
        Serial.println(line);                        // Imprimir la línea
      }
    }

    // Procesar la respuesta JSON
    Serial.println("Full response body:");
    Serial.println(responseBody);

    // Parsear el JSON
    DynamicJsonDocument doc(1024);  // Ajusta el tamaño según la respuesta
    DeserializationError error = deserializeJson(doc, responseBody);

    if (error) {
      Serial.println("Failed to parse JSON!");
      Serial.println(error.c_str());
      return;
    }

    // Extraer la transcripción
    const char* transcript = doc["results"][0]["alternatives"][0]["transcript"];
    float confidence = doc["results"][0]["alternatives"][0]["confidence"];

    Serial.println("Transcription: " + String(transcript));
    Serial.println("Confidence: " + String(confidence));
  }

  Serial.print("My Answer - ");
  Serial.println(My_Answer);
  int postion = My_Answer.indexOf('{');
  Serial.println(postion);
  ans = My_Answer.substring(postion);
  Serial.print("Json daata--");
  Serial.print(ans);
}