#pragma once

// ESP32 vendor libraries
#include <Client.h>
#include <FS.h>

// C++ standard libraries
#include <memory>

// Arduino third-party libraries
#include <esp32cam.h>

// Local header files
#include "illumination.h"

class Camera {
  public:
    Camera(
        esp32cam::CameraClass &camera, esp32cam::Pins pins, Light &light,
        int res_warmup_wait, int num_frame_buffers, int jpeg_quality
    ) :
      camera(camera),
      pins(pins),
      light(light),
      warmup_wait(res_warmup_wait),
      num_frame_buffers(num_frame_buffers),
      jpeg_quality(jpeg_quality) {}

    static bool save(std::unique_ptr<esp32cam::Frame> frame, const char *filename, FS &fs);

    bool init();

    bool useMaxRes();
    bool useHighRes();
    bool useLowRes();

    std::unique_ptr<esp32cam::Frame> acquire(uint32_t warmup_frames = 0);
    void streamMjpeg(Client &client);

  private:
    esp32cam::CameraClass &camera;
    esp32cam::Pins pins;
    Light &light;
    int warmup_wait;
    int num_frame_buffers;
    int jpeg_quality;

    static esp32cam::Resolution maxRes();
    static esp32cam::Resolution highRes();
    static esp32cam::Resolution lowRes();
};
