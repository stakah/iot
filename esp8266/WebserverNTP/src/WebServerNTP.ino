/*
   Copyright (c) 2015, Majenko Technologies
   All rights reserved.

   Redistribution and use in source and binary forms, with or without modification,
   are permitted provided that the following conditions are met:

 * * Redistributions of source code must retain the above copyright notice, this
     list of conditions and the following disclaimer.

 * * Redistributions in binary form must reproduce the above copyright notice, this
     list of conditions and the following disclaimer in the documentation and/or
     other materials provided with the distribution.

 * * Neither the name of Majenko Technologies nor the names of its
     contributors may be used to endorse or promote products derived from
     this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
   ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
   ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
   ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*
 * https://github.com/arduino-libraries/NTPClient/issues/149
 * 
 * Incorrect date/year with ESP8266 board 3.0.0
 * ============================================
 * 
 * Tools -> Board "..." -> Board Manager ...
 * 
 * Search for: esp8266
 * Choose version: 2.6.3 to install
 * Versions 3.0.0 and above have the bug that returns wrong date
 */
  
  
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESP8266mDNS.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#include <LittleFS.h>

// #include <Servo.h>

#include "util.h"
struct TimeUtil *timeUtil;
Util *util;
long epochTime = 0;


#ifndef STASSID
#define STASSID "ST2"
#define STAPSK  "Takahas31777049"
#endif
const char *ssid = STASSID;
const char *password = STAPSK;


#define DEBUG_NTPClient
#define DEBUG_UPDATE_INTERVAL 60000
unsigned long debugUpdate = 0;
bool debug = false;

// nodemu only
// #define SERVO_PIN D6

// Servo servo;
// int pos;

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

AsyncWebServer server(80);

const int led = LED_BUILTIN;
bool ledState = false;

#define setLEDState(state) ledState = state;

void blinkLED(unsigned long onTime) {
  /*
  | #| prevState | ledState | delay | LED | prevState' | ledState' | delay' | LED' |
  |--|-----------|----------|-------|-----|------------|-----------|--------|------|
  | 1| false     | false    |   -   |  -  | false      | false     |   -    | OFF  |
  | 2| false     | true     |   -   |  -  | true       | true      | mill   | ON   |
  | 3| true      | true     |  <T   |  -  | true       | true      | <T     | ON   |
  | 4| true      | true     |  >T   |  -  | true       | false     |   0    | OFF  |
  | 5| true      | false    |   -   |  -  | false      | false     |   0    | OFF  |
  */
  static unsigned long delay = millis();
  static bool prevState = false;
  int val = HIGH;
  if (onTime < 10) onTime = 10;

  if (prevState == false) {
    if (ledState == false) {
      // [1] -> [1]
      val = HIGH;
    } else {
      // [2] -> [3]
      prevState = true;
      delay = millis();
      val = LOW;
    }
  } else {
    val = LOW;
    if (ledState == false) {
      // [5] -> [1]
      prevState = false;
      delay = 0;
      val = HIGH;
    } else if (millis() - delay > onTime) {
      // [4] -> [5]
      prevState = false;
      ledState = false;
      delay = 0;
      val = HIGH;
    }
  }

  digitalWrite(LED_BUILTIN, val);

}


void updateDebug() {
    if (millis() - debugUpdate > DEBUG_UPDATE_INTERVAL) {
    debugUpdate = millis();
    debug = true;
  }
}

bool syncNTPTime() {
  bool ret = true;

  setLEDState(true);
  if (!timeClient.update()) {
    if (debug) Serial.println("NTP update time out");
    ret = false;
  } else {
    if (debug) Serial.println("NTP updated");
  }

  epochTime = timeClient.getEpochTime();

  return ret;
}

void incLocalTime() {
  setLEDState(true);

  static unsigned long lastSecond = millis();
  long interval = millis() - lastSecond;
  if (interval > 1000) {
    lastSecond = millis();
    epochTime += interval / 1000;
  }
}

String getData() {
  char temp[1000];
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;

  util->currentDateTime(epochTime, timeUtil);
  time_t localCTime = mktime(timeUtil->ptm);
  char* strCTime = strtok(ctime(&localCTime), "\n"); // strtok: removes \n at the end of ctime. Causes erro when parsing JSON.

  snprintf(temp, 1000,
           "{\n"
           "   \"success\": true,\n"
           "   \"data\": {\n"
           "     \"uptime\": \"%02d:%02d:%02d\",\n"
           "     \"epochTime\" : \"%d\",\n"
           "     \"formattedTime\" : \"%s\",\n"
           "     \"currentHour\" : \"%02d\",\n"
           "     \"currentMinute\" : \"%02d\",\n"
           "     \"currentSecond\" : \"%02d\",\n"
           "     \"weekDay\" : \"%s\",\n"
           "     \"monthDay\" : \"%02d\",\n"
           "     \"currentMonth\" : \"%02d\",\n"
           "     \"currentMonthName\" : \"%s\",\n"
           "     \"currentYear\" : \"%d\",\n"
           "     \"currentDate\" : \"%s\",\n"
           "     \"cTime\" : \"strlen=%d [%s]\"\n"
           "   }\n"
           " }",
           hr, min % 60, sec % 60,
           epochTime, 
           timeUtil->formattedTime.c_str(), 
           timeUtil->currentHour, 
           timeUtil->currentMinute, 
           timeUtil->currentSecond,
           timeUtil->weekDay.c_str(), 
           timeUtil->currentMonthDay, 
           timeUtil->currentMonth, 
           timeUtil->currentMonthName.c_str(), 
           timeUtil->currentYear, 
           timeUtil->formattedDate.c_str(),
           strlen(strCTime), strCTime
          );

  if (debug) {
    Serial.print("cTime: ");
    Serial.println(strCTime);
  }
  
  return temp;
}

int graphY[39];
void updateGraph() {
  static unsigned long period = millis();

  if ((millis() - period) > 1000) {
    period = millis();
    for (int i=1; i<39; i++) {
      graphY[i-1] = graphY[i];
    }
    graphY[38] = rand() % 130;
  }
}

String drawGraph() {
  String out;
  out.reserve(2600);
  char temp[70];
  out += "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" width=\"400\" height=\"150\">\n";
  out += "<rect width=\"400\" height=\"150\" fill=\"rgb(250, 230, 210)\" stroke-width=\"1\" stroke=\"rgb(0, 0, 0)\" />\n";
  out += "<g stroke=\"black\">\n";

  int x0 = 0;
  int y0 = graphY[0];
  for (int i = 1; i < 39; i++) {
    int x1 = i*10;
    int y1 = graphY[i];
    sprintf(temp, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" stroke-width=\"1\" />\n", x0, y0, x1, y1);
    x0 = x1;
    y0 = y1;
    out += temp;
  }
  out += "</g>\n</svg>\n";

  return out;
}

void handleNotFound2(AsyncWebServerRequest *request) {
  setLEDState(true);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += request->url();
  message += "\nMethod: ";
  message += (request->method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += request->args();
  message += "\n";

  for (uint8_t i = 0; i < request->args(); i++) {
    message += " " + request->argName(i) + ": " + request->arg(i) + "\n";
  }

  request->send(404, "text/plain", message);
  
}

void setup(void) {
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }


  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/www/index.html", "text/html");
  });
  server.on("/test.svg", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "image/svg+xml", drawGraph());
  });
  server.on("/data", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "application/json", getData());
  });
  server.onNotFound(handleNotFound2);

  server.begin();
  Serial.println("HTTP server started");

  // Initialize a NTPClient to get time
  timeClient.begin();
  // Set offset time in seconds to adjust for your timezone, for example:
  // GMT +1 = 3600
  // GMT +8 = 28800
  // GMT -1 = -3600
  // GMT 0 = 0
  timeClient.setTimeOffset(-3*3600);
  
  timeUtil = new TimeUtil();
  util = new Util();

  syncNTPTime();

  if (!LittleFS.begin()) {
    Serial.println("[ERR] An Error has occured while mounting LittleFS");
    return;
  }

  File file = LittleFS.open("/text.txt", "r");
  if (!file) {
    Serial.println("[ERR] Failed to open file '/text.txt' for reading");
    return;
  }

  Serial.println("File Content:");
  while (file.available()) {
    Serial.write(file.read());
  }
  file.close();

  // Servo
  // servo.attach(SERVO_PIN);
  // servo.write(0);
}

void incServo(int second){
  /*
  s | p
  0 | 0
  1 | 6
  29 | 174
  30 | 180
  31 | 174
  60 | 0
  */
  // if (second < 31)
  //   pos = second * 6;
  // else
  //   pos = (60 - second) * 6;

  // servo.write(pos);
  // Serial.printf("servo pos=%03d\n", pos);
}

void updateTime() {
  static unsigned long period = millis();

  if ((millis() - period) > 1000) {
    period = millis();
    if (!syncNTPTime()) 
      incLocalTime();

    // util->currentDateTime(epochTime, timeUtil);
    // incServo(timeUtil->currentSecond);
  }
  

  if (debug) {
    util->currentDateTime(epochTime, timeUtil);
    Serial.println("Epoch Time | Formatted Time | HH:mm:ss | Week day  | Month day | Month / name     | Year | Current date");
    Serial.printf("%d |       %8s | %02d:%02d:%02d | %9s |        %2d |    %2d /%9s | %4d | %s\n",
      epochTime
      ,timeUtil->formattedTime.c_str()
      ,timeUtil->currentHour
      ,timeUtil->currentMinute
      ,timeUtil->currentSecond
      ,timeUtil->weekDay.c_str()
      ,timeUtil->currentMonthDay
      ,timeUtil->currentMonth
      ,timeUtil->currentMonthName.c_str()
      ,timeUtil->currentYear
      ,timeUtil->formattedDate.c_str()
      );
  }
}

void loop(void) {
  blinkLED(0);
  debug = false; // Resets debug flag
  updateDebug();
  MDNS.update();
  updateTime();
  updateGraph();
}
