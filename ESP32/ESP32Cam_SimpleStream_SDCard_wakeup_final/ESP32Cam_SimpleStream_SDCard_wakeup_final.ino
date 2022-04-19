// UPLOAD INSTRUCTIONS

// Install v2.0.2 of ESP32 board support for the Arduino. Choose the "ESP32 Dev Module" board.
// Enable the PSRAM settings. Upload the sketch.
// Then perform the ESP32 Data Sketch Upload to upload web server data assets to the ESP32.


// HEADERS

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
#include "camera_pref.h"
#include "illumination.h"
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

// Preferences
Preferences pref;
CameraPreferences cam_pref(pref, "camera");

// Web server
void setTimelapse() {
  is_timelapse = true; // focus has been set
  cam_pref.setIsTimelapse(is_timelapse);
}
RefocusingServer server(80, SPIFFS, light, setTimelapse);


// INITIALIZATION

/*
 * 
 * Functions to initialize hardware and auxilary components
 * 
 */
void initSpiffs()
{ // initiliaze internal file system 
  if (!SPIFFS.begin()) 
  {
    Serial.println("Error SPIFFS...");
    return;
  }
  File root = SPIFFS.open("/");    
  File file = root.openNextFile(); 
  while (file)                     
  {// print all files to check if website HtML file exists
    Serial.print("File: ");
    Serial.println(file.name());
    file.close();
    file = root.openNextFile(); 
  }
}


const static auto maxRes = esp32cam::Resolution::find(1600, 1200);
void initCamera() {
  using namespace esp32cam;

  Config cfg;
  cfg.setPins(pins::AiThinker);
  cfg.setResolution(maxRes);
  //cfg.setGrayscale(); // causes problems, doesn't add any functionality other than numpy.mean()
  cfg.setBufferCount(2);
  cfg.setJpeg(80);

  bool ok = Camera.begin(cfg);
  if(not ok){
    ESP.restart();
  }
  Serial.println(ok ? "CAMERA OK" : "CAMERA FAIL");
}



/*
 * 
 * Setup the different functions
 * 
 */

void setup()
{
  Serial.begin(115200);

  // Reset the EEPROM's stored timelapse mode after each re-flash
  if (cam_pref.isFirstRun()) {
     cam_pref.setIsTimelapse(false);
  }

  // If the button is pressed, switch from timelapse mode back to refocusing mode
  pinMode(reedSwitchPin, INPUT_PULLUP);
  Serial.println("Press Button if you want to refocus (t..4s)");
  delay(4000);
  bool buttonPressed = !digitalRead(reedSwitchPin); // pull up
  Serial.print("Button pressed? ");
  if (buttonPressed) {
    Serial.println("yes. Switching from timelapse mode to refocusing mode.");
    cam_pref.setIsTimelapse(false);
  } else {
    Serial.println("no");
  }

  is_timelapse = cam_pref.isTimelapse();

  if (!is_timelapse) {
    Serial.println("In refocusing mode. Connect to Wifi and go to 192.168.4.1/enable once you're done with focusing.");
    if (hostWifiAP) {
      initWifiAP(wifiAPSSID);
    } else {
      joinWifi(wifiSSID, wifiPassword);
    }
    initSpiffs();
    server.init();
  } else {
    Serial.println("In timelapse mode.");
  }

  // initialize camera
  initCamera();
}

void loop()
{
  if (!is_timelapse) {
    // stream images and website to turn focus
    server.serve();
    return;
  }

  // we only want to take one picture, save it and go to deep-sleep.. zzzzz
  uint32_t frameIndex = cam_pref.getFrameIndex() + 1;
  String path = "/picture" + String(frameIndex) + ".jpg";

  if (!esp32cam::Camera.changeResolution(maxRes)) {
    Serial.println("SET-HI-RES FAIL");
  }

  // Acquire the image
  light.on();
  auto frame = esp32cam::capture();
  for (int i = 0; i < 5; i++) { // warm up the camera; usually takes some frames to change resolution too
    frame = esp32cam::capture(); // warm up / adust
  }
  light.off();

  // rest pin 4 and save image on SD card
  if (!SD_MMC.begin()) { // (!SD_MMC.begin("/sdcard", true)) { //1bit mode -
    Serial.println("SD Card Mount Failed");
  }
  Serial.printf("Picture file name: %s\n", path.c_str());

  fs::FS &fs = SD_MMC; 
  File file = fs.open(path.c_str(), FILE_WRITE);
  Serial.println("Opening file");
  if (!file) {
    Serial.println("Failed to open file in writing mode");
    file.close();
  } else {
    if (frame == nullptr) {
      Serial.println("CAPTURE FAIL");
      file.close();
    }
    Serial.printf("CAPTURE OK %dx%d %db\n", frame->getWidth(), frame->getHeight(),
                  static_cast<int>(frame->size()));

    file.write(frame->data(), frame->size()); // payload (image), payload length
    Serial.println("Finished writing file.");
    cam_pref.setFrameIndex(frameIndex);

    file.close();
  }

  Serial.println("Go to sleep now");
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) +
                 " Seconds");

  // Make sure LED is switched off
  light.off();
  rtc_gpio_hold_en(GPIO_NUM_4);
  esp_deep_sleep_start();
}
