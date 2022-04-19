#include "webserver.h"

// Arduino third-party libraries
#include <esp32cam.h>

// FIXME: these should be hidden inside in an encapsulated camera class, maybe?
const static auto loRes = esp32cam::Resolution::find(320, 240);
const static auto hiRes = esp32cam::Resolution::find(800, 600);
const static auto maxRes = esp32cam::Resolution::find(1600, 1200);

void RefocusingServer::init() {
  Serial.print("http://");
  Serial.println(WiFi.localIP());

  addHandler("/", std::bind(&RefocusingServer::handleIndex, this));
  addHandler("/index.html", std::bind(&RefocusingServer::handleIndex, this));
  // Mode selection
  addHandler("/enable", std::bind(&RefocusingServer::handleEnable, this));
  // Camera
  addHandler("/cam.bmp", std::bind(&RefocusingServer::handleBmp, this));
  addHandler("/cam-lo.jpg", std::bind(&RefocusingServer::handleJpgLo, this));
  addHandler("/cam-hi.jpg", std::bind(&RefocusingServer::handleJpgHi, this));
  addHandler("/cam.jpg", std::bind(&RefocusingServer::handleJpg, this));
  addHandler("/cam.mjpg", std::bind(&RefocusingServer::handleMjpeg, this));
  // Assets
  addHandler("/all.css", std::bind(&RefocusingServer::handleBootstrapAll, this));
  addHandler("/bootstrap.min.css", std::bind(&RefocusingServer::handleBootstrapMin, this));

  Serial.println("START SERVER");
  server.begin();
}

void RefocusingServer::serve() {
  server.handleClient();
}

// Route handlers

void RefocusingServer::handleIndex() {
  Serial.println("Handle index.html");
  File file = fs.open("/index.html", "r");
  server.streamFile(file, "text/html");
  file.close();
  return;
}

void RefocusingServer::handleBootstrapMin() {
  Serial.println("Handle bootstrap.min.css");
  File file = fs.open("/bootstrap.min.css", "r");
  server.streamFile(file, "text/css");
  file.close();
  return;
}

void RefocusingServer::handleBootstrapAll() {
  Serial.println("Handle all.css");
  File file = fs.open("/all.css", "r");
  server.streamFile(file, "text/css");
  file.close();
  return;
}

void RefocusingServer::handleBmp() {
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

void RefocusingServer::handleJpgLo() {
  for (int i = 0; i < 3; i++) {
    if (!esp32cam::Camera.changeResolution(loRes)) {
      Serial.println("SET-LO-RES FAIL");
    }
  }
  serveJpg();
}

void RefocusingServer::handleJpgHi() {
  for (int i = 0; i < 3; i++) {
    if (!esp32cam::Camera.changeResolution(maxRes)) {
      Serial.println("SET-HI-RES FAIL");
    }
  }
  serveJpg();
}

void RefocusingServer::handleJpg() {
  server.sendHeader("Location", "/cam-hi.jpg");
  server.send(302, "", "");
}

void RefocusingServer::handleMjpeg() {
  // let the camera "warm up" 
  for (int i = 0; i < 3; i++) {
    if (!esp32cam::Camera.changeResolution(maxRes)) {
      Serial.println("SET-HI-RES FAIL");
    }
  }

  // switch on LED for streaming
  light.on();


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
  light.off();
}

void RefocusingServer::handleEnable() {
  Serial.println("ENABLING!");
  on_enable();
  // FIXME: test to see if we can make everything work without modifying the is_timelapse global variable
  server.send(200, "text/html", "You logged yourself out. The camera will now start capturing images every N-seconds forever. To reactivate Wifi, you have to reflash the code or press the button..");
  Serial.println("Sent response");
}

// Utilities

void RefocusingServer::addHandler(const char *uri, WebServer::THandlerFunction fn) {
  server.on(uri, fn);
  Serial.println(uri);
}

void RefocusingServer::serveJpg() {
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
