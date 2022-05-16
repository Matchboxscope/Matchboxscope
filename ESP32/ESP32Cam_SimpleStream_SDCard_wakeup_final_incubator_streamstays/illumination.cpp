#include "illumination.h"

// ESP32 vendor libraries
#include <driver/rtc_io.h>

void Light::on() {
  //pinMode(pin, OUTPUT);; // does not work if we (re-?)declare the pinmode
  digitalWrite(pin, HIGH);
  //gpio_reset_pin(pin);
}

void Light::off() {
  //pinMode(pin, OUTPUT); // does not work if we (re-?)declare the pinmode
  digitalWrite(pin, LOW);
  //gpio_reset_pin(pin);
}

void Light::sleep() {
  off();
  // not sure if this works at all
  //rtc_gpio_hold_en(pin);
}
