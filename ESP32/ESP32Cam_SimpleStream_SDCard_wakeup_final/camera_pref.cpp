#include "camera_pref.h"
#include "Arduino.h"

const char cameraNamespace[] = "camera";

// First-run detection

const char dateKey[] = "date";
String compiled_date(__DATE__ " " __TIME__);

bool CameraPreferences::isFirstRun() {
  preferences.begin(cameraNamespace, false);
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

const char timelapseKey[] = "is_timelapse";

bool CameraPreferences::isTimelapse() {
  preferences.begin(cameraNamespace, true);
  bool value = preferences.getBool(timelapseKey, false);
  preferences.end();
  return value;
}

void CameraPreferences::setIsTimelapse(bool value) {
  preferences.begin(cameraNamespace, false);
  preferences.putBool(timelapseKey, value);
  preferences.end();
}

// Timelapse frame indexing

const char frameIndexKey[] = "frame_index";

uint32_t CameraPreferences::getFrameIndex() {
  preferences.begin(cameraNamespace, true);
  uint32_t value = preferences.getUInt(frameIndexKey, 0);
  preferences.end();
  return value;
}

void CameraPreferences::setFrameIndex(uint32_t value) {
  preferences.begin(cameraNamespace, false);
  preferences.putUInt(frameIndexKey, value);
  preferences.end();
}
