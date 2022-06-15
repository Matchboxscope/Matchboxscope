// UPLOAD INSTRUCTIONS

// Install v2.0.2 of ESP32 board support for the Arduino. Choose the "ESP32 Dev Module" board.
// Enable the PSRAM settings. Upload the sketch.
// Then perform the ESP32 Data Sketch Upload to upload web server data assets to the ESP32.


// HEADERS

// C++ standard libraries
#include <memory>

// ESP32 vendor libraries
#include <soc/soc.h>           // Disable brownour problems
#include <soc/rtc_cntl_reg.h>  // Disable brownour problems
#include <esp_camera.h>
#include <Preferences.h>
#include <SPIFFS.h>
#include <SD_MMC.h> // SD card

// Arduino core
#include "Arduino.h"

// Arduino third-party libraries
#include <esp32cam.h>

// Local header files
#include "device_pref.h"
#include "illumination.h"
#include "camera.h"
#include "webserver.h"
#include "wifi.h"


// TODOS

// TODO: Whitebalance/constant intensity
// TODO: Swap esp32cam.h with esp_camera?
// TODO: link /enable to website
// TODO: fit mjpeg into website proerly


// SETTINGS

// Startup
static const uint32_t refocus_button_debounce = 4000; // ms; duration to wait for the refocus button to stabilize before reading

// Hardware pins
static const gpio_num_t reedSwitchPin = GPIO_NUM_13; // pin of the external switch to "awake" the ESP (e.g. Reedrelay)
static const gpio_num_t lightPin = GPIO_NUM_4; // pin of the imaging illumination LED

// Image acquisition
static const int res_warmup_wait = 500; // ms; how long to wait after changing the camera resolution
static const uint32_t camera_warmup_frames = 5; // number of "warm-up" camera frames to wait before final acquisition
static const int num_frame_buffers = 2; // number of frame buffers to keep, must be at least 1; higher values enable higher streaming fps
static const int jpeg_quality = 80; // JPEG quality factor from 0 (worst) to 100 (best)

// Timelapse
static const uint64_t timelapseInterval = 60; // sec; timelapse interval

// Wifi
static const bool hostWifiAP = true; // whether to host a wifi AP or to join an existing network
const char *wifiAPSSID = "MatchboxScope"; 
const char *wifiSSID = "YOUR_SSID"; // the SSID of a wifi network to join, if not hosting a wifi AP
const char *wifiPassword = "YOUR_PASSWORD"; // the password of a wifi network to join, if not hosting a wifi AP

boolean sdInitialized = false;

// GLOBAL STATE

// Mode
bool is_timelapse = false; // in timelapse mode, the webserver is not enabled

// Hardware
Light light(lightPin);
Camera camera(
    esp32cam::Camera, esp32cam::pins::AiThinker, light,
    res_warmup_wait, num_frame_buffers, jpeg_quality
);

// Preferences
Preferences pref;
DevicePreferences device_pref(pref, "camera", __DATE__ " " __TIME__);

// Web server
void setTimelapse() {
  is_timelapse = true; // focus has been set
  device_pref.setIsTimelapse(is_timelapse);
}
RefocusingServer server(80, SPIFFS, camera, setTimelapse);


// INITIALIZATION
void initNonTimelapseFunctionalities() {
  if (hostWifiAP) {
    initWifiAP(wifiAPSSID);
  } else {
    joinWifi(wifiSSID, wifiPassword);
  }
  if (!SPIFFS.begin()) { // SPIFFS must be initialized before the web server, which depends on it
    Serial.println("Couldn't open SPIFFS!");
  }
  server.init();
}

void setup() {
  Serial.begin(115200);

  // Reset the EEPROM's stored timelapse mode after each re-flash
  if (device_pref.isFirstRun()) {
     device_pref.setIsTimelapse(false);
  }



  // Initialize the camera
  if (!camera.init()) {
    ESP.restart();
    return;
  }

  // 1-bit mode as suggested here:https://dr-mntn.net/2021/02/using-the-sd-card-in-1-bit-mode-on-the-esp32-cam-from-ai-thinker
  if (!SD_MMC.begin("/sdcard", true)) {
    Serial.println("SD Card Mount Failed");
  }
  else {
    Serial.println("SD Card Mounted");
  }
  sdInitialized = true;


  // If the button is pressed, switch from timelapse mode back to refocusing mode
  pinMode(reedSwitchPin, INPUT_PULLUP);
  Serial.println("Press Button if you want to refocus (t..4s)");
  delay(refocus_button_debounce);
  bool buttonPressed = !digitalRead(reedSwitchPin); // pull up
  Serial.print("Button pressed? ");
  if (buttonPressed) {
    Serial.println("yes. Switching from timelapse mode to refocusing mode.");
    device_pref.setIsTimelapse(false);
  } else {
    Serial.println("no");
  }

  // Initialize the remaining hardware, depending on the mode
  is_timelapse = device_pref.isTimelapse(); // set the global variable for the loop function
  if (is_timelapse) {
    Serial.println("In timelapse mode.");
    return;
  }
  Serial.println("In refocusing mode. Connect to Wifi and go to 192.168.4.1/enable once you're done with focusing.");
  initNonTimelapseFunctionalities();
}


// MAIN LOOP

bool saveImage(std::unique_ptr<esp32cam::Frame> frame, String filename) {
  if (frame == nullptr) {
    return false;
  }


  return Camera::save(std::move(frame), filename.c_str(), SD_MMC);
}

void loop() {
  if (!is_timelapse) {
    server.serve(); // serve webpage and image stream to adjust focus
    return;
  }

  // Acquire the image
  camera.useMaxRes();
  auto frame = camera.acquire(camera_warmup_frames);

  // Save image to SD card
  uint32_t frame_index = device_pref.getFrameIndex() + 1;
  if (saveImage(std::move(frame), "/picture" + String(frame_index) + ".jpg")) {
    device_pref.setFrameIndex(frame_index);
  };

  // Sleep
  light.sleep();
  Serial.print("Sleeping for ");
  Serial.print(timelapseInterval);
  Serial.println(" s");
  static const uint64_t usPerSec = 1000000; // Conversion factor from microseconds to seconds
  esp_sleep_enable_timer_wakeup(timelapseInterval * usPerSec);
  esp_deep_sleep_start();
}
