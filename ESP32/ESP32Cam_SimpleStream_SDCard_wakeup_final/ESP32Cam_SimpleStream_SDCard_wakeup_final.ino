//IMPORTANT: ENABLE PSRAM!
#include <esp32cam.h>
#include <WebServer.h>
#include "esp_camera.h"
#include "Arduino.h"
#include "soc/soc.h"           // Disable brownour problems
#include "soc/rtc_cntl_reg.h"  // Disable brownour problems
#include "driver/rtc_io.h"
#include <Preferences.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include <FS.h>
#include "SD_MMC.h" // SD card  

/*
 * 
 * TODO's:
 * 
 * - Whitebalance/constant intensity
 * - Swap esp32cam.h with esp_camera?
 * - Any overflow to be expected with pictureNumber?
 */

// Constant settings
#define WIFIAP                  // want to use the ESP32 as an access point?
#define PIN_LED 4               // pin of the LED flash
#define PIN_REEDSWITCH 13       // pin of the external switch to "awake" the ESP (e.g. Reedrelay)

// conversion factors for letting the ESP go sleep
#define uS_TO_S_FACTOR 1000000  // Conversion factor for micro seconds to seconds 
#define TIME_TO_SLEEP  10       // Time ESP32 will go to sleep (in seconds) 


// determine time of compiling
const char compile_date[] = __DATE__ " " __TIME__;


#ifdef WIFIAP
const char *SSID = "MatchboxScope";
#else
const char *SSID = "YOUR_SSID";
const char *PWD = "YOU_PASSWORD";
#endif

// Initiliaze constant memory (e.g. EEPROM)
Preferences preferences;

// define Webserver running on port 80
WebServer server(80);

unsigned char pictureNumber = 0; // 0..255

// switches to make sure the esp can wake up for focusssing/live mode 
bool is_activated = false;         // ready for operation? // setup has been enabled through HTTP?
bool is_reflashed = false;      // did we flash a new code? If so, refocus!


static auto loRes = esp32cam::Resolution::find(320, 240);
static auto hiRes = esp32cam::Resolution::find(800, 600);
static auto maxRes = esp32cam::Resolution::find(1600, 1200);



/*
 * 
 * Functions to initialize hardware and auxilary components
 * 
 */
void initSpiffs()
{ // initiliaze internal file system 
  if (!SPIFFS.begin()) 
  {
    Serial.println("Erreur SPIFFS...");
    return;
  }
  File root = SPIFFS.open("/");    
  File file = root.openNextFile(); 
  while (file)                     
  {// print all files to check if website HtML file exists
    Serial.print("File: ");
    Serial.println(file.name());
    file.close();
    file = root.openNextFile(); 
  }
}


void initiWiFi() {
  // connect to Wifi; either AP or through Router
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



void initCamera() {
  using namespace esp32cam;
  Config cfg;
  cfg.setPins(pins::AiThinker);
  cfg.setResolution(maxRes);
  //cfg.setGrayscale(); // causes problems, doesn't add any functionality other than numpy.mean()
  cfg.setBufferCount(2);
  cfg.setJpeg(80);

  bool ok = Camera.begin(cfg);
  Serial.println(ok ? "CAMERA OK" : "CAMERA FAIL");
}




/*
 * 
 * Webserver-related settings - endpoints to listen to 
 * 
 * 
*/
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
  // register all the endpoints
  char endpoint_index_non[]= "/";
  char endpoint_index[] = "/";
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


/*
 * 
 * 
 * Deep-sleep activation
 * 
 */

void enable() {
  Serial.println("ENABLING!");
  preferences.begin("camera", false);
  is_activated = 1; // focus has been set
  preferences.putUInt("is_activated", is_activated);    // permanently set the is_ready flat
  Serial.println(preferences.getUInt("is_activated", is_activated));
  preferences.end();
  server.send(200, "text/html", "You logged yourself out. The camera will now start capturing images every N-seconds forever. To reactivate Wifi, you have to reflash the code or press the button..");
}


/*
 * 
 * Setup the different functions
 * 
 */





void setup()
{
  Serial.begin(115200);

  // check if button is activated to avoid deep-sleep loop
  pinMode(PIN_REEDSWITCH, INPUT_PULLUP);
  Serial.println("Press Button if you want to refocus (t..4s)");
  delay(4000);

  // reading button state
  bool buttonState = digitalRead(PIN_REEDSWITCH); // pull up
  if(buttonState)
    Serial.println("Button has not been activated");
  else
    Serial.println("Button has been activated");

  // Opening pref-file
  preferences.begin("camera", false);

  // did we flash a new programm? If so, we want to avoid boot-loop and go to focus-mode independent from a pressed button.
  String compile_date_old_str = preferences.getString("compile_date_old", ""); 
  String compile_date_str(compile_date);
  
  // compare and see if we need to reset
  if(not compile_date_old_str.equals(compile_date_str)){ // we flashed a new code, lets switch to refocus mode
     preferences.putString("compile_date_old", compile_date_str);
     is_reflashed = true; // ot used yet
     preferences.putUInt("is_activated", 0); // force reset
  }
  
  // if we press a button and want to avoid bootloop check fo rhtat!
  if (!buttonState) { // it is always on - pulled up 
    preferences.putUInt("is_activated", 0); // force reset
  }
  
  is_activated = preferences.getUInt("is_activated", 0);
  preferences.end();

  // DEBUGGING
  if(is_activated)
    Serial.println("The setup has been activated?");
  else
    Serial.println("The setup has not been activated! Connect to Wifi and go to 192.168.4.1./enable once you'Re done with focussing");


  // init all external hardware parts only if we need it
  if (!is_activated) {
    initiWiFi();
    initSpiffs();
    initWebServerConfig();
  }

  // initialize camera
  initCamera();

  // setup pin to work with the torch/flash - make sure it's available for the SD card later!
  pinMode(PIN_LED, OUTPUT);
  

}

void loop()
{
  
  // if the setup is set-up, we only want to take one picture, save it and go to deep-sleep.. zzzzz
  // could go to setup?
  if (is_activated) {
    // open EEPROM preferences
    preferences.begin("camera", false);
    pictureNumber = preferences.getUInt("pictureNumber", 0) + 1;
    String path = "/picture" + String(pictureNumber) + ".jpg";

     if (!esp32cam::Camera.changeResolution(maxRes)) {
      Serial.println("SET-HI-RES FAIL");
    }

    // turn on led to see anything..
    digitalWrite(PIN_LED, HIGH); 
    
    // warm up the camera; usually takes some frames to change resolution too
    auto frame = esp32cam::capture();
    for (int i = 0; i < 5; i++) frame = esp32cam::capture(); // warm up / adust

    // Make sure LEd is switched off
    digitalWrite(4, LOW);

    // rest pinnumber 4 and save image on SD card
    if  (!SD_MMC.begin()) { // (!SD_MMC.begin("/sdcard", true)) { //1bit mode -
      Serial.println("SD Card Mount Failed");
    }
    Serial.printf("Picture file name: %s\n", path.c_str());

    fs::FS &fs = SD_MMC; 
    File file = fs.open(path.c_str(), FILE_WRITE);
    Serial.println("Opening file");
    if (!file)
    {
      Serial.println("Failed to open file in writing mode");
      file.close();
    }
    else
    {
      if (frame == nullptr) {
        Serial.println("CAPTURE FAIL");
        file.close();
      }
      Serial.printf("CAPTURE OK %dx%d %db\n", frame->getWidth(), frame->getHeight(),
                    static_cast<int>(frame->size()));

      file.write(frame->data(), frame->size()); // payload (image), payload length
      preferences.putUInt("pictureNumber", pictureNumber);
      preferences.end();


      file.close();
    }

    Serial.println("Go to sleep now");
    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
    Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) +
                   " Seconds");

    // Make sure LEd is switched off
    digitalWrite(4, LOW);
    rtc_gpio_hold_en(GPIO_NUM_4);
    esp_deep_sleep_start();
  }
  else {
    // stream images and website to turn focus
    server.handleClient();
  }
}
