void initCamera() {
  // INIT Camera
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  //init with high specs to pre-allocate larger buffers

  config.frame_size = FRAMESIZE_VGA;
  config.jpeg_quality = 10;
  config.fb_count = 2;

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x % x", err);
    ESP.restart();
    return;
  }
}


void initCameraSettings() {
  // Apply manual settings for the camera
  sensor_t * s = esp_camera_sensor_get();
  s->set_framesize(s, FRAMESIZE_QVGA);
  //s->set_vflip(s, 1);
  //s->set_hmirror(s, 1);
  s->set_exposure_ctrl(s, 0);  // (aec) 0 = disable , 1 = enable
  s->set_aec2(s, 0);           // (aec2) Auto EXP DSP 0 = disable , 1 = enable
  s->set_ae_level(s, 0);       // (ae_level) -2 to 2
  s->set_aec_value(s, exposureTime);    // (aec_value) 0 to 1200
  s->set_gain_ctrl(s, 0);                       // auto gain off
  s->set_brightness(s, 0);                      // -2 to 2
  s->set_special_effect(s, device_pref.getCameraEffect()); //mono - has to be set again?
  s->set_wb_mode(s, 0);
  s->set_awb_gain(s, 0);
  s->set_lenc(s, 1);
  s->set_agc_gain(s, gain);  // 0 to 30
}


bool saveImage(String filename) {

  // Stop stream
  isStreaming = false;

  // TODO: We need to stop the stream here!
  if (sdInitialized) { // Do not attempt to save anything to a non-existig SD card
    camera_fb_t * frameBuffer = NULL;

    // turn on LED
    ledcWrite(ledChannel, 255);

    // set maximum framesize
    sensor_t * s = esp_camera_sensor_get();
    setFrameSize(device_pref.getCameraFramesize()); // TODO: Why does it change the exposure time/brightness??! 
    s->set_aec_value(s, device_pref.getCameraExposureTime());    // (aec_value) 0 to 1200
    s->set_special_effect(s, device_pref.getCameraEffect()); //mono - has to be set again?
    s->set_agc_gain(s, device_pref.getCameraGain());  // 0 to 30


    // Take Picture with Camera, but first let camera warm up => make sure low res frame in buffer is freed
    for (int iwarmup = 0; iwarmup < 3; iwarmup++) {
      frameBuffer = esp_camera_fb_get();
      esp_camera_fb_return(frameBuffer);
    }
    frameBuffer = esp_camera_fb_get();

    if (!frameBuffer) {
      Serial.println("Camera capture failed");
      ESP.restart();
      return false;
    }
    ledcWrite(ledChannel, ledValueOld);

    // Save image to disk
    fs::FS &fs = SD_MMC;
    File imgFile = fs.open(filename.c_str(), FILE_WRITE);
    if (!imgFile) {
      Serial.println("Failed to open file in writing mode");
      return false;
    } else {
      imgFile.write(frameBuffer->buf, frameBuffer->len);
      Serial.println("Saved " + filename);
    }
    imgFile.close();
    esp_camera_fb_return(frameBuffer);
    delay(500);

    // resetFramesize to value before frame caputring
    setFrameSize(device_pref.getCameraFramesize());

    return true;
  }
  else {
    return false;
  }
}


typedef struct {
  httpd_req_t *req;
  size_t len;
} jpg_chunking_t;

#define PART_BOUNDARY "123456789000000000000987654321"
static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* _STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

httpd_handle_t stream_httpd = NULL;
httpd_handle_t camera_httpd = NULL;




void setFrameSize(int val) {
  /*
    FRAMESIZE_UXGA (1600 x 1200)
    FRAMESIZE_QVGA (320 x 240)
    FRAMESIZE_CIF (352 x 288)
    FRAMESIZE_VGA (640 x 480)
    FRAMESIZE_SVGA (800 x 600)
    FRAMESIZE_XGA (1024 x 768)
    FRAMESIZE_SXGA (1280 x 1024)
  */
  sensor_t * s = esp_camera_sensor_get();
  if (val == 0)
    s->set_framesize(s, FRAMESIZE_QQVGA);
  else if (val == 1)
    s->set_framesize(s, FRAMESIZE_HQVGA);
  else if (val == 2)
    s->set_framesize(s, FRAMESIZE_QVGA);
  else if (val == 3)
    s->set_framesize(s, FRAMESIZE_CIF);
  else if (val == 4)
    s->set_framesize(s, FRAMESIZE_VGA);
  else if (val == 5)
    s->set_framesize(s, FRAMESIZE_SVGA);
  else if (val == 6)
    s->set_framesize(s, FRAMESIZE_XGA);
  else if (val == 7)
    s->set_framesize(s, FRAMESIZE_SXGA);
  else if (val == 8)
    s->set_framesize(s, FRAMESIZE_QVGA);
  else
    s->set_framesize(s, FRAMESIZE_QVGA);
}

std::string get_content(httpd_req_t *req) {
  // https://github.com/chhartmann/RoboProg/blob/ccc3342fccebc030b337dbaf20f5e917b5b24e5f/src/web_interface.cpp
  char buf[1025];
  int received;

  unsigned int remaining = req->content_len;
  std::string content;
  content.reserve(remaining);

  while (remaining > 0) {
    if ((received = httpd_req_recv(req, buf, std::min(remaining, sizeof(buf) - 1))) <= 0) {
      if (received == HTTPD_SOCK_ERR_TIMEOUT) {
        /* Retry if timeout occurred */
        continue;
      }

      /* In case of unrecoverable error, close and delete the unfinished file*/
      httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to receive file");
    } else {
      buf[received] = '\0';
      content += buf;
      remaining -= received;
    }
  }
  return content;
}


// Stream& input;
StaticJsonDocument<1024> doc;
static esp_err_t json_handler(httpd_req_t *req) {
  //we have to ways of changinv values through Json and random..
  // json-only would be better

  // convert http to json
  std::string content = get_content(req);
  DeserializationError error = deserializeJson(doc, content);

  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to create file");
    return ESP_FAIL;
  }
  else {
    // Adjust sensor values from REST
    sensor_t * s = esp_camera_sensor_get();

    if (doc.containsKey("gain")) {
      gain = doc["gain"];
      device_pref.setCameraGain(gain);
      Serial.println(gain);
      s->set_agc_gain(s, device_pref.getCameraGain());  // 0 to 30
      s->set_special_effect(s, device_pref.getCameraEffect()); //mono - has to be set again?
    }
    if (doc.containsKey("exposureTime")) {
      exposureTime = doc["exposureTime"];
      device_pref.setCameraExposureTime(exposureTime);
      Serial.println(exposureTime);
      s->set_aec_value(s, device_pref.getCameraExposureTime());    // (aec_value) 0 to 1200
      s->set_special_effect(s, device_pref.getCameraEffect()); //mono - has to be set again?
    }
    if (doc.containsKey("framesize")) {
      // FRAMESIZE
      frameSize = doc["framesize"];
      Serial.print("framesize: ");
      device_pref.setCameraFramesize(frameSize);
      setFrameSize(device_pref.getCameraFramesize());
      Serial.println(device_pref.getCameraFramesize());
      s->set_special_effect(s, device_pref.getCameraEffect()); //mono - has to be set again?
    }
    if (doc.containsKey("effect")) {
      // FRAMESIZE
      effect = doc["effect"];
      Serial.print("effect: ");
      device_pref.setCameraEffect(effect);
      s->set_special_effect(s, device_pref.getCameraEffect()); //mono - has to be set again?
      Serial.println(device_pref.getCameraEffect());
    } 
    if (doc.containsKey("ledintensity")) {
      // LED Intensity
      ledintensity = doc["ledintensity"];
      ledValueOld = ledintensity;
      ledcWrite(ledChannel, ledintensity);
      Serial.println(ledintensity);
    }
    if (doc.containsKey("lensvalue")) {
      // LENS Value 
      lensValueOld = doc["lensvalue"];
      ledcWrite(lensChannel, lensValueOld);
      Serial.println(lensValueOld);
    }
    
    httpd_resp_set_hdr(req, "Connection", "close");
    httpd_resp_send(req, "OK", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
  }
}


int readFile(char *fname, httpd_req_t *req) {
  //https://github.com/k-kimura123/Final_candyart_project/blob/ff3c950d02614a1ea169afaa0ac1793b503db4c5/main/include/myserver_config.c
  int res;
  char buf[1024];

  FILE *fd = fopen(fname, "rb");
  if (fd == NULL) {
    ESP_LOGE(TAG, "ERROR opening file (%d) %s\n", errno, strerror(errno));
    httpd_resp_send_404(req);
    return 0;
  }
  do {
    res = fread(buf, 1, sizeof(buf), fd);
    if (res > 0) {
      httpd_resp_send_chunk(req, buf, res);
      printf("Read %d\n", res);
    }
  } while (res > 0);
  httpd_resp_send_chunk(req, NULL, 0);
  res = fclose(fd);
  if (res) {
    printf("Error closing file\n");
  }
  return 1;
}

static size_t jpg_encode_stream(void * arg, size_t index, const void* data, size_t len) {
  jpg_chunking_t *j = (jpg_chunking_t *)arg;
  if (!index) {
    j->len = 0;
  }
  if (httpd_resp_send_chunk(j->req, (const char *)data, len) != ESP_OK) {
    return 0;
  }
  j->len += len;
  return len;
}

static esp_err_t capture_handler(httpd_req_t *req) {

  // Stop stream
  isStreaming = false;

  // make sure buffer is freed and framesize is taken over
  sensor_t * s = esp_camera_sensor_get();
  setFrameSize(device_pref.getCameraFramesize()); // TODO: Why does it change the exposure time/brightness??! 
  s->set_aec_value(s, device_pref.getCameraExposureTime());    // (aec_value) 0 to 1200
  s->set_special_effect(s, device_pref.getCameraEffect()); //mono - has to be set again?
  s->set_agc_gain(s, device_pref.getCameraGain());  // 0 to 30

  camera_fb_t * frameBuffer = NULL;

  // Take Picture with Camera, but first let camera warm up => make sure low res frame in buffer is freed
  for (int iwarmup = 0; iwarmup < 3; iwarmup++) {
    frameBuffer = esp_camera_fb_get();
    esp_camera_fb_return(frameBuffer);
  }
  frameBuffer = esp_camera_fb_get();

  if (!frameBuffer) {
    Serial.println("Camera capture failed");
    ESP.restart();
    return false;
  }


  httpd_resp_set_type(req, "image/jpeg");
  httpd_resp_set_hdr(req, "Content-Disposition", "inline; filename=capture.jpeg");

  size_t out_len, out_width, out_height;
  uint8_t * out_buf;
  esp_err_t res = -1;
  //bool s;
  {  
    size_t fb_len = 0;
    if (frameBuffer->format == PIXFORMAT_JPEG) {
      fb_len = frameBuffer->len;
      res = httpd_resp_send(req, (const char *)frameBuffer->buf, frameBuffer->len);
    } else {
      jpg_chunking_t jchunk = {req, 0};
      res = frame2jpg_cb(frameBuffer, 80, jpg_encode_stream, &jchunk) ? ESP_OK : ESP_FAIL;
      httpd_resp_send_chunk(req, NULL, 0);
      fb_len = jchunk.len;
    }
    esp_camera_fb_return(frameBuffer);
    Serial.printf("JPG: %uB", (uint32_t)(fb_len));
    return res;
  }
  esp_camera_fb_return(frameBuffer);
  Serial.println("dl_matrix3du_alloc failed");
  httpd_resp_send_500(req);
  return ESP_FAIL;
}



static esp_err_t stream_handler(httpd_req_t *req) {
  camera_fb_t * fb = NULL;
  esp_err_t res = ESP_OK;
  size_t _jpg_buf_len = 0;
  uint8_t * _jpg_buf = NULL;
  char * part_buf[64];
  //  dl_matrix3du_t *image_matrix = NULL;

  static int64_t last_frame = 0;
  if (!last_frame) {
    last_frame = esp_timer_get_time();
  }

  res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
  if (res != ESP_OK) {
    return res;
  }

  // in case we want to stop it externally
  isStreaming = true;

  while (isStreaming) {
    fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      res = ESP_FAIL;
      ESP.restart();
    } else {
      {
        if (fb->format != PIXFORMAT_JPEG) {
          bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
          esp_camera_fb_return(fb);
          fb = NULL;
          if (!jpeg_converted) {
            Serial.println("JPEG compression failed");
            res = ESP_FAIL;
          }
        } else {
          _jpg_buf_len = fb->len;
          _jpg_buf = fb->buf;
        }
      }
    }
    if (res == ESP_OK) {
      size_t hlen = snprintf((char *)part_buf, 64, _STREAM_PART, _jpg_buf_len);
      res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
    }
    if (res == ESP_OK) {
      res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
    }
    if (res == ESP_OK) {
      res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
    }
    if (fb) {
      esp_camera_fb_return(fb);
      fb = NULL;
      _jpg_buf = NULL;
    } else if (_jpg_buf) {
      free(_jpg_buf);
      _jpg_buf = NULL;
    }
    if (res != ESP_OK) {
      break;
    }
  }

  last_frame = 0;
  return res;
}

enum state {fwd, rev, stp};
state actstate = stp;

static esp_err_t cmd_handler(httpd_req_t *req)
{

  // TODO: this should be arduinojson
  char*  buf;
  size_t buf_len;
  char variable[32] = {0,};
  char value[32] = {0,};

  // adjust parameters
  buf_len = httpd_req_get_url_query_len(req) + 1;
  if (buf_len > 1) {
    buf = (char*)malloc(buf_len);
    if (!buf) {
      httpd_resp_send_500(req);
      return ESP_FAIL;
    }
    if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
      if (httpd_query_key_value(buf, "var", variable, sizeof(variable)) == ESP_OK &&
          httpd_query_key_value(buf, "val", value, sizeof(value)) == ESP_OK) {
      } else {
        free(buf);
        httpd_resp_send_404(req);
        return ESP_FAIL;
      }
    } else {
      free(buf);
      httpd_resp_send_404(req);
      return ESP_FAIL;
    }
    free(buf);
  } else {
    httpd_resp_send_404(req);
    return ESP_FAIL;
  }

  int val = atoi(value);
  sensor_t * s = esp_camera_sensor_get();
  int res = 0;

  if (!strcmp(variable, "brightness"))
  {
    Serial.print("brightness ");
    float brightness = (float)val / 10;
    Serial.println(brightness);
    s->set_brightness(s, brightness);  // -2 to 2
    s->set_special_effect(s, device_pref.getCameraEffect()); //mono - has to be set again?
  }
  if (!strcmp(variable, "effect"))
  {
    Serial.print("effect ");
    float effect = val;
    device_pref.setCameraEffect(effect);
    s->set_special_effect(s, device_pref.getCameraEffect()); //mono - has to be set again?
    Serial.println(device_pref.getCameraEffect());
  }
  else if (!strcmp(variable, "gain"))
  {
    gain = val;
    device_pref.setCameraGain(gain);
    s->set_agc_gain(s, device_pref.getCameraGain());  // 0 to 30
    s->set_special_effect(s, device_pref.getCameraEffect()); //mono - has to be set again?

    Serial.print("gain ");
    Serial.println(gain);
    Serial.println(device_pref.getCameraGain());
  }
  else if (!strcmp(variable, "exposuretime"))
  {
    // EXPOSURE TIME
    exposureTime = val;
    device_pref.setCameraExposureTime(exposureTime);
    s->set_aec_value(s, device_pref.getCameraExposureTime());  // 0 to 30
    s->set_special_effect(s, device_pref.getCameraEffect()); //mono - has to be set again?

    Serial.print("exposureTime ");
    Serial.println(device_pref.getCameraExposureTime());
    Serial.println(exposureTime);
  }
  else if (!strcmp(variable, "framesize")) {
    // FRAMESIZE
    frameSize = val;
    device_pref.setCameraFramesize(frameSize);
    setFrameSize(device_pref.getCameraFramesize());
    s->set_special_effect(s, device_pref.getCameraEffect()); //mono - has to be set again?

    Serial.print("framesize: ");
    Serial.println(device_pref.getCameraFramesize());
    Serial.println(val);
  }
  else if (!strcmp(variable, "lenscorrection")) {
    // Timelapse Periode
    s->set_lenc(s, val);
    Serial.print(" lenscorrection: ");
    Serial.println(val);
  }


  else if (!strcmp(variable, "timelapseinterval")) {
    // Timelapse Periode
    device_pref.setTimelapseInterval(val);
    timelapseInterval = device_pref.getTimelapseInterval();

    Serial.print("timelapse Interval: ");
    Serial.println(device_pref.getTimelapseInterval());
    Serial.println(val);
  }
  else if (!strcmp(variable, "quality"))
  {
    // QUALITY
    Serial.println("quality");
    res = s->set_quality(s, val);
  }
  else if (!strcmp(variable, "flash"))
  {
    // FLASH
    ledValueOld = val;
    ledcWrite(ledChannel, ledValueOld);
    Serial.print("LED VAlue");
    Serial.println(ledValueOld);
  }
  else if (!strcmp(variable, "lens"))
  {
    // FLASH
    lensValueOld = val;
    ledcWrite(lensChannel, lensValueOld);
    Serial.print("LENS VAlue");
    Serial.println(lensValueOld);
  }
  else
  {
    Serial.println("variable");
    res = -1;
  }

  if (res) {
    return httpd_resp_send_500(req);
  }

  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  return httpd_resp_send(req, NULL, 0);

}

static esp_err_t restart_handler(httpd_req_t *req) {
  Serial.println("Restarting");
  httpd_resp_set_type(req, "application/json");
  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  httpd_resp_send(req, "OK", strlen("OK"));
  ESP.restart();
  return ESP_OK;
}

static esp_err_t id_handler(httpd_req_t *req) {
  Serial.println("Handling ID");
  static char json_response[1024];
  String uniqueID = "OMNISCOPE" __DATE__ " " __TIME__;
  Serial.println("Handling ID");
  Serial.println(uniqueID);

  char * p = json_response;
  *p++ = '{';
  p += sprintf(p, uniqueID.c_str());
  *p++ = '}';
  *p++ = 0;
  httpd_resp_set_type(req, "application/json");
  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  return httpd_resp_send(req, json_response, strlen(json_response));
}

static esp_err_t index_handler(httpd_req_t *req) {
  httpd_resp_set_type(req, "text/html");
  return httpd_resp_send(req, (const char *)INDEX_HTML, strlen(INDEX_HTML));
}

static esp_err_t status_handler(httpd_req_t *req) {
  static char json_response[1024];

  sensor_t * s = esp_camera_sensor_get();
  char * p = json_response;
  *p++ = '{';

  p += sprintf(p, "\"framesize\":%u,", s->status.framesize);
  p += sprintf(p, "\"quality\":%u,", s->status.quality);
  *p++ = '}';
  *p++ = 0;
  httpd_resp_set_type(req, "application/json");
  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  return httpd_resp_send(req, json_response, strlen(json_response));
}

static esp_err_t enable_handler(httpd_req_t *req) {
  device_pref.setIsTimelapse(true);
  static char json_response[1024];
  char * p = json_response;
  *p++ = '{';
  p += sprintf(p, "You have enabled long-time timelpase - reflash the code or release the button to wake up the ESP again");
  *p++ = '}';
  *p++ = 0;
  httpd_resp_set_type(req, "application/json");
  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  httpd_resp_send(req, json_response, strlen(json_response));

  ESP.restart();
  return 0;
}


esp_err_t indexhtml_handler(httpd_req_t *req) {
  ESP_LOGI(TAG, "url %s was hit", req->uri);
  printf("main page requested\r\n");
  httpd_resp_set_type(req, "text/html");
  readFile("/spiffs/index.html", req);

  return ESP_OK;
}



void startCameraServer()
{
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();

  httpd_uri_t index_uri = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = index_handler,
    .user_ctx  = NULL
  };

  httpd_uri_t indexhtml_uri = {
    .uri       = "/index.html",
    .method    = HTTP_GET,
    .handler   = indexhtml_handler,
    .user_ctx  = NULL
  };

  httpd_uri_t status_uri = {
    .uri       = "/status",
    .method    = HTTP_GET,
    .handler   = status_handler,
    .user_ctx  = NULL
  };

  httpd_uri_t cmd_uri = {
    .uri       = "/control",
    .method    = HTTP_GET,
    .handler   = cmd_handler,
    .user_ctx  = NULL
  };

  httpd_uri_t enable_uri = {
    .uri       = "/enable",
    .method    = HTTP_GET,
    .handler   = enable_handler,
    .user_ctx  = NULL
  };

  httpd_uri_t capture_uri = {
    .uri       = "/capture.jpeg",
    .method    = HTTP_GET,
    .handler   = capture_handler,
    .user_ctx  = NULL
  };

  httpd_uri_t stream_uri = {
    .uri       = "/stream.mjpeg",
    .method    = HTTP_GET,
    .handler   = stream_handler,
    .user_ctx  = NULL
  };

  httpd_uri_t id_uri = {
    .uri       = "/getid",
    .method    = HTTP_GET,
    .handler   = id_handler,
    .user_ctx  = NULL
  };

  httpd_uri_t restart_uri = {
    .uri       = "/restart",
    .method    = HTTP_GET,
    .handler   = restart_handler,
    .user_ctx  = NULL
  };

  httpd_uri_t postjson_uri = {
    .uri       = "/postjson",
    .method    = HTTP_POST,
    .handler   = json_handler,
    .user_ctx  = NULL
  };

  Serial.printf("Starting web server on port: '%d'\n", config.server_port);
  if (httpd_start(&camera_httpd, &config) == ESP_OK) {
    httpd_register_uri_handler(camera_httpd, &index_uri);
    httpd_register_uri_handler(camera_httpd, &cmd_uri);
    httpd_register_uri_handler(camera_httpd, &enable_uri);
    httpd_register_uri_handler(camera_httpd, &status_uri);
    httpd_register_uri_handler(camera_httpd, &capture_uri);
    httpd_register_uri_handler(camera_httpd, &id_uri);
    httpd_register_uri_handler(camera_httpd, &indexhtml_uri);
    httpd_register_uri_handler(camera_httpd, &postjson_uri);
    httpd_register_uri_handler(camera_httpd, &restart_uri);
  }

  config.server_port += 1;
  config.ctrl_port += 1;
  Serial.printf("Starting stream server on port: '%d'\n", config.server_port);
  if (httpd_start(&stream_httpd, &config) == ESP_OK) {
    httpd_register_uri_handler(stream_httpd, &stream_uri);
  }
}
