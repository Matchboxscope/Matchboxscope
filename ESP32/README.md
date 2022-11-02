# Matchboxscope ESP32 Software

## Setup

### Prerequisites

This Arduino sketch is written for the [Ai-Thinker ESP32-CAM](http://www.ai-thinker.com/pro_view-24.html) board. Before you can upload the sketch to the board, you need to carry out the following instructions once:

1. Install the Arduino IDE [here](https://arduino.cc/). This project is tested with v1.8.x of the Arduino IDE.
2. In the Preferences dialog of the Arduino IDE, add the following URL to the "Additional Board Manager URLs" dialog: `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_dev_index.json` (detailed instructions with screenshots [here](https://randomnerdtutorials.com/installing-the-esp32-board-in-arduino-ide-windows-instructions/))
3. In the Boards Manager dialog of the Arduino IDE, install v2.0.x of the esp32 boards package.
4. In the Libraries Manager dialog of the Arduino IDE, install v6.19.x of the [ArduinoJson](https://github.com/bblanchon/ArduinoJson) package (by Benoit Blanchon) and v2.0.x of the [WifiManager](https://github.com/tzapu/WiFiManager) package (by tzapu).
5. Download the latest Matchboxscope release from the [Matchboxscope releases page](https://github.com/beniroquai/Matchboxscope/releases) and unzip the archive.
6. Add v1.6 of the [ESP32Ping](https://github.com/marian-craciunescu/ESP32Ping) package to your Arduino libraries. You will need to do this manually, as it is not listed in the Arduino IDE's Library Manager. You can do this by copying the `libraries/ESP32Ping` subdirectory into your Arduino workspace's `libraries` directory.

### Software Upload

The Matchboxscope software consists of two components: firmware, which performs all hardware control and serves a web interface; and static web assets, which are used for the web interface provided by the firmware.

#### Firmware Configuration

You will upload the firmware to the ESP32-CAM board from the Arduino IDE, which you should use to open the `main.ino` file in this directory. However, before you upload the firmware to the ESP32-CAM board you will need to change some settings to match your needs.

Anglerfish settings:

- `timelapseIntervalAnglerfish`: **this is an undocumented setting.**
- `focuStackStepsizeAnglerfish`: **this is an undocumented setting.**
- `isTimelapseAnglerfish`: **this is an undocumented setting.**
- `isAcquireStack`: **this is an undocumented setting.**
- `myFile`: **this is not a setting and does not belong in this section.**

Wifi modes: there are three "modes" in which you can configure wifi connectivity for the firmware:

- Normal mode: the ESP32-CAM board will attempt to connect directly to a wifi network upon startup.
- Access Point mode: the ESP32-CAM board will bring up its own local wifi network as an access point; then you should use your computer (or other wifi-enabled device) to connect to that wifi network, after which you can use your web browser to open `192.168.4.1` to operate the Matchboxscope.
- Captive Portal mode: the ESP32-CAM board will first bring up its own local wifi network as an access point which serves a web interface for configuring wifi settings to connect to the internet from another wifi network; then you should use your computer (or other wifi-enabled device) to connect to that wifi network, after which you can use your web browser to open `192.168.4.1` to configure the Matchboxscope for internet connectivity.

Wifi settings:

- `hostWifiAP`: set to `true` to use the Matchboxscope in "Access Point" or "Captive Portal" mode.
- `isCaptivePortal`: set to `true` to use the Matchboxscope in "Captive Portal" mode.
- `mSSID`: set to the SSID of the wifi network to connect to in "Normal" mode.
- `mPASSWORD`: set to the password of the wifi network to connect to in "Normal" mode.
- `hostname`: **this is an undocumented setting.**
- `WifiManager`: **this is not a setting and does not belong in this section.**
- `previousCheckWifi`: **this is not a setting and does not belong in this section.**
- `delayReconnect`: **this is not a setting and does not belong in this section.**
- `isInternetAvailable`: **this is not a setting and does not belong in this section.**
- `isStreaming`: **this is not a setting and does not belong in this section.**
- `isStreamingStopped`: **this is not a setting and does not belong in this section.**

Google Drive upload settings:

- `myDomain`: **this is an undocumented setting.**
- `myScript`: **this is an undocumented setting.**
- `myFilename`: **this is an undocumented setting.**
- `mimeType`: **this is an undocumented setting.**
- `myImage`: **this is an undocumented setting.**
- `waitingTime`: **this is an undocumented setting.**

#### Firmware Upload

To upload the firmware, set the board type to ESP32 Arduino > ESP32 Dev Module and use a USB cable to connect your ESP32-CAM board to your computer. The following settings should work:

- Upload speed: 921600
- CPU Frequency: 240 MHz (WiFi/BT)
- Flash Frequency: 80MHz
- Flash Mode: QIO
- Flash Size: 4MB (32Mb) (**this is not a default setting**)
- Partition Scheme: Default 4MB with spiffs (1.2MB APP/1.5MB SPIFFS)
- Core Debug Level: None
- PSRAM: Enabled (**this is not a default setting**)
- Arduino Runs On: Core 1
- Events Run On: Core 1

You will need to set the USB port of the board based on your computer's settings. Then you should be able to compile and upload the firmware.

You only need to upload the firmware once to the ESP32-CAM if you are using the Matchboxscope software in Matchboxscope mode. If you are using it in Anglerfish mode, you will need to upload the firmware once before deploying it, for each time you deploy it; this is needed due to the way the Anglerfish mode uses the ESP32 board's deep sleep functionality.A

#### Static Web Asset Upload

The Matchboxscope firmware serves a web interface which depends on some HTML, CSS, and Javascript files as static web assets. Those files will need to be uploaded once to the ESP32's PSRAM as an SFPIFFS image, independently of the firmware. You should do this using the Tools > ESP32 Data Sketch Upload menu item. You only need to re-upload these static web assets when you modify them.

## Usage

## Matchboxscope Mode

Instructions for how to operate the Matchboxscope software in Matchboxscope mode are not provided at this time.

### Anglerfish Mode

Detailed instructions for how to operate the Matchboxscope software in Anglerfish mode are not provided at this time. General instructions for overall use of the hardware and software together are in the README.md file at the root of this repository.

## Troubleshooting

If you're on an Android phone configured to simultaneously use WiFi and mobile data to connect to the internet (which may be listed as "Wifi+" mode), it may be unable to access the Matchboxscope in Access Point mode at address 192.168.4.1; in that case, you should disable Wifi+ mode.

## License

All software in this directory, including the `main` subdirectory, is licensed under the MIT License, except where otherwise noted. Dependencies provided together with this software, under the `libraries` subdirectory, are distributed under their own respective licenses - refer to their README and LICENSE files for details.

## Collaboration
If you find this project useful, please like this repository, follow us on Twitter and cite the webpage! :-)
