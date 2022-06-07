#pragma once

// ESP32 vendor libraries
#include <WiFi.h>

// Arduino core
#include "Arduino.h"

void initWifiAP(const char *ssid) {
  Serial.print("Network SSID: ");
  Serial.println(ssid);

  WiFi.softAP(ssid);
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());
}

void joinWifi(const char *ssid, const char *password) {
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
    // we can even make the ESP32 to sleep
  }

  Serial.print("Connected. IP: ");
  Serial.println(WiFi.localIP());
}
