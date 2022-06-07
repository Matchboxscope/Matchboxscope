/*
  ESP32 Matchboxscoe Camera Softare
*/

//http://192.168.2.168/control?var=flash&val=100

#include "esp_wifi.h"
#include "esp_camera.h"
#include <WiFi.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include <SPIFFS.h>
#include "camera.h"

// WIFI
boolean hostWifiAP = true; // set this variable if you want the ESP32 to be the host
const char* mSSID = "Blynk";
const char* mPASSWORD = "12345678";
const char* mSSIDAP = "Matchboxscope";

// LED
const int freq = 5000;
const int ledChannel = 7;
const int pwmResolution = 8;
const int ledPin = 4;




void startCameraServer();





void setup()
{
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // prevent brownouts by silencing them

  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

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
    Serial.printf("Camera init failed with error 0x%x", err);
    ESP.restart();
    return;
  }

  // INIT SPIFFS
    if (!SPIFFS.begin()) { // SPIFFS must be initialized before the web server, which depends on it
    Serial.println("Couldn't open SPIFFS!");
  }


  //drop down frame size for higher initial frame rate
  sensor_t * s = esp_camera_sensor_get();
  s->set_framesize(s, FRAMESIZE_QVGA);
  s->set_vflip(s, 1);
  s->set_hmirror(s, 1);



  // INIT WIFI
  if (hostWifiAP)
    initWifiAP(mSSIDAP);
  else
    joinWifi(mSSID, mPASSWORD);




  startCameraServer();

  // INIT LED
  ledcSetup(ledChannel, freq, pwmResolution);
  ledcAttachPin(ledPin, ledChannel);
  ledcWrite(ledChannel, 255);
  delay(100);
  ledcWrite(ledChannel, 0);

}

void loop() {
  // put your main code here, to run repeatedly:
  delay(100);
  //Serial.printf("RSSi: %ld dBm\n",WiFi.RSSI());
}
