#pragma once

// Arduino core
#include "Arduino.h"

class Light {
  public:
    explicit Light(gpio_num_t pin) : pin(pin) {}
    void on();
    void off();
    void sleep();

  private:
    gpio_num_t pin;
};
