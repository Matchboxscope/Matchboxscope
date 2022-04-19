#include "Arduino.h"

// FIXME: we should move these constants out of this file
#ifdef WIFIAP
const char *SSID = "MatchboxScope";
#else
const char *SSID = "YOUR_SSID";
const char *PWD = "YOU_PASSWORD";
#endif

// FIXME: it would be nice to move these global variables to the main file
WebServer server(80);
bool is_timelapse = false; // in timelapse mode, the webserver is not enabled
Preferences pref;
CameraPreferences cam_pref(pref);

// TODO: this should be hidden inside an encapsulated camera class
static const int PIN_LED = 4;

void handleIndex() {
  Serial.println("Handle index.html");
  File file = SPIFFS.open("/index.html", "r");
  server.streamFile(file, "text/html");
  file.close();
  return;
}

void handleBootstrapMin() {
  Serial.println("Handle bootstrap.min.css");
  File file = SPIFFS.open("/bootstrap.min.css", "r");
  server.streamFile(file, "text/css");
  file.close();
  return;
}

void handleBootstrapAll() {
  Serial.println("Handle all.css");
  File file = SPIFFS.open("/all.css", "r");
  server.streamFile(file, "text/css");
  file.close();
  return;
}

// FIXME: these should be hidden inside in an encapsulated camera class, maybe?
const static auto loRes = esp32cam::Resolution::find(320, 240);
const static auto hiRes = esp32cam::Resolution::find(800, 600);

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

// FIXME: these should be hidden inside in an encapsulated camera class, maybe?
const static auto maxRes = esp32cam::Resolution::find(1600, 1200);

void handleJpgHi()
{
  for (int i = 0; i < 3; i++) {
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
{ 
  // let the camera "warm up" 
  for (int i = 0; i < 3; i++) {
    if (!esp32cam::Camera.changeResolution(maxRes)) {
      Serial.println("SET-HI-RES FAIL");
    }
  }

  // switch on LED for streaming
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
}

void enable() {
  Serial.println("ENABLING!");
  // FIXME: this function modifies a global variable declared in the main file! We should decouple it somehow (how???)
  is_timelapse = true; // focus has been set
  cam_pref.setIsTimelapse(is_timelapse);
  server.send(200, "text/html", "You logged yourself out. The camera will now start capturing images every N-seconds forever. To reactivate Wifi, you have to reflash the code or press the button..");
}


void initWebServerConfig()
{
  // register all the endpoints
  char endpoint_index_non[]= "/";
  char endpoint_index[] = "/index.html";
  char endpoint_allcss[] = "/all.css";
  char endpoint_bootstrapcss[] = "/bootstrap.min.css";
  char endpoint_enable[] = "/enable";
  char endpoint_cambmp[] = "/cam.bmp";
  char endpoint_camlo[] = "/cam-lo.jpg";
  char endpoint_camhi[] = "/cam-hi.jpg";
  char endpoint_cam[] = "/cam.jpg";
  char endpoint_cammjpg[] = "/cam.mjpg";
    
  server.on(endpoint_index_non, handleIndex);
  server.on(endpoint_index, handleIndex);
  server.on(endpoint_allcss, handleBootstrapAll);
  server.on(endpoint_bootstrapcss, handleBootstrapMin);

  server.on(endpoint_enable, enable);

  server.on(endpoint_cambmp, handleBmp);
  server.on(endpoint_camlo, handleJpgLo);
  server.on(endpoint_camhi, handleJpgHi);
  server.on(endpoint_cam, handleJpg);
  server.on(endpoint_cammjpg, handleMjpeg);

  Serial.print("http://");
  Serial.println(WiFi.localIP());
  Serial.println(endpoint_index_non);
  Serial.println(endpoint_index);
  Serial.println(endpoint_allcss);
  Serial.println(endpoint_bootstrapcss);
  Serial.println(endpoint_enable);
  Serial.println(endpoint_camlo);
  Serial.println(endpoint_camhi);
  Serial.println(endpoint_cam);
  Serial.println(endpoint_cammjpg);

  Serial.println("START SERVER");
  server.begin();
}
