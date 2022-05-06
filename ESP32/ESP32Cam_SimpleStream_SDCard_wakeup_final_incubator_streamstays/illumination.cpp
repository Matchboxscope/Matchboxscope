#include "illumination.h"

// ESP32 vendor libraries
#include <driver/rtc_io.h>

void Light::on() {
  pinMode(pin, OUTPUT);
  digitalWrite(pin, HIGH);
}

void Light::off() {
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
}

void Light::sleep() {
  off();
  rtc_gpio_hold_en(pin);
}
