/*
  ESP32 Matchboxscoe Camera Software
  inspired by:
  https://www.hackster.io/KDPA/esp32-cam-video-surveillance-robot-a22367
*/
// Some URLS for controlling e.g. the light intensity in the browser
//http://192.168.2.168/control?var=flash&val=100
//http://192.168.1.4/controll?var=lens&val=100

// external libraries

#include "esp_wifi.h"
#include "esp_camera.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include <SPIFFS.h>
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
#include "device_pref.h"
#include <AccelStepper.h>
#include <esp_log.h>"
#include <Update.h>

#include "Base64.h"


// Local header files
#include <HTTPClient.h>
#include <esp_camera.h>
#include "device_pref.h"
#include <ESP32Ping.h>
#include "html.h"
#include "camera.h"
//#include "cameraM5Stack.h"


/**********************

ANGLERFISH settings

**********************/
//FIXME: We mostly need to differentiate between Matchboxcope/Anglerfish, where Anglerfish has the "dangerzone" aka: deepsleep that will never wake up
const int timelapseIntervalAnglerfish = 10*60; // FIXME: This value should be adjustable through the GUI
const int focuStackStepsizeAnglerfish = 25; // FIXME: This value should be adjustable through the GUI
boolean isTimelapseAnglerfish = false; // keep as false!
boolean isAcquireStack = false; // acquire only single image or stack?


/**********************

WIFI

**********************/
boolean hostWifiAP = false; // set this variable if you want the ESP32 to be the host
boolean isCaptivePortal = true; // want to autoconnect to wifi networks?
const char* mSSID = "Blynk2";//"UC2 - F8Team"; //"IPHT - Konf"; // "Blynk";
const char* mPASSWORD = "12345678"; //"_lachmannUC2"; //"WIa2!DcJ"; //"12345678";
const char* mSSIDAP = "Matchboxscope";
const char* hostname = "matchboxscope";
WiFiManager wm;
// check wifi connectibility if not connected, try to reconnect - or restart?
unsigned long previousCheckWifi = 0;
unsigned long delayReconnect = 20000;  // 20 seconds delay
bool isInternetAvailable = false;

// Camera related
bool isStreaming = false;
bool isStreamingStoppped = false;


/*********************
*
* Upload to Google Drive Settings
*
*********************/

const char* myDomain = "script.google.com";
String myScript = "/macros/s/AKfycbwF8y5az641P2EUkooJjpEVn36Bpu2nAxYpQ8WOcr0kWiBmnxP2jH1xdsvjc55rN14w/exec";
String myFilename = "filename = ESP32 - CAM.jpg";
String mimeType = "&mimetype = imagejpeg";
String myImage = "&datase = ";

int waitingTime = 30000; //Wait 30 seconds to google response.


/**********************

Timelapse

**********************/
uint64_t timelapseInterval = -1; // FIXME: we should have a button in the GUI to enable it ;  sec; timelapse interval // will be read from preferences!
static uint64_t t_old = 0;

// Server
boolean isWebserver = true;


/**********************

Ext. HARDWARE

**********************/
// LED

const int freq = 8000; //800000;//19000; //12000
const int pwmResolution = 8; //15
const int ledChannel = 4; //some are used by the camera
const int ledPin = 4;
int ledValueOld = 0;

// LENS
const int lensChannel = 5; //some are used by the camera
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

  // INIT SERIAL
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();


  /*
  AGLERFISH RELATED
  */
  // Reset the EEPROM's stored timelapse mode after each re - flash
  isFirstRun = device_pref.isFirstRun();
  if (isFirstRun) {
    device_pref.setIsTimelapse(false);
  }

  // INIT CAMERA
  isCameraAttached = initCamera();
  if (isCameraAttached) {
    Serial.println("Camera is working");
    initCameraSettings();
  }
  else{
    Serial.println("Camera is not working");
  }

  // INIT SD
  // We initialize SD_MMC here rather than in setup() because SD_MMC needs to reset the light pin
  // with a different pin mode.
  // 1-bit mode as suggested here:https://dr-mntn.net/2021/02/using-the-sd-card-in-1-bit-mode-on-the-esp32-cam-from-ai-thinker
  if (!SD_MMC.begin("/sdcard", true)) { //FIXME: this sometimes leads to issues Unix vs. Windows formating - text encoding? Sometimes it copies to "sdcard" => Autoformating does this!!!
    Serial.println("SD Card Mount Failed");
    //FIXME: This should be indicated in the GUI 
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

  /* FIXME: I think a switch for the switch in case people have not connected it would be good? ;)
   *  FIXME2: Alternatively we could check for the existance of a file on the SD card - then=> no soldering required, yey!
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

  Serial.println("Using LEDc");
  digitalWrite(ledChannel, HIGH);
  delay(100);
  digitalWrite(ledChannel, LOW);
  ledcSetup(ledChannel, freq, pwmResolution);
  ledcAttachPin(ledPin, ledChannel);

  }
  else {
  Serial.println("Setting up LEDPWM");
  }
  */

  // retrieve old camera setting values
  exposureTime = device_pref.getCameraExposureTime();
  gain = device_pref.getCameraGain();
  timelapseInterval = device_pref.getTimelapseInterval();  // Get timelapseInterval
  Serial.print("exposureTime : "); Serial.println(exposureTime );
  Serial.print("gain : "); Serial.println(gain);
  Serial.print("timelapseInterval : "); Serial.println(timelapseInterval);

  // Setting up LED
  ledcSetup(ledChannel, freq, pwmResolution);
  ledcAttachPin(ledPin, ledChannel);

  // Test Hardware
  blinkLed(1);

  moveLens(255);
  delay(100);
  moveLens(0);



  // only for Anglerfish if already focussed
  isTimelapseAnglerfish = device_pref.isTimelapse(); // set the global variable for the loop function

  if (isTimelapseAnglerfish) {
    int ledIntensity = 255;

    // ONLY IF YOU WANT TO CAPTURE in ANGLERFISHMODE
    Serial.println("In timelapse mode.");
    // Save image to SD card
    uint32_t frame_index = device_pref.getFrameIndex() + 1;

    // save frame - eventually
    bool imageSaved = false;

    // FIXME: decide which method to use..
    imageSaved = doFocus(focuStackStepsizeAnglerfish, true, false, "anglerfish_" + String(frame_index));
    //imageSaved = snapPhoto("anglerfish_" + String(frame_index), ledIntensity);

    // also take Darkfield image
    //FIXME: This becomes obsolete nowimageSaved = snapPhoto("picture_LED1_"  + String(frame_index), 1, ledIntensity);
    if (imageSaved) {
      device_pref.setFrameIndex(frame_index);
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
    Serial.println("In refocusing mode. Connect to Wifi and go to 192.168.4.1enable once you're done with focusing.");
  }


  moveLens(255);
  delay(100);
  moveLens(0);

  // INIT WIFI
  // FIXME: The strategy should be:
  // 1. If starting up and no Wifi is set up (e.g. EEPROM settings empty), offer an AP (SSID: anglerfish)
  // 1.a. if settings are available => connect
  // 1.b. if wifi settings are available but not valid => offer AP
  // 2. Save Wifi settings from "captive portal" (not necessarily the Wifimanager.h - bloatware? ;) ) and connect
  // 3. continousyl check if wifi signal available, if not => restart
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


  // INIT SPIFFS
  /*
  if (!SPIFFS.begin()) { // SPIFFS must be initialized before the web server, which depends on it
  Serial.println("Couldn't open SPIFFS!");
  }
  */

  // INIT Webserver

  // Start the camera and command server
  startCameraServer();


  // OTA
  startOTAServer();

  // initiliaze timer
  t_old = millis();

  // FIXME: This is just a tet to see if this works in general - the standalone application works; My guess: An issue with the image dimensions
  isInternetAvailable = Ping.ping("www.google.com", 3);
  if (!isInternetAvailable) {
    Serial.println("Ping failed -> we are not connected to the internet most likely!");
  }
  else {
    Serial.println("Ping succesful -> we are connected to the internet most likely!.");
    // Save image in Google Drive
    saveCapturedImageGDrive();
  }




}


void loop() {

  // wait for incoming OTA client udpates
  OTAserver.handleClient(); // FIXME: the OTA, "REST API" and stream run on 3 different ports - cause: me not being able to merge OTA and REST; STREAM shuold be independent to have a non-blockig experience

  // Perform timelapse imaging
  if (timelapseInterval > 0 and ((millis() - t_old) > (1000 * timelapseInterval))) {
    //https://stackoverflow.com/questions/67090640/errors-while-interacting-with-microsd-card
    t_old = millis();
    uint32_t frame_index = device_pref.getFrameIndex() + 1;
    bool imageSaved = false;

    // turn on led
    setLED(ledValueOld);


    if (isAcquireStack) { // FIXME: We could have a switch in the GUI for this settig
      // acquire a stack
      // FIXME: decide which method to use..
      imageSaved = doFocus(5, true, false, "/anglerfish_" + String(frame_index));

      // switch off lens
      moveLens(0); // save energy

    }
    else {
      // Acquire the image and save
      moveLens(lensValueOld);
      imageSaved = saveImage("/picture" + String(frame_index) + ".jpg");
    }

    if (imageSaved) {
      device_pref.setFrameIndex(frame_index);
    };
    // turn off led
    setLED(0);
  }

  // checking for WIFI connection
  unsigned long currentTime = millis(); // number of milliseconds since the upload
  if ((WiFi.status() != WL_CONNECTED) && (currentTime - previousCheckWifi >= delayReconnect)) {
    Serial.print(millis());
    Serial.println("Reconnecting to WIFI network");
    WiFi.disconnect();
    if (0)
      WiFi.reconnect();
    else
      ESP.restart();
    previousCheckWifi = currentTime;
  }
}
