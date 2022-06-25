/*
  ESP32 Matchboxscoe Camera Softare
*/
//#define DEFAULT_STORAGE_TYPE_ESP32 STORAGE_SPIFFS
#define DEFAULT_STORAGE_TYPE_ESP32 STORAGE_SD

//http://192.168.2.168/control?var=flash&val=100

#include "esp_wifi.h"
#include "esp_camera.h"
#include <WiFi.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include <SPIFFS.h>
#include "camera.h"
#include <SD_MMC.h> // SD card
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <esp32-hal-ledc.h>
#include "esp_http_server.h"
#include "esp_timer.h"
#include "esp_camera.h"
#include "img_converters.h"
#include "Arduino.h"
#include <SPIFFS.h>
#include "ArduinoJson.h"
#include "html.h"
#include "device_pref.h"

// stepper motor
#include <AccelStepper.h>

#include <esp_log.h>"

#include <HTTPClient.h>
#include <esp_camera.h>

// Local header files
#include "device_pref.h"

#include "SimpleFTPServer.h"
FtpServer ftpSrv;   //set #define FTP_DEBUG in ESP8266FtpServer.h to see ftp verbose on serial

// ANGLERFISH?
const boolean isAnglerfish = false;
const int timelapseIntervalAnglerfish = 60;
boolean isTimelapseAnglerfish = false;

// OMNISCOPE
const boolean isOmniscope = true;

// WIFI
boolean hostWifiAP = false; // set this variable if you want the ESP32 to be the host
boolean isCaptivePortal = true; // want to autoconnect to wifi networks?
const char* mSSID = "BenMur";//"UC2 - F8Team"; //"IPHT - Konf"; // "Blynk";
const char* mPASSWORD = "MurBen3128"; //"_lachmannUC2"; //"WIa2!DcJ"; //"12345678";
const char* mSSIDAP = "Matchboxscope";
const char* hostname = "matchboxscope";

//WiFiManager, Global intialization. Once its business is done, there is no need to keep it around
WiFiManager wm;

// Camera related
bool isStreaming = false;
bool isStreamingStoppped = false;

// Timelapse
uint64_t timelapseInterval = 60; // sec; timelapse interval // will be read from preferences!
static uint64_t t_old = 0;
boolean isTimelapse = true;

// Server
boolean isWebserver = true;
boolean isFTPServer = true;

// LED
//int pwmResolution = 15;
//int freq = 800000;//19000; //12000
const int freq = 12000;
const int pwmResolution = 8;
const int ledChannel = 7;

const int ledPin = 4;
int ledValueOld = 0;

// LENS
const int lensChannel = 6;
const int lensPin = 13;
int lensValueOld = 0;

// SWITCH for resetting
const int reedSwitchPin = 13;
const int refocus_button_debounce = 1000;

// global camera parameters for REST
uint32_t gain = 0;
uint32_t exposureTime = 0;
uint32_t frameSize = 0;
uint32_t ledintensity = 0;
uint32_t effect = 2;

// SD Card parameters
boolean sdInitialized = false;
boolean isFirstRun = false;
boolean isUseSD = true;

// initiliaze stepper motor
//#define IS_MOTOR
int pinStep = 12;
int pinDir = 13;

// Preferences
Preferences pref;
DevicePreferences device_pref(pref, "camera", __DATE__ " " __TIME__);





void setup()
{
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // prevent brownouts by silencing them

  // device-specific settings
  if (isOmniscope){
    isUseSD = false;
    isWebserver = true;
    isFTPServer = false;
    isTimelapseAnglerfish = false;
    isCaptivePortal = false;
    hostWifiAP = false;
    isTimelapse = false;
  }
  else if(isAnglerfish){
    isUseSD=true;
    isFTPServer=false;
  }

  // INIT SERIAL
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  /*
  * AGLERFISH RELATED
  */

  // Reset the EEPROM's stored timelapse mode after each re-flash
  isFirstRun = device_pref.isFirstRun();
  if (isFirstRun) {
    device_pref.setIsTimelapse(false);
  }

  // INIT CAMERA
  initCamera();
  initCameraSettings();

  // INIT SD
  // We initialize SD_MMC here rather than in setup() because SD_MMC needs to reset the light pin
  // with a different pin mode.
  // 1-bit mode as suggested here:https://dr-mntn.net/2021/02/using-the-sd-card-in-1-bit-mode-on-the-esp32-cam-from-ai-thinker
  if (not isUseSD or !SD_MMC.begin(" / sdcard", true)) {
    Serial.println("SD Card Mount Failed");
    sdInitialized = false;
  }
  else {
    sdInitialized = true;
    Serial.println("SD Card Mounted");

    // Check for an SD card
    uint8_t cardType = SD_MMC.cardType();
    if (cardType == CARD_NONE) {
      Serial.println("No SD card attached");
    }
    else {
      Serial.println(cardType);
    }
  }

  if(isAnglerfish){
    // If the button is pressed, switch from timelapse mode back to refocusing mode
    pinMode(reedSwitchPin, INPUT_PULLDOWN);
    Serial.println("Press Button if you want to refocus (t..1s)");
    delay(refocus_button_debounce);
    bool buttonPressed = !digitalRead(reedSwitchPin); // pull up
    Serial.print("Button pressed ? ");
    if (buttonPressed) {
      Serial.println("yes. Switching from timelapse mode to refocusing mode.");
      device_pref.setIsTimelapse(false);
    } else {
      Serial.println("no");
    }
  }

  // retrieve old camera setting values
  exposureTime = device_pref.getCameraExposureTime();
  gain = device_pref.getCameraGain();
  timelapseInterval = device_pref.getTimelapseInterval();  // Get timelapseInterval
  Serial.print("exposureTime : "); Serial.println(exposureTime );
  Serial.print("gain : "); Serial.println(gain);
  Serial.print("timelapseInterval : "); Serial.println(timelapseInterval);


  // INIT LED
  ledcSetup(ledChannel, freq, pwmResolution);
  ledcAttachPin(ledPin, ledChannel);
  blinkLed(1);
  
  // INIT LENS
  ledcSetup(lensChannel, freq, pwmResolution);
  ledcAttachPin(lensPin, lensChannel);
  blinkLed(1);
  ledcWrite(lensChannel, 255);
  delay(100);
  ledcWrite(lensChannel, 0);

  // INIT STEPPER
  /*#ifdef IS_MOTOR
  AccelStepper stepper(1, pinStep, pinDir);

  stepper.setMaxSpeed(10000);
  stepper.setAcceleration(10000);
  stepper.enableOutputs();

  Serial.println("Movinb forward");
  stepper.runToNewPosition(-1000);
  Serial.println("Movinb bwrd");
  stepper.runToNewPosition(1000);
  #endif
  */

  // Initialize the remaining hardware, depending on the mode
  isTimelapseAnglerfish = device_pref.isTimelapse(); // set the global variable for the loop function
  if (isAnglerfish) {
    if(isTimelapseAnglerfish){
      // ONLY IF YOU WANT TO CAPTURE in ANGLERFISHMODE
      Serial.println("In timelapse mode.");
      // Save image to SD card
      uint32_t frame_index = device_pref.getFrameIndex() + 1;
      if (saveImage(" / picture" + String(frame_index) + ".jpg")) {
        device_pref.setFrameIndex(frame_index);
      };

      // Sleep
      Serial.print("Sleeping for ");
      Serial.print(timelapseIntervalAnglerfish);
      Serial.println(" s");
      static const uint64_t usPerSec = 1000000; // Conversion factor from microseconds to seconds
      esp_sleep_enable_timer_wakeup(timelapseIntervalAnglerfish * usPerSec);
      esp_deep_sleep_start();
      return;
    }
    else{
      Serial.println("In refocusing mode. Connect to Wifi and go to 192.168.4.1 / enable once you're done with focusing.");
    }
  }

  // INIT WIFI
  if (isWebserver) {
    if (isCaptivePortal) {
      isFirstRun=false;
      autoconnectWifi(isFirstRun);
    }
    else {
      if (hostWifiAP) {
        initWifiAP(mSSIDAP);
      }
      else {
        joinWifi(mSSID, mPASSWORD);
      }
    }
  }

  // Blink LED twice
  blinkLed(2);
  

  // INIT SPIFFS
  if (!SPIFFS.begin()) { // SPIFFS must be initialized before the web server, which depends on it
    Serial.println("Couldn't open SPIFFS!");
  }

  // INIT Webserver
  if (isWebserver) {
    // Start the camera and command server
    startCameraServer();

    // INIT FTP Server
    ftpSrv.setCallback(_callback);
    ftpSrv.setTransferCallback(_transferCallback);
    ftpSrv.begin("esp32", "esp32");   //username, password for ftp.   (default 21, 50009 for PASV)
  }

  // initiliaze timer
  t_old = millis();

}

void blinkLed(int nTimes){
  for(int iBlink=0; iBlink<nTimes; iBlink++){
    ledcWrite(ledChannel, 255);
    delay(50);
    ledcWrite(ledChannel, 0);
    delay(50);
  }
  delay(150);
}

void loop() {
  if (isFTPServer) {
    ftpSrv.handleFTP();
  }

  // Perform timelapse imaging
  if (timelapseInterval>0 and isTimelapse and ((millis() - t_old) > (1000 * timelapseInterval))) {
    //https://stackoverflow.com/questions/67090640/errors-while-interacting-with-microsd-card
    t_old = millis();

    uint32_t frame_index = device_pref.getFrameIndex() + 1;
    // Acquire the image and save

    if (saveImage(" / picture" + String(frame_index) + ".jpg")) {
      device_pref.setFrameIndex(frame_index);
    };
  }
}
