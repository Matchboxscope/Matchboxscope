#include "camera.h"

bool Camera::save(std::unique_ptr<esp32cam::Frame> frame, const char *filename, FS &fs) {
  Serial.print("Saving to: ");
  Serial.println(filename);
  File file = fs.open(filename, FILE_WRITE);
  Serial.println("Opening file");
  if (!file) {
    Serial.println("Failed to open file in writing mode");
    file.close();
    return false;
  }

  file.write(frame->data(), frame->size()); // payload (image), payload length
  Serial.println("Finished writing file.");
  file.close();
  return true;
}

bool Camera::init() {
  esp32cam::Config cfg;
  cfg.setPins(pins);
  cfg.setResolution(maxRes());
  cfg.setBufferCount(num_frame_buffers);
  cfg.setJpeg(jpeg_quality);

  Serial.print("Camera ok? ");
  if (!camera.begin(cfg)) {
    Serial.println("no");
    return false;
  }
  Serial.println("yes");
  return true;
}

bool Camera::useMaxRes() {
  bool success = camera.changeResolution(Camera::maxRes(), warmup_wait);
  if (!success) {
    Serial.println("SET-MAX-RES FAIL");
  }
  return success;
}

bool Camera::useHighRes() {
  bool success = camera.changeResolution(Camera::highRes(), warmup_wait);
  if (!success) {
    Serial.println("SET-HIGH-RES FAIL");
  }
  return success;
}

bool Camera::useLowRes() {
  bool success = camera.changeResolution(Camera::lowRes(), warmup_wait);
  if (!success) {
    Serial.println("SET-LOW-RES FAIL");
  }
  return success;
}

std::unique_ptr<esp32cam::Frame> Camera::acquire(uint32_t warmup_frames) {
  light.on();
  auto frame = camera.capture();
  for (uint32_t i = 0; i < warmup_frames; ++i) {
    frame = camera.capture();
  }
  light.off();
  if (frame == nullptr) {
    Serial.println("CAPTURE FAIL");
    return frame;
  }

  Serial.printf(
      "CAPTURE OK %dx%d %db\n",
      frame->getWidth(), frame->getHeight(), static_cast<int>(frame->size())
  );
  return frame;
}

void Camera::streamMjpeg(Client &client) {
  light.on();
  Serial.println("STREAM BEGIN");
  auto startTime = millis();
  auto streamed_count = camera.streamMjpeg(client);
  light.off();
  if (streamed_count <= 0) {
    Serial.printf("STREAM ERROR %d\n", streamed_count);
    return;
  }
  auto elapsed = millis() - startTime;
  Serial.printf("STREAM END %dfrm %0.2ffps\n", streamed_count, 1000.0 * streamed_count / elapsed);
}

esp32cam::Resolution Camera::maxRes() {
  return esp32cam::Resolution::find(1600, 1200);
}

esp32cam::Resolution Camera::highRes() {
  return esp32cam::Resolution::find(800, 600);
}

esp32cam::Resolution Camera::lowRes() {
  return esp32cam::Resolution::find(320, 240);
}
