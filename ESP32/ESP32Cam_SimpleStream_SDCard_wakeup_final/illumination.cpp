#include "illumination.h"

void Light::on() {
  pinMode(pin, OUTPUT);
  digitalWrite(pin, HIGH);
}

void Light::off() {
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
}
