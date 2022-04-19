#pragma once

// Arduino core
#include "Arduino.h"

class Light {
  public:
    explicit Light(int pin) : pin(pin) {}
    void enable();
    void disable();

  private:
    int pin;
};

void Light::enable() {
  pinMode(pin, OUTPUT);
  digitalWrite(pin, HIGH);
}

void Light::disable() {
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
}
