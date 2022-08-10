/*
  ESP32 Matchboxscoe Camera Software
  inspired by:
  https://www.hackster.io/KDPA/esp32-cam-video-surveillance-robot-a22367
*/
//#define DEFAULT_STORAGE_TYPE_ESP32 STORAGE_SPIFFS
#define DEFAULT_STORAGE_TYPE_ESP32 STORAGE_SD

//http://192.168.2.168/control?var=flash&val=100
//http://192.168.1.4/control?var=lens&val=100

// external libraries
#include <Adafruit_NeoPixel.h>
#include "esp_wifi.h"
#include "esp_camera.h"
#include <WiFi.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include <SPIFFS.h>
#include "camera.h"
//#include "cameraM5Stack.h"
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
#include <AccelStepper.h>
#include <esp_log.h>"
#include "ESP32FtpServer.h"
#include <Update.h>


// Local header files
#include <HTTPClient.h>
#include <esp_camera.h>
#include "device_pref.h"

// FTP server to access data outside the "Jar"
FtpServer ftpSrv;   //set #define FTP_DEBUG in ESP32FtpServer.h to see ftp verbose on serial

/**********************

ANGLERFISH settings

**********************/
const boolean isAnglerfish = true;
const int timelapseIntervalAnglerfish = 60;
boolean isTimelapseAnglerfish = false; // keep as false!
boolean anglerfishIsAcquireStack = true; // acquire only single image or stack?

/**********************

OMNISCOPE settings

**********************/
const boolean isOmniscope = false;

/**********************

WIFI

**********************/
boolean hostWifiAP = false; // set this variable if you want the ESP32 to be the host
boolean isCaptivePortal = true; // want to autoconnect to wifi networks?
const char* mSSID = "BenMur";//"UC2 - F8Team"; //"IPHT - Konf"; // "Blynk";
const char* mPASSWORD = "MurBen3128"; //"_lachmannUC2"; //"WIa2!DcJ"; //"12345678";
const char* mSSIDAP = "Matchboxscope";
const char* hostname = "matchboxscope";
WiFiManager wm;

// Camera related
bool isStreaming = false;
bool isStreamingStoppped = false;


/**********************

Timelapse

**********************/
uint64_t timelapseInterval = 60; // sec; timelapse interval // will be read from preferences!
static uint64_t t_old = 0;
boolean isTimelapse = true;

// Server
boolean isWebserver = true;
boolean isFTPServer = true;


/**********************

Ext. HARDWARE

**********************/
// LED
const int freq = 8000; //800000;//19000; //12000
const int pwmResolution = 8; //15
const int ledChannel = 3; //some are used by the camera
const int ledPin = 4;
int ledValueOld = 0;

// LENS
const int lensChannel =  4; //some are used by the camera
const int lensPin = 13;
uint32_t lensValueOld = 0;

// Button/reed-switch for resetting
const int reedSwitchPin = 12;
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

// LED ARRAY
int ledMatrixCount = 2;

// Declare our NeoPixel strip object:
Adafruit_NeoPixel strip;

/*
*  OTA Server
*/

WebServer OTAserver(82);

/*
* CAMERA
*/
bool isCameraAttached = false;

void setup()
{
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // prevent brownouts by silencing them

  // device-specific flags
  if (isOmniscope){
    Serial.println("I', an omniscope");
    isUseSD = false;
    isWebserver = true;
    isFTPServer = false;
    isTimelapseAnglerfish = false;
    isCaptivePortal = true;
    hostWifiAP = false;
    isTimelapse = false;
  }
  else if(isAnglerfish){
    Serial.println("I', an anglerfish");
    isUseSD = true;
    isWebserver = true;
    isFTPServer = true;
    isTimelapseAnglerfish = false;
    isCaptivePortal = true;//false;
    hostWifiAP = false;//true;
    isTimelapse = false;
  }
  else{ // most likely it's matchboxscope
    Serial.println("I', a matchboxscope");
    isUseSD = true;
    isWebserver = true;
    isFTPServer = true;
    isTimelapseAnglerfish = false;
    isCaptivePortal = false;
    hostWifiAP = false;
    isTimelapse = true;
  }

  // INIT SERIAL
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();


  /*
  * AGLERFISH RELATED
  */
  // Reset the EEPROM's stored timelapse mode after each re - flash
  isFirstRun = device_pref.isFirstRun();
  if (isFirstRun) {
    device_pref.setIsTimelapse(false);
  }

  // INIT CAMERA
  isCameraAttached = initCamera();
  if (isCameraAttached) {
    initCameraSettings();
  }

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

  if (isAnglerfish) {
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

    // LED ARRAY
    strip = Adafruit_NeoPixel(ledMatrixCount, ledPin, NEO_GRB + NEO_KHZ800);
    strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
    strip.show();            // Turn OFF all pixels ASAP
    strip.setBrightness(255); // Set BRIGHTNESS to about 1/5 (max = 255)
    //ledcSetup(ledChannel, freq, pwmResolution);
    //ledcAttachPin(0, ledChannel); // necessary?
  }
  else {
    Serial.println("Setting up LED / PWM");
    ledcSetup(ledChannel, freq, pwmResolution);
    ledcAttachPin(ledPin, ledChannel);
  }

  // retrieve old camera setting values
  exposureTime = device_pref.getCameraExposureTime();
  gain = device_pref.getCameraGain();
  timelapseInterval = device_pref.getTimelapseInterval();  // Get timelapseInterval
  Serial.print("exposureTime : "); Serial.println(exposureTime );
  Serial.print("gain : "); Serial.println(gain);
  Serial.print("timelapseInterval : "); Serial.println(timelapseInterval);


  // INIT LED
  blinkLed(1);

  // INIT LENS
  moveLens(255);
  delay(100);
  moveLens(0);


  // only for Anglerfish if already focussed
  isTimelapseAnglerfish = device_pref.isTimelapse(); // set the global variable for the loop function
  if (isAnglerfish) {
    if (isTimelapseAnglerfish) {
      int ledIntensity = 255;

      // ONLY IF YOU WANT TO CAPTURE in ANGLERFISHMODE
      Serial.println("In timelapse mode.");
      // Save image to SD card
      uint32_t frame_index = device_pref.getFrameIndex() + 1;

      // save frame - eventually
      bool imageSaved = false;
      imageSaved = snapPhoto(" / picture_LED0_" + String(frame_index), 0, ledIntensity);

      if (isAnglerfish) {
        // also take Darkfield image
        imageSaved = snapPhoto(" / picture_LED1_"  + String(frame_index), 1, ledIntensity);

        if (imageSaved) {
          device_pref.setFrameIndex(frame_index);
        }
      }

      // Sleep
      Serial.print("Sleeping for ");
      Serial.print(timelapseIntervalAnglerfish);
      Serial.println(" s");
      static const uint64_t usPerSec = 1000000; // Conversion factor from microseconds to seconds
      esp_sleep_enable_timer_wakeup(timelapseIntervalAnglerfish * usPerSec);
      esp_deep_sleep_start();
      return;
    }
    else {
      Serial.println("In refocusing mode. Connect to Wifi and go to 192.168.4.1 / enable once you're done with focusing.");
    }
  }

  moveLens(255);
  delay(100);
  moveLens(0);

  // INIT WIFI
  if (isWebserver) {
    if (isCaptivePortal) {
      // create a captive portal to connect to an existing WiFi AP with SSID/PW provided through the portal
      isFirstRun = false;
      autoconnectWifi(isFirstRun);
    }
    else {
      if (hostWifiAP) {
        // create an ESP32-based AP
        initWifiAP(mSSIDAP);
      }
      else {
        // connect to an existing Wifi /W SSID/PW
        joinWifi(mSSID, mPASSWORD);
      }
    }
  }

  // INIT SPIFFS
  /*
    if (!SPIFFS.begin()) { // SPIFFS must be initialized before the web server, which depends on it
    Serial.println("Couldn't open SPIFFS!");
    }
  */

  // INIT Webserver
  if (isWebserver) {
    // Start the camera and command server
    startCameraServer();

    moveLens(255);
    delay(100);
    moveLens(0);

    // INIT FTP Server
    //ftpSrv.setCallback(_callback);
    //ftpSrv.setTransferCallback(_transferCallback);
    if (isFTPServer and sdInitialized) {
      Serial.println("Starting FTP Server");
      ftpSrv.begin("esp32", "esp32");   //username, password for ftp.   (default 21, 50009 for PASV)
    }
  }

  // OTA
  startOTAServer();

  // initiliaze timer
  t_old = millis();



}


void loop() {

  // wait for incoming OTA client udpates
  OTAserver.handleClient();

  // offer file transfer via FTP - eventually
  if (isFTPServer and sdInitialized) {
    ftpSrv.handleFTP();
  }

  // Perform timelapse imaging
  if (timelapseInterval > 0 and isTimelapse and ((millis() - t_old) > (1000 * timelapseInterval))) {
    //https://stackoverflow.com/questions/67090640/errors-while-interacting-with-microsd-card
    t_old = millis();

    if (anglerfishIsAcquireStack) {
      // acquire a stack
      doFocus(5, true, true);
      // switch off lens
      moveLens(0);
    }
    else {
      uint32_t frame_index = device_pref.getFrameIndex() + 1;
      // Acquire the image and save
      moveLens(lensValueOld);
      if (saveImage(" / picture" + String(frame_index) + ".jpg")) {
        device_pref.setFrameIndex(frame_index);
      };
    }
  }
}

void setLED(int numberLED, int intensity) {
  if (isAnglerfish) {
    // use LED strip
    Serial.print("LED ARRAY:");
    Serial.println(intensity);
    strip.setPixelColor(numberLED, strip.Color(intensity, intensity, intensity));
    strip.show();            // Turn OFF all pixels ASAP
  }
  else {
    // use internal LED/TORCH
    Serial.print("LED:");
    Serial.println(intensity);
    ledcWrite(ledChannel, intensity);
  }
}

void blinkLed(int nTimes) {
  //TODO: Be careful with this - interferes with sensor and ledcWrite?!
  for (int iBlink = 0; iBlink < nTimes; iBlink++) {
    setLED(0, 255);
    setLED(1, 255);
    delay(50);
    setLED(0, 0);
    setLED(1, 0);
    delay(50);
  }
  delay(150);
}
