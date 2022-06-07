/*
ESP32-CAM Remote Control 
*/

const char* ssid = "Blynk";
const char* password = "Your password";

#include "esp_wifi.h"
#include "esp_camera.h"
#include <WiFi.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

void startCameraServer();

const int MotPin0 = 12;  
const int MotPin1 = 13;  
const int MotPin2 = 14;  
const int MotPin3 = 15;  




void configureCamera() {
  // Camera module pinout configuration
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = 5; // Y2_GPIO_NUM
  config.pin_d1 = 18; // Y3_GPIO_NUM
  config.pin_d2 = 19; // Y4_GPIO_NUM
  config.pin_d3 = 21; // Y5_GPIO_NUM
  config.pin_d4 = 36; // Y6_GPIO_NUM
  config.pin_d5 = 39; // Y7_GPIO_NUM
  config.pin_d6 = 34; // Y8_GPIO_NUM
  config.pin_d7 = 35; // Y9_GPIO_NUM
  config.pin_xclk = 0; // XCLK_GPIO_NUM
  config.pin_pclk = 22; // PCLK_GPIO_NUM
  config.pin_vsync = 25; // VSYNC_GPIO_NUM
  config.pin_href = 23; // HREF_GPIO_NUM
  config.pin_sscb_sda = 26; // SIOD_GPIO_NUM
  config.pin_sscb_scl = 27; // SIOC_GPIO_NUM
  config.pin_pwdn = 32; // PWDN_GPIO_NUM
  config.pin_reset = -1; // RESET_GPIO_NUM
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  // Init with high specs to pre-allocate larger buffers
  if (psramFound()) {
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  sensor_t *s = esp_camera_sensor_get();
  s->set_framesize(s, FRAMESIZE_SVGA);
}

void setup() 
{
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // prevent brownouts by silencing them
  
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  configureCamera();
  
  //drop down frame size for higher initial frame rate
  sensor_t * s = esp_camera_sensor_get();
  // s->set_framesize(s, FRAMESIZE_QVGA);
  // s->set_vflip(s, 1);
  // s->set_hmirror(s, 1);
  // s->set_framesize(s, FRAMESIZE_QVGA);
  camera_fb_t * fb = NULL;
  fb = esp_camera_fb_get();
  Serial.println("Frame width:");
  //display_image(fb->width, fb->height, fb->pixformat, fb->buf, fb->len);
  Serial.println(fb->width);
  //return the frame buffer back to be reused
  esp_camera_fb_return(fb);

  
  ledcSetup(7, 5000, 8);
  ledcAttachPin(4, 7);  //pin4 is LED
  
  Serial.println("ssid: " + (String)ssid);
  Serial.println("password: " + (String)password);

  WiFi.softAP(ssid);
  delay(500);

  long int StartTime=millis();

  /*
  int8_t power;
  esp_wifi_set_max_tx_power(20);
  esp_wifi_get_max_tx_power(&power);
  Serial.printf("wifi power: %d \n",power); 
  */

  fb = NULL;
  fb = esp_camera_fb_get();
  Serial.println("Frame width 2:");
  //display_image(fb->width, fb->height, fb->pixformat, fb->buf, fb->len);
  Serial.println(fb->width);
  //return the frame buffer back to be reused
  esp_camera_fb_return(fb);

  
  startCameraServer();

  for (int i=0;i<5;i++) 
  {
    ledcWrite(7,10);  // flash led
    delay(200);
    ledcWrite(7,0);
    delay(200);    
  }       

  fb = NULL;
  fb = esp_camera_fb_get();
  Serial.println("Frame width 3:");
  //display_image(fb->width, fb->height, fb->pixformat, fb->buf, fb->len);
  Serial.println(fb->width);
  //return the frame buffer back to be reused
  esp_camera_fb_return(fb);
  
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(1000);
  //Serial.printf("RSSi: %ld dBm\n",WiFi.RSSI()); 
}
