// UPLOAD INSTRUCTIONS

// Install v2.0.2 of ESP32 board support for the Arduino. Choose the "ESP32 Dev Module" board.
// Enable the PSRAM settings. Upload the sketch.
// Then perform the ESP32 Data Sketch Upload to upload web server data assets to the ESP32.


// HEADERS

// C++ standard libraries
#include <memory>

// ESP32 vendor libraries
#include <Preferences.h>
#include <SPIFFS.h>
#include <FS.h>
#include "SD_MMC.h" // SD card
#include "soc/soc.h"           // Disable brownour problems
#include "soc/rtc_cntl_reg.h"  // Disable brownour problems
#include "driver/rtc_io.h"
#include "esp_camera.h"

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

// Hardware pins
static const int reedSwitchPin = 13; // pin of the external switch to "awake" the ESP (e.g. Reedrelay)
static const int lightPin = 4; // pin of the imaging illumination LED

// Sleep
#define uS_TO_S_FACTOR 1000000  // Conversion factor for micro seconds to seconds 
#define TIME_TO_SLEEP  10       // Time ESP32 will go to sleep (in seconds) 

// Image acquisition
static const int res_warmup_wait = 500; // ms; how long to wait after changing the camera resolution
static const uint32_t camera_warmup_frames = 5; // number of "warm-up" camera frames to wait before final acquisition
static const int num_frame_buffers = 2; // number of frame buffers to keep, must be at least 1; higher values enable higher streaming fps
static const int jpeg_quality = 80; // JPEG quality factor from 0 (worst) to 100 (best)

// Wifi
static const bool hostWifiAP = true; // whether to host a wifi AP or to join an existing network
const char *wifiAPSSID = "MatchboxScope"; 
const char *wifiSSID = "YOUR_SSID"; // the SSID of a wifi network to join, if not hosting a wifi AP
const char *wifiPassword = "YOUR_PASSWORD"; // the password of a wifi network to join, if not hosting a wifi AP


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
  if (!SPIFFS.begin()) {
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

  // If the button is pressed, switch from timelapse mode back to refocusing mode
  pinMode(reedSwitchPin, INPUT_PULLUP);
  Serial.println("Press Button if you want to refocus (t..4s)");
  delay(4000);
  bool buttonPressed = !digitalRead(reedSwitchPin); // pull up
  Serial.print("Button pressed? ");
  if (buttonPressed) {
    Serial.println("yes. Switching from timelapse mode to refocusing mode.");
    device_pref.setIsTimelapse(false);
  } else {
    Serial.println("no");
  }

  if (!camera.init()) {
    ESP.restart();
  }

  is_timelapse = device_pref.isTimelapse(); // set the global variable for the loop function
  if (is_timelapse) {
    Serial.println("In timelapse mode.");
    return;
  }

  Serial.println("In refocusing mode. Connect to Wifi and go to 192.168.4.1/enable once you're done with focusing.");
  initNonTimelapseFunctionalities();
}


// MAIN LOOP

bool saveImage(std::unique_ptr<esp32cam::Frame> frame, String filename, FS &fs) {
  Serial.printf("Saving to: %s\n", filename.c_str());
  File file = fs.open(filename.c_str(), FILE_WRITE);
  Serial.println("Opening file");
  if (!file) {
    Serial.println("Failed to open file in writing mode");
    file.close();
    return false;
  }

  file.write(frame->data(), frame->size()); // payload (image), payload length
  Serial.println("Finished writing file.");
  file.close();
  return true;
}

void loop() {
  if (!is_timelapse) {
    server.serve(); // serve webpage and image stream to adjust focus
    return;
  }

  // Acquire the image
  if (!camera.useMaxRes()) {
    Serial.println("SET-HI-RES FAIL");
  }
  auto frame = camera.acquire(camera_warmup_frames);
  if (frame == nullptr) {
    Serial.println("CAPTURE FAIL");
    return;
  }
  Serial.printf(
      "CAPTURE OK %dx%d %db\n",
      frame->getWidth(), frame->getHeight(), static_cast<int>(frame->size())
  );

  // Save image to SD card
  if (!SD_MMC.begin()) { // initialized here rather than setup() because it resets the light pin with a different pin mode
    Serial.println("SD Card Mount Failed");
  }
  uint32_t frame_index = device_pref.getFrameIndex() + 1;
  if (saveImage(std::move(frame), "/picture" + String(frame_index) + ".jpg", SD_MMC)) {
    device_pref.setFrameIndex(frame_index);
  };

  Serial.println("Go to sleep now");
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) +
                 " Seconds");

  // Make sure LED is switched off
  light.off();
  rtc_gpio_hold_en(GPIO_NUM_4);
  esp_deep_sleep_start();
}
