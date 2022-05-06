#include "device_pref.h"

// Arduino core
#include "Arduino.h"

// First-run detection

static const char dateKey[] = "date";

bool DevicePreferences::isFirstRun() {
  preferences.begin(group_name, false);
  String stored_date = preferences.getString(dateKey, "");  // FIXME
  preferences.putString(dateKey, compiled_date); // FIXME?
  preferences.end();

  // Serial.println("Stored date:");
  // Serial.println(stored_date);
  // Serial.println("Compiled date:");
  // Serial.println(compiled_date);

  Serial.print("First run? ");
  if (!stored_date.equals(compiled_date)) {
    Serial.println("yes");
  } else {
    Serial.println("no");
  }

  return !stored_date.equals(compiled_date);
}

// Timelapse mode

static const char timelapseKey[] = "is_timelapse";

bool DevicePreferences::isTimelapse() {
  preferences.begin(group_name, true);
  bool value = preferences.getBool(timelapseKey, false);
  preferences.end();
  return value;
}

void DevicePreferences::setIsTimelapse(bool value) {
  preferences.begin(group_name, false);
  preferences.putBool(timelapseKey, value);
  preferences.end();
}

// Timelapse frame indexing

static const char frameIndexKey[] = "frame_index";

uint32_t DevicePreferences::getFrameIndex() {
  preferences.begin(group_name, true);
  uint32_t value = preferences.getUInt(frameIndexKey, 0);
  preferences.end();
  return value;
}

void DevicePreferences::setFrameIndex(uint32_t value) {
  preferences.begin(group_name, false);
  preferences.putUInt(frameIndexKey, value);
  preferences.end();
}
