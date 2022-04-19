//IMPORTANT: ENABLE PSRAM!
#include <esp32cam.h>
#include <WebServer.h>
#include "ArduinoJson.h"
#include "internal/mjpeg.hpp"
#include "internal/config.hpp"

#include <WiFi.h>
#include <SPIFFS.h>
#include <FS.h>

#define FLASH_PIN 4

#define WIFIAP

#ifdef WIFIAP
const char *SSID = "MatchboxScope";
#else
const char *SSID = "ATTNVezUEM";
const char *PWD = "t8bfmze5a#id";
#endif

WebServer server(80);

static auto loRes = esp32cam::Resolution::find(320, 240);
static auto hiRes = esp32cam::Resolution::find(800, 600);
static auto maxRes = esp32cam::Resolution::find(1600, 1200);


StaticJsonDocument<250> jsonDocument;
char buffer[250];


void initSpiffs()
{
  if (!SPIFFS.begin()) /* Démarrage du gestionnaire de fichiers SPIFFS */
  {
    Serial.println("Erreur SPIFFS...");
    return;
  }

  /* Détection des fichiers présents sur l'Esp32 */
  File root = SPIFFS.open("/");    /* Ouverture de la racine */
  File file = root.openNextFile(); /* Ouverture du 1er fichier */
  while (file)                     /* Boucle de test de présence des fichiers - Si plus de fichiers la boucle s'arrête*/

  {
    Serial.print("File: ");
    Serial.println(file.name());
    file.close();
    file = root.openNextFile(); /* Lecture du fichier suivant */
  }
}

void handleIndex() {
  Serial.println("Handle index.html");
  File file = SPIFFS.open("/index.html", "r");
  size_t sent = server.streamFile(file, "text/html");
  file.close();
  return;
}


void handleBootstrapMin() {
  Serial.println("Handle bootstrap.min.css");
  File file = SPIFFS.open("/bootstrap.min.css", "r");
  size_t sent = server.streamFile(file, "text/css");
  file.close();
  return;
}

void handleBootstrapAll() {
  Serial.println("Handle all.css");
  File file = SPIFFS.open("/all.css", "r");
  size_t sent = server.streamFile(file, "text/css");
  file.close();
  return;
}





void initWebServerConfig()
{
  server.on("/", handleIndex);
  server.on("/index.html", handleIndex);
  server.on("/all.css", handleBootstrapAll);
  server.on("/bootstrap.min.css", handleBootstrapMin);

  server.on("/led", HTTP_POST, handleLED);

  server.on("/cam.bmp", handleBmp);
  server.on("/cam-lo.jpg", handleJpgLo);
  server.on("/cam-hi.jpg", handleJpgHi);
  server.on("/cam.jpg", handleJpg);
  server.on("/cam.mjpg", handleMjpeg);

  server.begin();
}


void connectToWiFi() {
#ifdef WIFIAP
  Serial.print("Network SSID: ");
  Serial.println(SSID);

  WiFi.softAP(SSID);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

#else
  Serial.print("Connecting to ");
  Serial.println(SSID);

  WiFi.begin(SSID, PWD);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
    // we can even make the ESP32 to sleep
  }

  Serial.print("Connected. IP: ");
  Serial.println(WiFi.localIP());
#endif
}


void handleBmp()
{
  if (!esp32cam::Camera.changeResolution(loRes)) {
    Serial.println("SET-LO-RES FAIL");
  }

  auto frame = esp32cam::capture();
  if (frame == nullptr) {
    Serial.println("CAPTURE FAIL");
    server.send(503, "", "");
    return;
  }
  Serial.printf("CAPTURE OK %dx%d %db\n", frame->getWidth(), frame->getHeight(),
                static_cast<int>(frame->size()));

  if (!frame->toBmp()) {
    Serial.println("CONVERT FAIL");
    server.send(503, "", "");
    return;
  }
  Serial.printf("CONVERT OK %dx%d %db\n", frame->getWidth(), frame->getHeight(),
                static_cast<int>(frame->size()));

  server.setContentLength(frame->size());
  server.send(200, "image/bmp");
  WiFiClient client = server.client();
  frame->writeTo(client);
}



void serveJpg()
{
  auto frame = esp32cam::capture();
  if (frame == nullptr) {
    Serial.println("CAPTURE FAIL");
    server.send(503, "", "");
    return;
  }
  Serial.printf("CAPTURE OK %dx%d %db\n", frame->getWidth(), frame->getHeight(),
                static_cast<int>(frame->size()));

  server.setContentLength(frame->size());
  server.send(200, "image/jpeg");
  WiFiClient client = server.client();
  frame->writeTo(client);
}

void handleJpgLo()
{
  for (int i = 0; i < 3; i++) {
    if (!esp32cam::Camera.changeResolution(loRes)) {
      Serial.println("SET-LO-RES FAIL");
    }
  }
  serveJpg();
}

void handleJpgHi()
{ for (int i = 0; i < 3; i++) {
    if (!esp32cam::Camera.changeResolution(maxRes)) {
      Serial.println("SET-HI-RES FAIL");
    }
  }
  serveJpg();
}

void handleJpg()
{
  server.sendHeader("Location", "/cam-hi.jpg");
  server.send(302, "", "");
}

void handleMjpeg()
{ for (int i = 0; i < 3; i++) {
    if (!esp32cam::Camera.changeResolution(hiRes)) {
      Serial.println("SET-HI-RES FAIL");
    }
  }

  Serial.println("STREAM BEGIN");
  WiFiClient client = server.client();
  auto startTime = millis();
  int res = esp32cam::Camera.streamMjpeg(client);
  if (res <= 0) {
    Serial.printf("STREAM ERROR %d\n", res);
    return;
  }
  auto duration = millis() - startTime;
  Serial.printf("STREAM END %dfrm %0.2ffps\n", res, 1000.0 * res / duration);
}

void handleLED() {

  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);

  bool ledstate = jsonDocument["ledstate"];

  digitalWrite(FLASH_PIN, ledstate);

  server.sendHeader("Location", "/cam-hi.jpg");
  server.send(200, "application/json", "{}");
}




void setup()
{
  Serial.begin(115200);
  Serial.println();

  // switch on / off FLASH light
  pinMode(FLASH_PIN, OUTPUT);
  digitalWrite(FLASH_PIN, HIGH);

  connectToWiFi();

  {
    using namespace esp32cam;
    Config cfg;
    cfg.setPins(pins::AiThinker);
    cfg.setResolution(maxRes);
    //cfg.setGrayscale();
    cfg.setBufferCount(2);
    cfg.setJpeg(80);

    bool ok = Camera.begin(cfg);
    Serial.println(ok ? "CAMERA OK" : "CAMERA FAIL");
  }

  initSpiffs();


  Serial.print("http://");
  Serial.println(WiFi.localIP());
  Serial.println("  /cam.bmp");
  Serial.println("  /led");
  Serial.println("  /cam-lo.jpg");
  Serial.println("  /cam-hi.jpg");
  Serial.println("  /cam.mjpeg");
  Serial.println("  /index.html");
  Serial.println("  /style.css");

  Serial.println("  /all.css");
  Serial.println("  /BootstrapMin.css");
  initWebServerConfig();

  Serial.println("START SERVER");
}

bool is_stream = false;


struct MjpegConfig
{
  /** @brief minimum interval between frame captures. */
  int minInterval = 0;
  /** @brief maximum number of frames before disconnecting. */
  int maxFrames = -1;
  /** @brief time limit of writing one frame in millis. */
  int frameTimeout = 10000;
};

/*
MjpegConfig& cfg{
  0,
  -1,
  10000
};
*/

void loop()
{
/*
  WiFiClient client = server.client();
  esp32cam::detail::MjpegHeader hdr;
  hdr.prepareResponseHeaders();
  hdr.writeTo(client);

  using Ctrl = esp32cam::detail::MjpegController;
  Ctrl ctrl(cfg);
  
    auto act = ctrl.decideAction();
    switch (act) {
      case Ctrl::CAPTURE: {
        ctrl.notifyCapture();
        break;
      }
      case Ctrl::RETURN: {
        ctrl.notifyReturn(capture());
        break;
      }
      case Ctrl::SEND: {
        hdr.preparePartHeader(ctrl.getFrame()->size());
        hdr.writeTo(client);
        ctrl.notifySent(ctrl.getFrame()->writeTo(client, cfg.frameTimeout));
        hdr.preparePartTrailer();
        hdr.writeTo(client);
        break;
      }
      case Ctrl::STOP: {
        client.stop();
        return ctrl.countSentFrames();
      }
      default: {
        delay(act);
        break;
      }
    }*/
  
  
  server.handleClient();
}
