//IMPORTANT: ENABLE PSRAM!
#include <esp32cam.h>
#include <WebServer.h>
#include "esp_camera.h"
#include "Arduino.h"
#include "soc/soc.h"           // Disable brownour problems
#include "soc/rtc_cntl_reg.h"  // Disable brownour problems
#include "driver/rtc_io.h"
#include <Preferences.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include <FS.h>
#include "SD_MMC.h" // SD card  

// Compilation constants
#define WIFIAP                  // want to use the ESP32 as an access point?

// Local header files
#include "camera_pref.h"
#include "webserver.h" // FIXME: this header file declares a `server` global variable, but it also depends on the `is_timelapse` global variable declared in this main .ino file!

/*
 * 
 * TODO's:
 * 
 * - Whitebalance/constant intensity
 * - Swap esp32cam.h with esp_camera?
 * - link /enable to website
 * - fit mjjpeg into wevbsite proerly
 */

// Constant settings
static const int PIN_REEDSWITCH = 13; // pin of the external switch to "awake" the ESP (e.g. Reedrelay)

// conversion factors for letting the ESP go sleep
#define uS_TO_S_FACTOR 1000000  // Conversion factor for micro seconds to seconds 
#define TIME_TO_SLEEP  10       // Time ESP32 will go to sleep (in seconds) 


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


void initiWiFi() {
  // connect to Wifi; either AP or through Router
#ifdef WIFIAP
  Serial.print("Network SSID: ");
  Serial.println(SSID);

  WiFi.softAP(SSID);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

#else
  Serial.print("Connecting to ");
  Serial.println(SSID);

  WiFi.begin(SSID, PWD);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
    // we can even make the ESP32 to sleep
  }

  Serial.print("Connected. IP: ");
  Serial.println(WiFi.localIP());
#endif
}



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
  pinMode(PIN_REEDSWITCH, INPUT_PULLUP);
  Serial.println("Press Button if you want to refocus (t..4s)");
  delay(4000);
  bool buttonPressed = !digitalRead(PIN_REEDSWITCH); // pull up
  Serial.print("Button pressed? ");
  if (buttonPressed) {
    Serial.println("yes.");
    Serial.println("Switching from timelapse mode to refocusing mode.");
    cam_pref.setIsTimelapse(false);
  } else {
    Serial.println("no");
  }

  is_timelapse = cam_pref.isTimelapse();

  if (!is_timelapse) {
    Serial.println("In refocusing mode. Connect to Wifi and go to 192.168.4.1/enable once you're done with focusing.");
    initiWiFi();
    initSpiffs();
    initWebServerConfig();
  } else {
    Serial.println("In timelapse mode.");
  }

  // initialize camera
  initCamera();

  // setup pin to work with the flash LED - make sure it's available for the SD card later!
  pinMode(PIN_LED, OUTPUT);
}

void loop()
{
  if (!is_timelapse) {
    // stream images and website to turn focus
    server.handleClient();
    return;
  }

  // we only want to take one picture, save it and go to deep-sleep.. zzzzz
  uint32_t frameIndex = cam_pref.getFrameIndex() + 1;
  String path = "/picture" + String(frameIndex) + ".jpg";

  if (!esp32cam::Camera.changeResolution(maxRes)) {
    Serial.println("SET-HI-RES FAIL");
  }

  // turn on led to see anything..
  digitalWrite(PIN_LED, HIGH); 
  
  // warm up the camera; usually takes some frames to change resolution too
  auto frame = esp32cam::capture();
  for (int i = 0; i < 5; i++) {
    frame = esp32cam::capture(); // warm up / adust
  }

  // Make sure LEd is switched off
  digitalWrite(4, LOW);

  // rest pinnumber 4 and save image on SD card
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

  // Make sure LEd is switched off
  digitalWrite(4, LOW);
  rtc_gpio_hold_en(GPIO_NUM_4);
  esp_deep_sleep_start();
}
