//IMPORTANT: ENABLE PSRAM!

#include <esp32cam.h>
#include <WebServer.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include <FS.h>

//const char *SSID = "ATTNVezUEM";
//const char *PWD = "t8bfmze5a#id";


const char *SSID = "Blynk";
const char *PWD = "12345678";
const char *SSID_AP = "MatchboxScope";

WebServer server(80);

static auto loRes = esp32cam::Resolution::find(320, 240);
static auto hiRes = esp32cam::Resolution::find(800, 600);
static auto maxRes = esp32cam::Resolution::find(1600,1200);

void connectToWiFi() {
  Serial.print("Connecting to ");
  Serial.println(SSID);

  /*
   * WiFi.begin(SSID, PWD);
   * while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
    // we can even make the ESP32 to sleep
  }

  Serial.print("Connected. IP: ");
  Serial.println(WiFi.localIP());

   */

  WiFi.softAP(SSID_AP);
    IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
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
  if (!esp32cam::Camera.changeResolution(loRes)) {
    Serial.println("SET-LO-RES FAIL");
  }
  serveJpg();
}

void handleJpgHi()
{
  if (!esp32cam::Camera.changeResolution(maxRes)) {
    Serial.println("SET-HI-RES FAIL");
  }
  serveJpg();
}

void handleJpg()
{
  server.sendHeader("Location", "/cam-hi.jpg");
  server.send(302, "", "");
}

void handleMjpeg()
{
  if (!esp32cam::Camera.changeResolution(hiRes)) {
    Serial.println("SET-HI-RES FAIL");
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

void handleWebpage(){
  Serial.println("Handle Webpage");
  File file = SPIFFS.open("/index.html", "r");  
  size_t sent = server.streamFile(file, "text/html");  
  file.close();  
  return;  
}

void handleStyle(){
  Serial.println("Handle Webpage");
  File file = SPIFFS.open("/style.css", "r");  
  size_t sent = server.streamFile(file, "text/html");  
  file.close();  
  return;  
}

void handleBootstrapMin(){
  Serial.println("Handle bootstrap.min.css");
  File file = SPIFFS.open("/bootstrap.min.css", "r");  
  size_t sent = server.streamFile(file, "text/html");  
  file.close();  
  return;  
}

void handleBootstrapAll(){
  Serial.println("Handle all.css");
  File file = SPIFFS.open("/all.css", "r");  
  size_t sent = server.streamFile(file, "text/html");  
  file.close();
  return;  
}




void setup()
{
  Serial.begin(115200);
  Serial.println();
  
  
 connectToWiFi();

  {
    using namespace esp32cam;
    Config cfg;
    cfg.setPins(pins::AiThinker);
    cfg.setResolution(maxRes);
    cfg.setGrayscale();
    cfg.setBufferCount(2);
    cfg.setJpeg(80);

    bool ok = Camera.begin(cfg);
    Serial.println(ok ? "CAMERA OK" : "CAMERA FAIL");
  }

if(!SPIFFS.begin(true)){
  Serial.println("An Error has occurred while mounting SPIFFS");
  return;
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

  

  server.on("/cam.bmp", handleBmp);
  server.on("/cam-lo.jpg", handleJpgLo);
  server.on("/cam-hi.jpg", handleJpgHi);
  server.on("/cam.jpg", handleJpg);
  server.on("/cam.mjpeg", handleMjpeg);
  server.on("/index.html", handleWebpage);
  server.on("/style.css", handleStyle);
  server.on("/all.css", handleBootstrapAll);
  server.on("/bootstrap.min.css", handleBootstrapMin);
  server.on("/", handleWebpage);

  server.begin();
  Serial.println("START");
}

void loop()
{
  server.handleClient();
}
