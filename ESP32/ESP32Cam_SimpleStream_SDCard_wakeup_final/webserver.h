#pragma once

// C++ standard libraries
#include <functional>

// ESP32 vendor libraries
#include <WebServer.h>
#include <FS.h>

// Local header file
#include "camera.h"
#include "illumination.h"

class RefocusingServer {
  public:
    explicit RefocusingServer(int port, FS &fs, Camera &camera, std::function<void(void)> on_enable) :
      server(port), fs(fs), camera(camera), on_enable(on_enable) {}

    void init();
    void serve();

  private:
    WebServer server;
    FS &fs;
    Camera &camera;
    std::function<void(void)> on_enable;

    // Route handlers
    void handleIndex();
    void handleBootstrapMin();
    void handleBootstrapAll();
    void handleBmp();
    void handleJpgLo();
    void handleJpgHi();
    void handleJpg();
    void handleMjpeg();
    void handleEnable();

    // Utilities
    void addHandler(const char *uri, WebServer::THandlerFunction fn);
    void serveJpg();
};
