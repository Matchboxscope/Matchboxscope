#pragma once

// ESP32 vendor libraries
#include <Preferences.h>

class CameraPreferences {
  public:
    explicit CameraPreferences(Preferences &preferences, const char *group_name) :
      preferences(preferences), group_name(group_name) {}
    bool isFirstRun();
    bool isTimelapse();
    void setIsTimelapse(bool value);
    uint32_t getFrameIndex();
    void setFrameIndex(uint32_t value);

  private:
    Preferences &preferences;
    const char *group_name;
};
