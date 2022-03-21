//IMPORTANT: ENABLE PSRAM!
#include <esp32cam.h>
#include <WebServer.h>
#include "esp_camera.h"
#include "Arduino.h"
#include "soc/soc.h"           // Disable brownour problems
#include "soc/rtc_cntl_reg.h"  // Disable brownour problems
#include "driver/rtc_io.h"

#include <WiFi.h>
#include <SPIFFS.h>
#include <FS.h>

#include "SD_MMC.h" // SD card      
#include <EEPROM.h> // EEPROM     

#define WIFIAP
#define PIN_LED 4

#ifdef WIFIAP
const char *SSID = "MatchboxScope";
#else
const char *SSID = "ATTNVezUEM";
const char *PWD = "t8bfmze5a#id";
#endif

// define the number of bytes you want to access
#define EEPROM_SIZE 1
unsigned char pictureNumber = 0; // 0..255


WebServer server(80);

static auto loRes = esp32cam::Resolution::find(320, 240);
static auto hiRes = esp32cam::Resolution::find(800, 600);
static auto maxRes = esp32cam::Resolution::find(1600, 1200);


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


static String generate_file_name()
{
  // initialize EEPROM with predefined size
  EEPROM.begin(EEPROM_SIZE);
  pictureNumber = EEPROM.read(0) + 1;

  // Path where new picture will be saved in SD Card
  return "/picture" + String(pictureNumber) + ".jpg";
}

static bool saveImage (String path)
{
  digitalWrite(4,HIGH);
  auto frame = esp32cam::capture();
  for (int i = 0; i < 10; i++) auto frame = esp32cam::capture(); // warm up
  digitalWrite(PIN_LED, LOW);

  fs::FS &fs = SD_MMC;
  Serial.printf("Picture file name: %s\n", path.c_str());

  File file = fs.open(path.c_str(), FILE_WRITE);
  if (!file)
  {
    Serial.println("Failed to open file in writing mode");
    return ESP_ERR_NO_MEM;
  }
  else
  {
    if (frame == nullptr) {
      Serial.println("CAPTURE FAIL");
      return false;
    }
    Serial.printf("CAPTURE OK %dx%d %db\n", frame->getWidth(), frame->getHeight(),
                  static_cast<int>(frame->size()));
    file.write(frame->data(), frame->size()); // payload (image), payload length
    EEPROM.write(0, pictureNumber);
    EEPROM.commit();
    file.close();
    return true;
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
  /*
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");

    asyncserver.on("/", HTTP_GET, [](AsyncWebServerRequest * request)
    {
      Serial.println("get page");
      request->send(SPIFFS, "/index.html", "text/html");
    });

    asyncserver.on("/bootstrap.min.css", HTTP_GET, [](AsyncWebServerRequest * request)
    {
      request->send(SPIFFS, "/bootstrap.min.css", "text/css");
    });

    asyncserver.on("/all.css", HTTP_GET, [](AsyncWebServerRequest * request)
    {
      request->send(SPIFFS, "/all.css", "text/css");
    });
  */
  server.on("/index.html", handleIndex);
  server.on("/all.css", handleBootstrapAll);
  server.on("/bootstrap.min.css", handleBootstrapMin);

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

  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, HIGH);


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
  digitalWrite(PIN_LED, LOW);
  rtc_gpio_hold_en(GPIO_NUM_4);

}


void initSD() {
  Serial.println("Starting SD Card");
  if (!SD_MMC.begin("/sdcard", true))
  {
    Serial.println("SD Card Mount Failed");
    return;
  }

  uint8_t cardType = SD_MMC.cardType();
  if (cardType == CARD_NONE)
  {
    Serial.println("No SD Card attached");
    return;
  }
}

void initCamera() {
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



void setup()
{
  Serial.begin(115200);
  Serial.println();

  connectToWiFi();
  iniSD();

  // setup flash
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);
  delay(100);
  digitalWrite(4, LOW);

  // init camera
  initCamera();
  initSpiffs();

  // Save picture
  String path = generate_file_name();
  if (saveImage(path) == ESP_OK)
  {
    Serial.printf("Saved file to path: %s\n", path.c_str());
  }
  else
  {
    Serial.printf("Failed to save file to path: %s\n", path.c_str());
  }





  Serial.print("http://");
  Serial.println(WiFi.localIP());
  Serial.println("  /cam.bmp");
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

void loop()
{
  server.handleClient();
}
