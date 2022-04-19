#pragma once

// ESP32 vendor libraries
#include <Preferences.h>

static const char cameraNamespace[] = "camera";

class CameraPreferences {
  public:
    explicit CameraPreferences(Preferences &preferences) : preferences(preferences) {}
    bool isFirstRun();
    bool isTimelapse();
    void setIsTimelapse(bool value);
    uint32_t getFrameIndex();
    void setFrameIndex(uint32_t value);

  private:
    Preferences &preferences;
};
