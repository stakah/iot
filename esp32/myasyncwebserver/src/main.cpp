#include <Arduino.h>
#include "WiFi.h"
#include "ESPAsyncWebServer.h"

const char* ssid = "TP-Link_5033";
const char* password =  "Takahas31777049";
  AsyncWebServer server(80);

void setup() {
  // Serial setup to 115200 bauds
  Serial.begin(115200);

  // Wifi setup
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
 
  Serial.println(WiFi.localIP());

  server.on("/hello", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", "Hello World");
  });

  server.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
}