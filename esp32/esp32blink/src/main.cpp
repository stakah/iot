#include <Arduino.h>

int LED_BUILTIN = 2;

void setup()
{
  // initialize LED digital pin as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  // Setup serial
  Serial.begin(115200);
}

void loop()
{
  // turn the LED on (HIGH is the voltage level)
  digitalWrite(LED_BUILTIN, HIGH);

  // Sending serial message
  Serial.println("LED is on ...");

  // wait for a second
  delay(1000);

  // turn the LED off by making the voltage LOW
  digitalWrite(LED_BUILTIN, LOW);

  // Sending serial message
  Serial.println("LED is off ...");

   // wait for a second
  delay(1000);
}
