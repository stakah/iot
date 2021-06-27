#include <Arduino.h>
#include "WiFi.h"
#include "util.h"

const char* ssid = "TP-Link_5033";
const char* password =  "Takahas31777049";

// Built-in LED pin for ESP32
const int LED_BUILTIN = 2;

void setup() {
  // Serial setup to 115200 bauds
  Serial.begin(115200);

  // Wifi setup
  WiFi.begin(ssid, password);

  // Setting LED pin to OUTPUT
  pinMode(LED_BUILTIN, OUTPUT);

  // Start with LED on
  digitalWrite(LED_BUILTIN, HIGH);

  Serial.printf("Connecting to WiFi (SSID:%s)", ssid);
  while (WiFi.status() != WL_CONNECTED) {
    // Blink LED 4 times
    for (int i=0; i<4; i++) {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(50);
      digitalWrite(LED_BUILTIN, LOW);
      delay(100);
    }
    delay(500);
    // Send a dot to Serial monitor
    Serial.print(".");
  }

  // If connected, this message will be sent 
  // and the LED will remain on.
  Serial.printf("\nConnected to the Wifi network:\n- SSID = %s", ssid);
  digitalWrite(LED_BUILTIN, HIGH);

  // Print the device IP address
  IPAddress localIP = WiFi.localIP();
  char *c_localIP = stringToCharArray(localIP.toString());
  Serial.printf("\n- IP Addr: %s", c_localIP);

  // Print the device MAC address
  char *c_macAddr = stringToCharArray(WiFi.macAddress());
  Serial.printf("\n- MAC Addr: %s", c_macAddr);

}

void loop() {
  // put your main code here, to run repeatedly:
}