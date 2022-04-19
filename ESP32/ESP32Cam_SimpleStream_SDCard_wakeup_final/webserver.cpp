#include "webserver.h"

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
  if (!camera.useLowRes()) {
    Serial.println("SET-LO-RES FAIL");
  }

  auto frame = camera.acquire();
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
    if (!camera.useLowRes()) {
      Serial.println("SET-LO-RES FAIL");
    }
  }
  serveJpg();
}

void RefocusingServer::handleJpgHi() {
  for (int i = 0; i < 3; i++) {
    if (!camera.useMaxRes()) {
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
    if (!camera.useMaxRes()) {
      Serial.println("SET-HI-RES FAIL");
    }
  }

  WiFiClient client = server.client();
  camera.streamMjpeg(client);
}

void RefocusingServer::handleEnable() {
  Serial.println("ENABLING!");
  on_enable();
  server.send(200, "text/html", "You logged yourself out. The camera will now start capturing images every N-seconds forever. To reactivate Wifi, you have to reflash the code or press the button..");
}

// Utilities

void RefocusingServer::addHandler(const char *uri, WebServer::THandlerFunction fn) {
  server.on(uri, fn);
  // Serial.print("  ");
  // Serial.println(uri);
}

void RefocusingServer::serveJpg() {
  auto frame = camera.acquire();
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
