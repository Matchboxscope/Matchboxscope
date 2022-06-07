// UPLOAD INSTRUCTIONS

// Install v2.0.2 of ESP32 board support for the Arduino. Choose the "ESP32 Dev Module" board.
// Enable the PSRAM settings. Upload the sketch.
// Then perform the ESP32 Data Sketch Upload to upload web server data assets to the ESP32.


// HEADERS

// C++ standard libraries
#include <memory>

// ESP32 vendor libraries
#include <soc/soc.h>           // Disable brownour problems
#include <soc/rtc_cntl_reg.h>  // Disable brownour problems
#include <esp_camera.h>
#include <Preferences.h>
#include <SPIFFS.h>
#include <SD_MMC.h> // SD card
#include <driver/rtc_io.h>
#include "esp_camera.h"

// Arduino core
#include "Arduino.h"

// stepper motor
#include <AccelStepper.h>

// Arduino third-party libraries

// Local header files
#include "device_pref.h"
#include "illumination.h"
//#include "camera.h"
//#include "webserver.h"
#include "wifi.h"

// TODOS

// TODO: Whitebalance/constant intensity
// TODO: Swap esp32cam.h with esp_camera?
// TODO: link /enable to website
// TODO: fit mjpeg into website proerly


// SETTINGS

// Startup
static const uint32_t refocus_button_debounce = 4000; // ms; duration to wait for the refocus button to stabilize before reading
boolean sdInitialized = false;

// Hardware pins
static const gpio_num_t reedSwitchPin = GPIO_NUM_13; // pin of the external switch to "awake" the ESP (e.g. Reedrelay)
static const gpio_num_t lightPin = GPIO_NUM_4; // pin of the imaging illumination LED

// Image acquisition
static const int res_warmup_wait = 500; // ms; how long to wait after changing the camera resolution
static const uint32_t camera_warmup_frames = 5; // number of "warm-up" camera frames to wait before final acquisition
static const int num_frame_buffers = 2; // number of frame buffers to keep, must be at least 1; higher values enable higher streaming fps
static const int jpeg_quality = 80; // JPEG quality factor from 0 (worst) to 100 (best)

// Timelapse
static const uint64_t timelapseInterval = 5; // sec; timelapse interval
static uint64_t t_old = 0;

// Wifi
const char *wifiAPSSID = "MatchboxScope";
const char *wifiSSID = "Blynk"; // the SSID of a wifi network to join, if not hosting a wifi AP
const char *wifiPassword = "12345678"; // the password of a wifi network to join, if not hosting a wifi AP


// GLOBAL STATE
boolean hostWifiAP = false;
// Mode
bool is_timelapse = false; // in timelapse mode, the webserver is not enabled

// Hardware
Light light(lightPin);
/*
Camera camera(
  esp32cam::Camera, esp32cam::pins::AiThinker, light,
  res_warmup_wait, num_frame_buffers, jpeg_quality
);
*/

void startCameraServer();
// Preferences
Preferences pref;
DevicePreferences device_pref(pref, "camera", __DATE__ " " __TIME__);

// Web server
void setTimelapse() {
  is_timelapse = true; // focus has been set
  device_pref.setIsTimelapse(is_timelapse);
}
//RefocusingServer server(80, SPIFFS, camera, setTimelapse);


// INITIALIZATION
bool initCamera(){
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
  config.frame_size = FRAMESIZE_VGA;
  config.jpeg_quality = 10;
  config.fb_count = 2;
  
    
  // camera init
  Serial.print("Camera ok? ");
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    Serial.println("no");
    return false;
  }
  else {
    sensor_t *s = esp_camera_sensor_get();
    s->set_framesize(s, FRAMESIZE_SVGA);
    Serial.println("yes");
    return true;
  }
}


void initNonTimelapseFunctionalities() {
  if (hostWifiAP) {
    initWifiAP(wifiAPSSID);
  } else {
    joinWifi(wifiSSID, wifiPassword);
  }
  if (!SPIFFS.begin()) { // SPIFFS must be initialized before the web server, which depends on it
    Serial.println("Couldn't open SPIFFS!");
  }
 // server.init();
}


void setup() {
  // prevent brownouts by silencing them
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

  Serial.begin(115200);
  Serial.setDebugOutput(true);

  // initialize camera
  initCamera();

  // Reset the EEPROM's stored timelapse mode after each re-flash
  if (device_pref.isFirstRun()) {
    device_pref.setIsTimelapse(false);
  }

  // Initialize the camera
  camera_fb_t * fb = NULL;
  fb = esp_camera_fb_get();
  Serial.println("Frame width:");
  Serial.println(fb->width);
  //return the frame buffer back to be reused
  esp_camera_fb_return(fb);


  Serial.println("In refocusing mode. Connect to Wifi and go to 192.168.4.1/enable once you're done with focusing.");
  initNonTimelapseFunctionalities();

  // initiliaze timer
  t_old = millis();


  // We initialize SD_MMC here rather than in setup() because SD_MMC needs to reset the light pin
  // with a different pin mode.
  // 1-bit mode as suggested here:https://dr-mntn.net/2021/02/using-the-sd-card-in-1-bit-mode-on-the-esp32-cam-from-ai-thinker
  if (!SD_MMC.begin("/sdcard", true)) {
    Serial.println("SD Card Mount Failed");
  }
  else {
    Serial.println("SD Card Mounted");
  }
  sdInitialized = true;


  /*
     By setting the parameter mode1bit to true,
     the ESP32-CAM just uses the GPIO02 to read
     and write data to the SD card, which frees
     up the GPIO pins GPIO04 (HS_DATA1),
     GPIO12 (HS_DATA2), and GPIO13 (HS_DATA3) as
     further GPIO pins for input or output.
     To use one or all of these GPIO pins for input
     or output, you have to define the GPIO pin accordingly
     after the initialization of the SD card. Otherwise, the
     initialization of the SD card would re-configure the
     GPIO pins again. For instance, the following code
     snippet configures the GPIO pin GPIO12 as an output
     pin and sets it to low:
   * */

   
  startCameraServer();

  // initiliaze stepper motor
  int pinStep = 12;
  int pinDir = 13;

  pinMode(lightPin, OUTPUT);
  digitalWrite(lightPin, HIGH);
  Serial.println("Turning motor");

  AccelStepper stepper(1, pinStep, pinDir);

  stepper.setMaxSpeed(10000);
  stepper.setAcceleration(10000);
  stepper.enableOutputs();
  stepper.runToNewPosition(-1000);
  stepper.runToNewPosition(1000);

  digitalWrite(lightPin, LOW);
  Serial.println("Done Turning motor");

  // Check for an SD card
  uint8_t cardType = SD_MMC.cardType();
  if (cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    return;
  }
  else {
    Serial.println(cardType);
  }
}


bool saveImage(String filename) {
    camera_fb_t * frameBuffer = NULL;

    // Take Picture with Camera
    frameBuffer = esp_camera_fb_get();
    if (!frameBuffer) {
        Serial.println("Camera capture failed");
        return false;
    }

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
    delay(5000);
    return true;
}


// MAIN LOOP
void loop() {

  server.serve(); // serve webpage and image stream to adjust focus

  if ((millis() - t_old) > (1000 * timelapseInterval)) {
    //https://stackoverflow.com/questions/67090640/errors-while-interacting-with-microsd-card
    t_old = millis();

    // camera.useMaxRes(); // TODO: update!
         uint32_t frame_index = device_pref.getFrameIndex() + 1;
    // Acquire the image and save
    light.on();
    if (saveImage("/picture" + String(frame_index) + ".jpg")) {
      device_pref.setFrameIndex(frame_index);
    };

    // Save image to SD card

    light.sleep();
  }
}
