/*
  ESP32 Matchboxscoe Camera Software
  inspired by:
  https://www.hackster.io/KDPA/esp32-cam-video-surveillance-robot-a22367
*/
// Some URLS for controlling e.g. the light intensity in the browser
//http://192.168.2.168/control?var=flash&val=100
//http://192.168.1.4/controll?var=lens&val=100

/*
  HEADERS
*/

// ESP-IDF headers (in alphabetical order!)
#include <esp_log.h>
#include <esp_camera.h>
#include <esp_http_server.h>
#include <esp_timer.h>
#include <esp_wifi.h>
#include <img_converters.h>
#include <soc/soc.h>
#include <soc/rtc_cntl_reg.h>

// Arduino ESP32 headers (in alphabetical order!)
#include <Arduino.h>
#include <esp32-hal-ledc.h>
#include <FS.h>
#include <HTTPClient.h>
#include <SD_MMC.h>
#include <SPIFFS.h>
#include <Update.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

// Third-party library headers  (in alphabetical order!)
#include "ArduinoJson.h" // https://github.com/bblanchon/ArduinoJson
#include "ESP32Ping.h" // https://github.com/marian-craciunescu/ESP32Ping
#include "WiFiManager.h" // https://github.com/tzapu/WiFiManager

// Local header files (in alphabetical order!)
#include "Base64.h"
#include "camera.h"
//#include "cameraM5Stack.h"
#include "device_pref.h"
#include "html.h"

/*
  USER-CONFIGURED SETTINGS
*/

// Anglerfish

//FIXME: We mostly need to differentiate between Matchboxcope/Anglerfish, where Anglerfish has the "dangerzone" aka: deepsleep that will never wake up
const int timelapseIntervalAnglerfish = 10;//10*60; // FIXME: This value should be adjustable through the GUI
const int focuStackStepsizeAnglerfish = 25; // FIXME: This value should be adjustable through the GUI

// Wifi

const boolean hostWifiAP = true; // set this variable if you want the ESP32 to be the host
const boolean isCaptivePortal = false; // want to autoconnect to wifi networks?
const char *mSSID = "Blynk";
const char *mPASSWORD = "12345678";
const char *mSSIDAP = "Matchboxscope";
const char *hostname = "matchboxscope";

// Google Drive Upload

const char *myDomain = "script.google.com";
const String myScript = "/macros/s/AKfycbwF8y5az641P2EUkooJjpEVn36Bpu2nAxYpQ8WOcr0kWiBmnxP2jH1xdsvjc55rN14w/exec";
const String myFilename = "filename = ESP32 - CAM.jpg";
const String mimeType = "&mimetype = imagejpeg";
const String myImage = "&datase = ";

const int waitingTime = 30000; //Wait 30 seconds to google response.

/*
  GLOBAL CONSTANTS & STATE
*/

// LED

const int freq = 8000; //800000;//19000; //12000
const int pwmResolution = 8; //15
const int ledChannel = 4; //some are used by the camera
const int ledPin = 4;
int ledValueOld = 0;

// LED array
int ledMatrixCount = 2;

// Lens
const int lensChannel = 5; //some are used by the camera
const int lensPin = 12;
uint32_t lensValueOld = 0;

// Button/reed-switch for resetting
const int refocus_button_debounce = 1000;

// Camera
uint32_t gain = 0;
uint32_t exposureTime = 0;
uint32_t frameSize = 0;
uint32_t ledintensity = 0;
uint32_t effect = 2;
bool isStreaming = false;
bool isStreamingStoppped = false;
bool isCameraAttached = false;

// SD Card
boolean sdInitialized = false;
boolean isFirstRun = false;
boolean isUseSD = true;

// Timelapse
uint64_t timelapseInterval = -1; // FIXME: we should have a button in the GUI to enable it ;  sec; timelapse interval // will be read from preferences!
static uint64_t t_old = 0;

// Preferences
Preferences pref;
DevicePreferences device_pref(pref, "camera", __DATE__ " " __TIME__);

// Wifi
WiFiManager wm;
bool isInternetAvailable = false;
// check wifi connectibility if not connected, try to reconnect - or restart?
unsigned long previousCheckWifi = 0;
unsigned long delayReconnect = 20000;  // 20 seconds delay

// Server
boolean isWebserver = true;

// Anglerfish
boolean isTimelapseAnglerfish = false; // keep as false!
boolean isAcquireStack = false; // acquire only single image or stack?
File myFile;

// OTA server
WebServer OTAserver(82);

void setup()
{
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // prevent brownouts by silencing them

  // INIT SERIAL
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  //Print the wakeup reason for ESP32
  print_wakeup_reason();
  detachInterrupt(T3); // FIXME: just in case?



  /*
    AGLERFISH RELATED
  */
  // Reset the EEPROM's stored timelapse mode after each re - flash
  isFirstRun = device_pref.isFirstRun();
  if (isFirstRun) {
    device_pref.setIsTimelapse(false);
  }

  // retrieve old camera setting values
  exposureTime = device_pref.getCameraExposureTime();
  gain = device_pref.getCameraGain();
  timelapseInterval = device_pref.getTimelapseInterval();  // Get timelapseInterval
  Serial.print("exposureTime : "); Serial.println(exposureTime );
  Serial.print("gain : "); Serial.println(gain);
  Serial.print("timelapseInterval : "); Serial.println(timelapseInterval);


  // INIT CAMERA
  isCameraAttached = initCamera();
  if (isCameraAttached) {
    Serial.println("Camera is working");
    initCameraSettings(); // set camera settings - exposure time and gain will betaken from preferences
  }
  else {
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
    device_pref.setIsTimelapse(false); // FIXME: if SD card is missing => streaming mode!
    // FIXME: won't work since LEDC is not yet initiated blinkLed(5);
    /*
      setLens(255); delay(100); setLens(0); delay(100);
      setLens(255); delay(100); setLens(0); delay(100);
      setLens(255); delay(100); setLens(0); delay(100);
    */
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

    listDir(SD_MMC, "/", 0);
    Serial.println("Writing to file...");
    writeFile(SD_MMC, "/debug.txt", "Starting to debug...");

  }





  // Setting up LED
  appendFile(SD_MMC, "/debug.txt", "LOG 1!\n");
  ledcSetup(ledChannel, freq, pwmResolution);
  ledcAttachPin(ledPin, ledChannel);
  appendFile(SD_MMC, "/debug.txt", "LOG 1.1!\n");
  ledcSetup(lensChannel, freq, pwmResolution);
  ledcAttachPin(lensPin, lensChannel);


  // Test Hardware
  appendFile(SD_MMC, "/debug.txt", "LOG 2!\n");
  setLED(255);
  delay(100);
  setLED(0);

  setLens(255);
  delay(100);
  setLens(0);



  // only for Anglerfish if already focussed
  appendFile(SD_MMC, "/debug.txt", "LOG 3!\n");
  isTimelapseAnglerfish = device_pref.isTimelapse(); // set the global variable for the loop function

  if (isTimelapseAnglerfish) {
    appendFile(SD_MMC, "/debug.txt", "LOG 4!\n");
    int ledIntensity = 255;

    // override  camera settings
    sensor_t * s = esp_camera_sensor_get();
    s->set_framesize(s, FRAMESIZE_UXGA);
    s->set_quality(s, 10);


    // ONLY IF YOU WANT TO CAPTURE in ANGLERFISHMODE
    Serial.println("In timelapse mode.");
    // Save image to SD card
    uint32_t frame_index = device_pref.getFrameIndex() + 1;

    // save frame - eventually
    bool imageSaved = false;

    // FIXME: decide which method to use..
    appendFile(SD_MMC, "/debug.txt", "LOG 5!\n");
    device_pref.setFrameIndex(frame_index);
    imageSaved = doFocus(focuStackStepsizeAnglerfish, true, false, "anglerfish_" + String(frame_index));
    //imageSaved = snapPhoto("anglerfish_" + String(frame_index), ledIntensity);

    // also take Darkfield image
    //FIXME: This becomes obsolete nowimageSaved = snapPhoto("picture_LED1_"  + String(frame_index), 1, ledIntensity);
    appendFile(SD_MMC, "/debug.txt", "LOG 6!\n");
    /*if (imageSaved) {//FIXME: we should increase framenumber even if failed - since a corrupted file may lead to issues?
      device_pref.setFrameIndex(frame_index);
      }
    */

    // Sleep
    Serial.print("Sleeping for ");
    Serial.print(timelapseIntervalAnglerfish);
    Serial.println(" s");
    static const uint64_t usPerSec = 1000000; // Conversion factor from microseconds to seconds
    esp_sleep_enable_timer_wakeup(timelapseIntervalAnglerfish * usPerSec);
    myFile.close();
    SD_MMC.end(); // FIXME: may cause issues when file not closed? categoreis: LED/SD-CARD issues


    // After SD Card init? and after the Lens was used?
    // ATTENTIONN: DON'T USE ANY SD-CARD RELATED GPIO!!
    // set a wakeup pin so that we reset the Snow-white deepsleep and turn on the Wifi again: // FIXME: Makes sense?
    //esp_sleep_enable_ext0_wakeup(GPIO_NUM_15, 1); //=> GPIO: 4, level: 1
    //Setup interrupt on Touch Pad 3 (GPIO15)
    //touchAttachInterrupt(T3, callbackTouchpad, 40);
    //Configure Touchpad as wakeup source
    //esp_sleep_enable_touchpad_wakeup();

    Serial.println("Set pin 12 high to wake up the ESP32 from deepsleep");


    esp_deep_sleep_start();
    return;
  }
  else {
    Serial.println("In refocusing mode. Connect to Wifi and go to 192.168.4.1enable once you're done with focusing.");
  }

  setLens(255);
  delay(100);
  setLens(0);

  // After SD Card init? and after the Lens was used?
  // ATTENTIONN: DON'T USE ANY SD - CARD RELATED GPIO!!
  // set a wakeup pin so that we reset the Snow-white deepsleep and turn on the Wifi again: // FIXME: Makes sense?
  //esp_sleep_enable_ext0_wakeup(GPIO_NUM_15, 1); //=> GPIO: 4, level: 1

  //Setup interrupt on Touch Pad 3 (GPIO15)
  //touchAttachInterrupt(T3, callbackTouchpad, 40);
  //Configure Touchpad as wakeup source
  //esp_sleep_enable_touchpad_wakeup();

  Serial.println("Set pin 12 high to wake up the ESP32 from deepsleep");


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
  if (!isInternetAvailable or hostWifiAP) {
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

  // reconnect wifi

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
      setLens(0); // save energy

    }
    else {
      // Acquire the image and save
      imageSaved = saveImage("/picture" + String(frame_index) + ".jpg", ledValueOld, lensValueOld);
    }

    if (true) { //FIXME: we should increase framenumber even if failed - since a corrupted file may lead to issues? (imageSaved) {
      device_pref.setFrameIndex(frame_index);
    };
    // turn off led
    setLED(0);
  }

  // checking for WIFI connection
  unsigned long currentTime = millis(); // number of milliseconds since the upload
  if (not hostWifiAP and (WiFi.status() != WL_CONNECTED) && (currentTime - previousCheckWifi >= delayReconnect)) {
    Serial.print(millis());
    Serial.println("Reconnecting to WIFI network");
    WiFi.disconnect();
    // FIXME: Here we should offer an ACCESS POINT where we can supply credentials for SSID and PW
    if (0)
      WiFi.reconnect();
    else
      ESP.restart();
    previousCheckWifi = currentTime;
  }
}



//https://randomnerdtutorials.com/esp32-deep-sleep-arduino-ide-wake-up-sources/
void print_wakeup_reason() {
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch (wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 :
      Serial.println("Wakeup caused by external signal using RTC_IO");
      break;
    case ESP_SLEEP_WAKEUP_EXT1 :
      Serial.println("Wakeup caused by external signal using RTC_CNTL");
      break;
    case ESP_SLEEP_WAKEUP_TIMER :
      Serial.println("Wakeup caused by timer");
      // VSM still not working after automatic reboot - hitting Reset does the job :/
      ESP.restart(); // FIMXE: Yup, this is weird: Since we connect the awake-Button AND the VCM transistor to Pin12, the pin is still in input mode when the esp is woken up by timer..so we have to get another cause for the wake up=> force restart!
      break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD :
      Serial.println("Wakeup caused by touchpad");
      break;
    case ESP_SLEEP_WAKEUP_ULP :
      Serial.println("Wakeup caused by ULP program");
      break;
    default :
      Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason);
      break;
  }
}

void writeFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
}


void appendFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if (!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if (file.print(message)) {
    Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
}

void callbackTouchpad() {

}
