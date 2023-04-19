#%%
import serial
import time
import serial.tools.list_ports
from PIL import Image
import base64
import io
import numpy as np
import matplotlib.pyplot as plt
import tifffile as tif

devices = serial.tools.list_ports.comports(include_links=False)

serialport = devices[-1].device 

# connect to Camera
serialdevice = serial.Serial(port=serialport,baudrate=1000000,timeout=1)

#%%
iError = 0
t0 = time.time()

message = ""
imageString = ""
for iimage in range(100):
  try:
      #read image and decode
      imageB64 = serialdevice.readline()
      #imageB64 = str(imageB64).split("+++++")[-1].split("----")[0]
      image = np.array(Image.open(io.BytesIO(base64.b64decode(imageB64))))
      
      print("framerate: "+(str(1/(time.time()-t0))))
      tif.imsave("test_stack_esp32.tif", image, append=True)
  except Exception as e:
    print(e)
    iError += 1
    
print(iError)

#%%
        
        
''' ESP CODE

#include "esp_camera.h"
#include <base64.h>
#define CAMERA_MODEL_AI_THINKER
#include "camera.h"

#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22


#define BAUD_RATE 2000000

void setup()
{
  Serial.begin(BAUD_RATE);

  cameraInit();
}

void loop() {
  grabImage();
}

void cameraInit() {
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
  config.xclk_freq_hz = 10000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_QVGA; //320x240
  config.jpeg_quality = 10;
  config.fb_count = 2;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {

    return;
  }

  sensor_t * s = esp_camera_sensor_get();
  s->set_hmirror(s, 1);
  s->set_vflip(s, 1);
}

void grabImage() {

  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb || fb->format != PIXFORMAT_JPEG) {
  } else {
    delay(40);
    String encoded = base64::encode(fb->buf, fb->len);
    Serial.write(encoded.c_str(), encoded.length());
    Serial.println();
  }
  esp_camera_fb_return(fb);
}

'''
# %%
