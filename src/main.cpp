#include <Arduino.h>

const int pin = LED_BUILTIN;

void setup() {
  pinMode(pin, OUTPUT);
}

void loop() {
  digitalWrite(pin, false);
  delay(500);
  digitalWrite(pin, true);
  delay(1500);
}