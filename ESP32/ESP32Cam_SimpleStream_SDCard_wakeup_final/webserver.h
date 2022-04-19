#pragma once

// C++ standard libraries
#include <functional>

// ESP32 vendor libraries
#include <WebServer.h>
#include <FS.h>

// Local header file
#include "illumination.h"

class RefocusingServer {
  public:
    explicit RefocusingServer(int port, FS &fs, Light &light, std::function<void(void)> on_enable) :
      server(port), fs(fs), light(light), on_enable(on_enable) {}

    void init();
    void serve();

  private:
    WebServer server;
    FS &fs;
    Light &light;
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
