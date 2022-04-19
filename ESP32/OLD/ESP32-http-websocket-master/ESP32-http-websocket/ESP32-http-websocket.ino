#include <ArduinoJson.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include "SPIFFS.h"

/* =============== config section start =============== */

#define ENABLE_TFT 0  // tested on TTGO T Display
int ledPin = 2;
String ledState="OFF";

#define HTTP_PORT 8000

const int BUTTON_PIN = 35;
const int LED_PIN = 17;

const char* hostName = "esp32-arduino"; // is default anyway..

// WiFi credentials
#define NUM_NETWORKS 2
// Add your networks credentials here
const char* ssidTab[NUM_NETWORKS] = {
    "Blynk",
    "ATTNVezUEM"
};
const char* passwordTab[NUM_NETWORKS] = {
    "12345678",
    "t8bfmze5a#id"
};


#define LOG(f_, ...) \
  { Serial.printf((f_), ##__VA_ARGS__); }


// you can provide credentials to multiple WiFi networks
WiFiMulti wifiMulti;
IPAddress myip;

// https://github.com/me-no-dev/ESPAsyncWebServer/issues/324 - sometimes
AsyncWebServer server(HTTP_PORT);
AsyncWebSocket ws("/ws");

StaticJsonDocument<200> jsonDocTx;
StaticJsonDocument<100> jsonDocRx;

//extern const char index_html_start[] asm("_binary_src_index_html_start");
//const String html = String((const char*)index_html_start);

bool wsconnected = false;

void notFound(AsyncWebServerRequest* request) {
  request->send(404, "text/plain", "Not found");
}

void onWsEvent(AsyncWebSocket* server, AsyncWebSocketClient* client,
               AwsEventType type, void* arg, uint8_t* data, size_t len) {
  if (type == WS_EVT_CONNECT) {
    wsconnected = true;
    LOG("ws[%s][%u] connect\n", server->url(), client->id());
    // client->printf("Hello Client %u :)", client->id());
    client->ping();
  } else if (type == WS_EVT_DISCONNECT) {
    wsconnected = false;
    LOG("ws[%s][%u] disconnect\n", server->url(), client->id());
  } else if (type == WS_EVT_ERROR) {
    LOG("ws[%s][%u] error(%u): %s\n", server->url(), client->id(),
        *((uint16_t*)arg), (char*)data);
  } else if (type == WS_EVT_PONG) {
    LOG("ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(), len,
        (len) ? (char*)data : "");
  } else if (type == WS_EVT_DATA) {
    AwsFrameInfo* info = (AwsFrameInfo*)arg;
    String msg = "";
    if (info->final && info->index == 0 && info->len == len) {
      // the whole message is in a single frame and we got all of it's data
      LOG("ws[%s][%u] %s-msg[%llu]\r\n", server->url(), client->id(),
          (info->opcode == WS_TEXT) ? "txt" : "bin", info->len);

      if (info->opcode == WS_TEXT) {
        for (size_t i = 0; i < info->len; i++) {
          msg += (char)data[i];
        }
        LOG("%s\r\n\r\n", msg.c_str());

        deserializeJson(jsonDocRx, msg);

        uint8_t ledState = jsonDocRx["led"];
        if (ledState == 1) {
          digitalWrite(LED_PIN, HIGH);
        }
        if (ledState == 0) {
          digitalWrite(LED_PIN, LOW);
        }
        jsonDocRx.clear();
      }
    }
  }
}

void taskWifi(void* parameter);
void taskStatus(void* parameter);

void setup() {
  Serial.begin(115200);

  // Initialize SPIFFS
  if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  xTaskCreate(taskWifi,   /* Task function. */
              "taskWifi", /* String with name of task. */
              20000,      /* Stack size in bytes. */
              NULL,       /* Parameter passed as input of the task */
              2,          /* Priority of the task. */
              NULL);      /* Task handle. */
}

void taskWifi(void* parameter) {
  uint8_t stat = WL_DISCONNECTED;
  static char output[200];
  int cnt = 0;
  int lastButtonState = digitalRead(BUTTON_PIN);
  TickType_t xLastWakeTime = xTaskGetTickCount();

  /* Configure Wi-Fi */
  for (int i = 0; i < NUM_NETWORKS; i++) {
    wifiMulti.addAP(ssidTab[i], passwordTab[i]);
    Serial.printf("WiFi %d: SSID: \"%s\" ; PASS: \"%s\"\r\n", i, ssidTab[i],
                  passwordTab[i]);
  }

  while (stat != WL_CONNECTED) {
    stat = wifiMulti.run();
    Serial.printf("WiFi status: %d\r\n", (int)stat);
    delay(100);
  }

  Serial.print("WiFi connected");
  Serial.println(stat);
  Serial.printf("IP address: ");
  Serial.println(myip = WiFi.localIP());
  
  /* Start web server and web socket server */
/*  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(200, "text/html", html);
  });*/
  server.onNotFound(notFound);


  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });
  
  // Route to load style.css file
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/style.css", "text/css");
  });

  // Route to set GPIO to HIGH
  server.on("/on", HTTP_GET, [](AsyncWebServerRequest *request){
    digitalWrite(ledPin, HIGH);    
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });
  
  // Route to set GPIO to LOW
  server.on("/off", HTTP_GET, [](AsyncWebServerRequest *request){
    digitalWrite(ledPin, LOW);    
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });


  
  ws.onEvent(onWsEvent);
  server.addHandler(&ws);
  server.begin();



  

  LOG("Type in browser:\r\n%s:%d\r\n", hostName, HTTP_PORT);

  while (1) {
    while (WiFi.status() == WL_CONNECTED) {
      cnt++;
      if ((wsconnected == true) &&
          ((lastButtonState != digitalRead(BUTTON_PIN)) || (cnt % 100 == 0))) {
        lastButtonState = digitalRead(BUTTON_PIN);
        jsonDocTx.clear();
        jsonDocTx["counter"] = cnt;
        jsonDocTx["button"] = lastButtonState;

        serializeJson(jsonDocTx, output, 200);

        Serial.printf("Sending: %s", output);
        if (ws.availableForWriteAll()) {
          ws.textAll(output);
          Serial.printf("...done\r\n");
        } else {
          Serial.printf("...queue is full\r\n");
        }
      } else {
        vTaskDelayUntil(&xLastWakeTime, 10 / portTICK_PERIOD_MS);
      }
    }

    stat = wifiMulti.run();
    myip = WiFi.localIP();
    LOG("WiFi status: %d\r\n", (int)stat);
    delay(500);
  }
}

void loop() {
  //Serial.printf("[RAM: %d]\r\n", esp_get_free_heap_size());
  //delay(1000);
  // add ping mechanism in case of websocket connection timeout
  // ws.cleanupClients();
  // ws.pingAll();
}


// Replaces placeholder with LED state value
String processor(const String& var){
  Serial.println(var);
  if(var == "STATE"){
    if(digitalRead(ledPin)){
      ledState = "ON";
    }
    else{
      ledState = "OFF";
    }
    Serial.print(ledState);
    return ledState;
  }
  return String();
}
