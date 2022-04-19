#pragma once

// Arduino core
#include "Arduino.h"

class Light {
  public:
    explicit Light(int pin) : pin(pin) {}
    void on();
    void off();

  private:
    int pin;
};
