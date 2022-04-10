#include <ArduinoWebsockets.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include "SPIFFS.h"
#include "esp_camera.h"

// Access point WiFi mode, comment it if you need to wifi client mode
#define AP


// Change this to your network SSID
const char *ssid = "ESP32";
const char *password = "88888888";

camera_fb_t *fb = NULL;

using namespace websockets;
WebsocketsServer websocketserver;
AsyncWebServer webserver(80);



void configureWiFi() {
  // WiFi Access point
  #ifdef AP
    WiFi.softAP(ssid);
    Serial.print("AP IP Address: ");
    Serial.println(WiFi.softAPIP());
  #endif

  // WiFi station
  #ifndef AP
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    // Waiting for connection to wifi
    while (WiFi.status() != WL_CONNECTED) {
      Serial.print(".");
      delay(1000);
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  #endif
}

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

void configureWebSerwer() {
  // Route default path
  webserver.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    Serial.println("Requesting index page...");
    request->send(SPIFFS, "/index.html", "text/html", false);
  });

  // Route to load custom.css file
  webserver.on("/css/custom.css", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/css/custom.css", "text/css"); });

  // Route to load custom.js file
  webserver.on("/js/custom.js", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/js/custom.js", "text/javascript"); });

  // Route to load custom.js file
  webserver.on("/js/joy.js", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/js/joy.js", "text/javascript"); });

  webserver.begin();
  websocketserver.listen(82);
  Serial.print("Is server live? ");
  Serial.println(websocketserver.available());
}

void setup() {
  Serial.begin(115200);
  Serial.println("Setup");

  pinMode(4,OUTPUT);
  digitalWrite(4,LOW);

  configureCamera();

  configureWiFi();

  // Initialize SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  // Configure webserver
  configureWebSerwer();
}

void handle_message(WebsocketsMessage msg) {
  int commaIndex = msg.data().indexOf(',');
  int steerValue = msg.data().substring(0, commaIndex).toInt();
  int forwardValue = msg.data().substring(commaIndex + 1).toInt();

  Serial.println(commaIndex);
  Serial.println(steerValue);
  Serial.println(forwardValue);


    digitalWrite(4,forwardValue);
  
}

int counter = 0;
int digit = 1;

void loop() {
  // Mjpeg camera stream
  auto client = websocketserver.accept();
  client.onMessage(handle_message);

  while (client.available()) {
    client.poll();
    fb = esp_camera_fb_get();
    client.sendBinary((const char *)fb->buf, fb->len);
    esp_camera_fb_return(fb);
    fb = NULL;
  }
}
