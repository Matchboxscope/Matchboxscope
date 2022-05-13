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
#include <driver/rtc_io.h>

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
static const uint64_t timelapseInterval = 5; // sec; timelapse interval
static uint64_t t_old = 0;


// Wifi
const char *wifiAPSSID = "MatchboxScope";
const char *wifiSSID = "YOUR_SSID"; // the SSID of a wifi network to join, if not hosting a wifi AP
const char *wifiPassword = "YOUR_PASSWORD"; // the password of a wifi network to join, if not hosting a wifi AP


// GLOBAL STATE
boolean hostWifiAP = true;
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

  Serial.println("In refocusing mode. Connect to Wifi and go to 192.168.4.1/enable once you're done with focusing.");
  initNonTimelapseFunctionalities();


  // take initial frame to get led happy?
  auto frame = camera.acquire(1);
  saveImage(std::move(frame), "/background.jpg");
  light.on();
  frame = camera.acquire(1);
  saveImage(std::move(frame), "/foreground.jpg"); 
  
  // initiliaze timer
  t_old = millis();

}

bool sdInitialized = false;

// MAIN LOOP

bool saveImage(std::unique_ptr<esp32cam::Frame> frame, String filename) {
  if (frame == nullptr) {
    return false;
  }

  // We initialize SD_MMC here rather than in setup() because SD_MMC needs to reset the light pin
  // with a different pin mode.
  if (not sdInitialized) {
    if (!SD_MMC.begin()) {
      Serial.println("SD Card Mount Failed");
      return false;
    }
    sdInitialized = true;
  }

  return Camera::save(std::move(frame), filename.c_str(), SD_MMC);


}


void loop() {

  server.serve(); // serve webpage and image stream to adjust focus

  if ((millis() - t_old) > (1000 * timelapseInterval)) {
    //https://stackoverflow.com/questions/67090640/errors-while-interacting-with-microsd-card
    t_old = millis();
    // Acquire the image
    camera.useMaxRes();
    auto frame = camera.acquire(camera_warmup_frames);

    // Save image to SD card
    uint32_t frame_index = device_pref.getFrameIndex() + 1;
    light.sleep();
    
    if (saveImage(std::move(frame), "/picture" + String(frame_index) + ".jpg")) {
      device_pref.setFrameIndex(frame_index);
    };
  }
}
