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

#include <esp_log.h>"

#include <HTTPClient.h>
#include <esp_camera.h>

// Local header files
#include "device_pref.h"

#include "SimpleFTPServer.h"
FtpServer ftpSrv;   //set #define FTP_DEBUG in ESP8266FtpServer.h to see ftp verbose on serial

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

// LED
const int freq = 5000;
const int ledChannel = 7;
const int pwmResolution = 8;
const int ledPin = 4;
int ledValueOld = 0;

// global camera parameters for REST
uint32_t gain = 0;
uint32_t exposureTime = 0;
uint32_t frameSize = 0;
uint32_t ledintensity = 0;

// SD Card parameters
boolean sdInitialized = false;

// Preferences
Preferences pref;
DevicePreferences device_pref(pref, "camera", __DATE__ " " __TIME__);

void setup()
{
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // prevent brownouts by silencing them

  // INIT SERIAL
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();


  // INIT WIFI
  if (isWebserver) {
    if (isCaptivePortal) {
      autoconnectWifi();
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

  // INIT Camera
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  //init with high specs to pre-allocate larger buffers

  config.frame_size = FRAMESIZE_VGA;
  config.jpeg_quality = 10;
  config.fb_count = 2;

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x % x", err);
    ESP.restart();
    return;
  }

  // INIT SPIFFS
  if (!SPIFFS.begin()) { // SPIFFS must be initialized before the web server, which depends on it
    Serial.println("Couldn't open SPIFFS!");
  }

  // INIT SD
  // We initialize SD_MMC here rather than in setup() because SD_MMC needs to reset the light pin
  // with a different pin mode.
  // 1-bit mode as suggested here:https://dr-mntn.net/2021/02/using-the-sd-card-in-1-bit-mode-on-the-esp32-cam-from-ai-thinker
  if (!SD_MMC.begin("/sdcard", true)) {
    Serial.println("SD Card Mount Failed");
    sdInitialized = false;
  }
  else {
    sdInitialized = true;
    ftpSrv.setCallback(_callback);
    ftpSrv.setTransferCallback(_transferCallback);
    ftpSrv.begin("esp32", "esp32");   //username, password for ftp.   (default 21, 50009 for PASV)
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


  // retrieve old values
  exposureTime = device_pref.getCameraExposureTime();
  gain = device_pref.getCameraGain();
  timelapseInterval = device_pref.getTimelapseInterval();  // Get timelapseInterval
  Serial.print("exposureTime: "); Serial.println(exposureTime );
  Serial.print("gain: "); Serial.println(gain);
  Serial.print("timelapseInterval: "); Serial.println(timelapseInterval);


  // Apply manual settings for the camera
  sensor_t * s = esp_camera_sensor_get();
  s->set_framesize(s, FRAMESIZE_QVGA);
  //s->set_vflip(s, 1);
  //s->set_hmirror(s, 1);
  s->set_exposure_ctrl(s, 0);  // (aec) 0 = disable , 1 = enable
  s->set_aec2(s, 0);           // (aec2) Auto EXP DSP 0 = disable , 1 = enable
  s->set_ae_level(s, 0);       // (ae_level) -2 to 2
  s->set_aec_value(s, exposureTime);    // (aec_value) 0 to 1200
  s->set_gain_ctrl(s, 0);                       // auto gain off
  s->set_brightness(s, 0);                      // -2 to 2
  s->set_special_effect(s, 2);                  //mono
  s->set_wb_mode(s, 0);
  s->set_awb_gain(s, 0);
  s->set_lenc(s, 1);
  s->set_agc_gain(s, gain);  // 0 to 30

  // INIT Webserver
  if (isWebserver) {
    // Start the camera and command server
    startCameraServer();
  }

  // INIT LED
  ledcSetup(ledChannel, freq, pwmResolution);
  ledcAttachPin(ledPin, ledChannel);
  ledcWrite(ledChannel, 255);
  delay(100);
  ledcWrite(ledChannel, 0);

  // initiliaze timer
  t_old = millis();

}


bool saveImage(String filename) {

  // Stop stream
  isStreaming = false;

  // TODO: We need to stop the stream here!
  if(sdInitialized){ // Do not attempt to save anything to a non-existig SD card
    camera_fb_t * frameBuffer = NULL;

    ledcWrite(ledChannel, 255);
    // Take Picture with Camera, but first let camera warm up
    for (int iwarmup = 0; iwarmup < 3; iwarmup++) {
      frameBuffer = esp_camera_fb_get();
      esp_camera_fb_return(frameBuffer);
    }
    frameBuffer = esp_camera_fb_get();

    if (!frameBuffer) {
      Serial.println("Camera capture failed");
      return false;
    }
    ledcWrite(ledChannel, ledValueOld);

    fs::FS &fs = SD_MMC;
    File imgFile = fs.open(filename.c_str(), FILE_WRITE);
    if (!imgFile) {
      Serial.println("Failed to open file in writing mode");
      return false;
    } else {
      imgFile.write(frameBuffer->buf, frameBuffer->len);
      Serial.println("Saved " + filename);

    }
    imgFile.close();
    esp_camera_fb_return(frameBuffer);
    delay(500);
    return true;
  }
  else{
    return false;
  }

  
}


void loop() {
  if (isWebserver) {
    ftpSrv.handleFTP();
  }

  // Perform timelapse imaging
  if (timelapseInterval>0 and isTimelapse and ((millis() - t_old) > (1000 * timelapseInterval))) {
    //https://stackoverflow.com/questions/67090640/errors-while-interacting-with-microsd-card
    t_old = millis();

    uint32_t frame_index = device_pref.getFrameIndex() + 1;
    // Acquire the image and save

    if (saveImage("/picture" + String(frame_index) + ".jpg")) {
      device_pref.setFrameIndex(frame_index);
    };

  }
}
