/*
// Observation:
if voltage of battery drops => SD card error since file may be written only partially
File cannot be recovered and since we do the ++1 for file name 


exception decoder for below error:

PC: 0x400d5d05: doFocus(int, bool, bool, String) at /Users/bene/Dropbox/Dokumente/Promotion/PROJECTS/Matchboxscope/ESP32/main/app_httpd.ino line 659 =>fb_len = frameBuffer->len;
EXCVADDR: 0x00000004

Decoding stack results
0x400d5d02: doFocus(int, bool, bool, String) at /Users/bene/Dropbox/Dokumente/Promotion/PROJECTS/Matchboxscope/ESP32/main/app_httpd.ino line 657 =>esp_camera_fb_return(frameBuffer);
0x400d639a: setup() at /Users/bene/Dropbox/Dokumente/Promotion/PROJECTS/Matchboxscope/ESP32/main/main.ino line 227
0x400ea996: loopTask(void*) at /Users/bene/Library/Arduino15/packages/esp32/hardware/esp32/2.0.3/cores/esp32/main.cpp line 42


// only a complete reflash resolved this issue..


Guru Meditation Error: Core  1 panic'ed (LoadProhibited). Exception was unhandled.

Core  1 register dump:
PC      : 0x400d5d05  PS      : 0x00060a30  A0      : 0x800d639d  A1      : 0x3ffb2740  
A2      : 0x00000000  A3      : 0x0000004b  A4      : 0x00000000  A5      : 0x3ffb27ec  
A6      : 0x00000000  A7      : 0x00001b7a  A8      : 0x800d5d05  A9      : 0x3ffb2720  
A10     : 0x00000000  A11     : 0x000000ff  A12     : 0xeff7f958  A13     : 0x3ffc5cf4  
A14     : 0x3ffc5cf4  A15     : 0x3ffb26dc  SAR     : 0x00000001  EXCCAUSE: 0x0000001c  
EXCVADDR: 0x00000004  LBEG    : 0x4008ced9  LEND    : 0x4008cee9  LCOUNT  : 0xfffffffe  


Backtrace:0x400d5d02:0x3ffb27400x400d639a:0x3ffb27b0 0x400ea996:0x3ffb2820 




ELF file SHA256: 0000000000000000

Rebooting...
ets Jun  8 2016 00:22:57

rst:0xc (SW_CPU_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:1184
load:0x40078000,len:12812
load:0x40080400,len:3032
entry 0x400805e4

Wakeup was not caused by deep sleep: 0
Stored date:
Oct  1 2022 20:49:38
Compiled date:
Oct  1 2022 20:49:38
First run? no
Camera init success!Camera is working
SD Card Mounted
2
Writing to file...
Writing file: /debug.txt
File written
Set pin 4 high to wake up the ESP32 from deepsleep
exposureTime : 547
gain : 0
timelapseInterval : 0
Appending to file: /debug.txt
Message appended
Appending to file: /debug.txt
Message appended
Appending to file: /debug.txt
Message appended
LED : 255
LED : 255
LED : 0
LED : 0
LENS Value: 255
LENS Value: 0
Appending to file: /debug.txt
Message appended
Appending to file: /debug.txt
Message appended
In timelapse mode.
Appending to file: /debug.txt
Message appended
LENS Value: 0
/anglerfish_35_Z_0
LED : 255
Saved /anglerfish_35_Z_0.jpg
LED : 0
JPG: 6627BLENS Value: 25
/anglerfish_35_Z_25
LED : 255
Saved /anglerfish_35_Z_25.jpg
LED : 0
JPG: 6627BLENS Value: 50
/anglerfish_35_Z_50
LED : 255
Failed to open file in writing mode
LED : 0
JPG: 6627BLENS Value: 75
/anglerfish_35_Z_75
LED : 255
Failed to open file in writing mode
LED : 0
Guru Meditation Error: Core  1 panic'ed (LoadProhibited). Exception was unhandled.

Core  1 register dump:
PC      : 0x400d5d05  PS      : 0x00060a30  A0      : 0x800d639d  A1      : 0x3ffb2740  
A2      : 0x00000000  A3      : 0x0000004b  A4      : 0x00000000  A5      : 0x3ffb27ec  
A6      : 0x00000000  A7      : 0x000019e3  A8      : 0x800d5d05  A9      : 0x3ffb2720  
A10     : 0x00000000  A11     : 0x000000ff  A12     : 0xd6d6cca1  A13     : 0x3ffc5cf4  
A14     : 0x3ffc5cf4  A15     : 0x3ffb26dc  SAR     : 0x00000001  EXCCAUSE: 0x0000001c  
EXCVADDR: 0x00000004  LBEG    : 0x4008ced9  LEND    : 0x4008cee9  LCOUNT  : 0xfffffffe  


Backtrace:0x400d5d02:0x3ffb27400x400d639a:0x3ffb27b0 0x400ea996:0x3ffb2820 




ELF file SHA256: 0000000000000000

Rebooting...
ets Jun  8 2016 00:22:57

rst:0xc (SW_CPU_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:1184
load:0x40078000,len:12812
load:0x40080400,len:3032
entry 0x400805e4

Wakeup was not caused by deep sleep: 0
Stored date:
Oct  1 2022 20:49:38
Compiled date:
Oct  1 2022 20:49:38
First run? no
Camera init success!Camera is working
SD Card Mounted
2
Writing to file...
Writing file: /debug.txt
File written
Set pin 4 high to wake up the ESP32 from deepsleep
exposureTime : 547
gain : 0
timelapseInterval : 0
Appending to file: /debug.txt
Message appended
Appending to file: /debug.txt
Message appended
Appending to file: /debug.txt
Message appended
LED : 255
LED : 255
LED : 0
LED : 0
LENS Value: 255
LENS Value: 0
Appending to file: /debug.txt
Message appended
Appending to file: /debug.txt
Message appended
In timelapse mode.
Appending to file: /debug.txt
Message appended
LENS Value: 0
/anglerfish_35_Z_0
LED : 255
Saved /anglerfish_35_Z_0.jpg
LED : 0
JPG: 6727BLENS Value: 25
/anglerfish_35_Z_25
LED : 255
Saved /anglerfish_35_Z_25.jpg
LED : 0
JPG: 6737BLENS Value: 50
/anglerfish_35_Z_50
LED : 255
Failed to open file in writing mode
LED : 0
JPG: 6739BLENS Value: 75
/anglerfish_35_Z_75
LED : 255
Failed to open file in writing mode
LED : 0
Guru Meditation Error: Core  1 panic'ed (LoadProhibited). Exception was unhandled.

Core  1 register dump:
PC      : 0x400d5d05  PS      : 0x00060a30  A0      : 0x800d639d  A1      : 0x3ffb2740  
A2      : 0x00000000  A3      : 0x0000004b  A4      : 0x00000000  A5      : 0x3ffb27ec  
A6      : 0x00000032  A7      : 0x00001a53  A8      : 0x800d5d05  A9      : 0x3ffb2720  
A10     : 0x00000000  A11     : 0x000000ff  A12     : 0xb848348f⸮ets Jun  8 2016 00:22:57

rst:0x1 (POWERON_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:1184
load:0x40078000,len:12812
load:0x40080400,len:3032
entry 0x400805e4

Wakeup was not caused by deep sleep: 0
Stored date:
Oct  1 2022 20:49:38
Compiled date:
Oct  1 2022 20:49:38
First run? no
Camera init success!Camera is working
SD Card Mounted
2
Writing to file...
Writing file: /debug.txt
File written
Set pin 4 high to wake up the ESP32 from deepsleep
exposureTime : 547
gain : 0
timelapseInterval : 0
Appending to file: /debug.txt
Message appended
Appending to file: /debug.txt
Message appended
Appending to file: /debug.txt
Message appended
LED : 255
LED : 255
LED : 0
LED : 0
LENS Value: 255
LENS Value: 0
Appending to file: /debug.txt
Message appended
Appending to file: /debug.txt
Message appended
In timelapse mode.
Appending to file: /debug.txt
Message appended
LENS Value: 0
/anglerfish_35_Z_0
LED : 255
Saved /anglerfish_35_Z_0.jpg
LED : 0
JPG: 6663BLENS Value: 25
/anglerfish_35_Z_25
LED : 255
Saved /anglerfish_35_Z_25.jpg
LED : 0
JPG: 6664BLENS Value: 50
/anglerfish_35_Z_50
LED : 255
Failed to open file in writing mode
LED : 0
JPG: 6664BLENS Value: 75
/anglerfish_35_Z_75
LED : 255
Failed to open file in writing mode
LED : 0
Guru Meditation Error: Core  1 panic'ed (LoadProhibited). Exception was unhandled.

Core  1 register dump:
PC      : 0x400d5d05  PS      : 0x00060a30  A0      : 0x800d639d  A1      : 0x3ffb2740  
A2      : 0x00000000  A3      : 0x0000004b  A4      : 0x00000000  A5      : 0x3ffb27ec  
A6      : 0x00000019  A7      : 0x00001a08  A8      : 0x800d5d05  A9      : 0x3ffb2720  
A10     : 0x00000000  A11     : 0x000000ff  A12     : 0x0f26449a  A13     : 0x3ffc5cf4  
A14     : 0x3ffc5cf4  A15     : 0x3ffb26dc  SAR     : 0x00000001  EXCCAUSE: 0x0000001c  
EXCVADDR: 0x00000004  LBEG    : 0x4008ced9  LEND    : 0x4008cee9  LCOUNT  : 0xfffffffe  


Backtrace:0x400d5d02:0x3ffb27400x400d639a:0x3ffb27b0 0x400ea996:0x3ffb2820 




ELF file SHA256: 0000000000000000

Rebooting...
ets Jun  8 2016 00:22:57

rst:0xc (SW_CPU_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:1184
load:0x40078000,len:12812
load:0x40080400,len:3032
entry 0x400805e4

Wakeup was not caused by deep sleep: 0
Stored date:
Oct  1 2022 20:49:38
Compiled date:
Oct  1 2022 20:49:38
First run? no
ets Jun  8 2016 00:22:57

rst:0x8 (TG1WDT_SYS_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:1184
load:0x00000000,len:0
load:0x00000000,len:-53504
ets Jun  8 2016 00:22:57

rst:0x7 (TG0WDT_SYS_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:1184
load:0x40078000,len:12812
load:0x40080400,len:3032
entry 0x400805e4

Brownout detector was triggered

ets Jun  8 2016 00:22:57

rst:0xc (SW_CPU_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:1184
load:0x40078000,len:12812
load:0x40080400,len:3032
entry 0x400805e4

Wakeup was not caused by deep sleep: 0
Stored date:
Oct  1 2022 20:49:38
Compiled date:
Oct  1 2022 20:49:38
First run? no
ets Jun  8 2016 00:22:57

rst:0x8 (TG1WDT_SYS_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:1184
load:0x40078000,len:12812
load:0x40080400,len:3032
entry 0x400805e4

Brownout detector was triggered

ets Jun  8 2016 00:22:57

rst:0xc (SW_CPU_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:1184
load:0x40078000,len:12812
load:0x40080400,len:3032
entry 0x400805e4

Wakeup was not caused by deep sleep: 0
Stored date:
Oct  1 2022 20:49:38
Compiled date:
Oct  1 2022 20:49:38
First run? no
ets Jun  8 2016 00:22:57

rst:0x8 (TG1WDT_SYS_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:1184
load:0x40078000,len:12812
load:0x40080400,len:3032
entry 0x400805e4

Brownout detector was triggered

ets Jun  8 2016 00:22:57

rst:0xc (SW_CPU_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:1184
load:0x40078000,len:12812
load:0x40080400,len:3032
entry 0x400805e4

Wakeup was not caused by deep sleep: 0
Stored date:
Oct  1 2022 20:49:38
Compiled date:
Oct  1 2022 20:49:38
First run? no
ets Jun  8 2016 00:22:57

rst:0x8 (TG1WDT_SYS_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:1184
load:0x40078000,len:12812
load:0x40080400,len:3032
entry 0x400805e4

Brownout detector was triggered

ets Jun  8 2016 00:22:57

rst:0xc (SW_CPU_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:1184
load:0x40078000,len:12812
load:0x40080400,len:3032
entry 0x400805e4

Wakeup was not caused by deep sleep: 0
Stored date:
Oct  1 2022 20:49:38
Compiled date:
Oct  1 2022 20:49:38
First run? no
ets Jun  8 2016 00:22:57

rst:0x8 (TG1WDT_SYS_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:1184
load:0x40078000,len:12812
load:0x40080400,len:3032
entry 0x400805e4

Brownout detector was triggered

ets Jun  8 2016 00:22:57

rst:0xc (SW_CPU_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:1184
load:0x40078000,len:12812
load:0x40080400,len:3032
entry 0x400805e4

Wakeup was not caused by deep sleep: 0
Stored date:
Oct  1 2022 20:49:38
Compiled date:
Oct  1 2022 20:49:38
First run? no
ets Jun  8 2016 00:22:57

rst:0x8 (TG1WDT_SYS_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:1184
load:0x00000000,len:0
load:0x00000000,len:-53504
ets Jun  8 2016 00:22:57

rst:0x7 (TG0WDT_SYS_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:1184
load:0x40078000,len:12812
load:0x40080400,len:3032
csum err:0x8c!=0x67
ets_main.c 371 
ets Jun  8 2016 00:22:57

rst:0x7 (TG0WDT_SYS_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:1184
load:0x40078000,len:12812
load:0x40080400,len:3032
entry 0x400805e4

Brownout detector was triggered

ets Jun  8 2016 00:22:57

rst:0xc (SW_CPU_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:1184
load:0x40078000,len:12812
load:0x40080400,len:3032
entry 0x400805e4

Wakeup was not caused by deep sleep: 0
Stored date:
Oct  1 2022 20:49:38
Compiled date:
Oct  1 2022 20:49:38
First run? no
ets Jun  8 2016 00:22:57

rst:0x8 (TG1WDT_SYS_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:1184
load:0x40078000,len:12812
load:0x40080400,len:3032
entry 0x400805e4

Brownout detector was triggered

ets Jun  8 2016 00:22:57

rst:0xc (SW_CPU_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:1184
load:0x40078000,len:12812
load:0x40080400,len:3032
entry 0x400805e4

Wakeup was not caused by deep sleep: 0
Stored date:
Oct  1 2022 20:49:38
Compiled date:
Oct  1 2022 20:49:38
First run? no
Camera init success!Camera is working
E (2170) sdmmc_common: sdmmc_init_ocr: send_op_cond (1) returned 0x107
E (2171) vfs_fat_sdmmc: sdmmc_card_init failed (0x107).
SD Card Mount Failed
Set pin 4 high to wake up the ESP32 from deepsleep
exposureTime : 547
gain : 0
timelapseInterval : 0
Appending to file: /debug.txt
Failed to open file for appending
Appending to file: /debug.txt
Failed to open file for appending
Appending to file: /debug.txt
Failed to open file for appending
LED : 255
LED : 255
LED : 0
LED : 0
LENS Value: 255
LENS Value: 0
Appending to file: /debug.txt
Failed to open file for appending
Appending to file: /debug.txt
Failed to open file for appending
In timelapse mode.
Appending to file: /debug.txt
Failed to open file for appending
LENS Value: 0
/anglerfish_35_Z_0
LED : 255
LED : 0
JPG: 6708BLENS Value: 25
/anglerfish_35_Z_25
LED : 255
LED : 0
JPG: 2393BLENS Value: 50
/anglerfish_35_Z_50
LED : 255
LED : 0
JPG: 2386BLENS Value: 75
/anglerfish_35_Z_75
LED : 255
LED : 0
JPG: 2385BLENS Value: 100
/anglerfish_35_Z_100
LED : 255
LED : 0
JPG: 2385BLENS Value: 125
/anglerfish_35_Z_125
LED : 255
LED : 0
JPG: 2378BLENS Value: 150
/anglerfish_35_Z_150
LED : 255
LED : 0
JPG: 2392BLENS Value: 175
/anglerfish_35_Z_175
LED : 255
LED : 0
JPG: 2392BLENS Value: 200
/anglerfish_35_Z_200
LED : 255
LED : 0
JPG: 2391BLENS Value: 225
/anglerfish_35_Z_225
LED : 255
LED : 0
JPG: 2391BLENS Value: 250
/anglerfish_35_Z_250
LED : 255
LED : 0
JPG: 2393BAppending to file: /debug.txt
Failed to open file for appending
Sleeping for 600 s
ets Jun  8 2016 00:22:57

rst:0x1 (POWERON_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:1184
load:0x40078000,len:12812
load:0x40080400,len:3032
entry 0x400805e4

Wakeup was not caused by deep sleep: 0
Stored date:
Oct  1 2022 20:49:38
Compiled date:
Oct  1 2022 20:49:38
First run? no
Camera init success!Camera is working
E (1172) sdmmc_common: sdmmc_init_ocr: send_op_cond (1) returned 0x107
E (1173) vfs_fat_sdmmc: sdmmc_card_init failed (0x107).
SD Card Mount Failed
Set pin 4 high to wake up the ESP32 from deepsleep
exposureTime : 547
gain : 0
timelapseInterval : 0
Appending to file: /debug.txt
Failed to open file for appending
Appending to file: /debug.txt
Failed to open file for appending
Appending to file: /debug.txt
Failed to open file for appending
LED : 255
LED : 255
LED : 0
LED : 0
LENS Value: 255
LENS Value: 0
Appending to file: /debug.txt
Failed to open file for appending
Appending to file: /debug.txt
Failed to open file for appending
In timelapse mode.
Appending to file: /debug.txt
Failed to open file for appending
LENS Value: 0
/anglerfish_35_Z_0
LED : 255
LED : 0
JPG: 7177BLENS Value: 25
/anglerfish_35_Z_25
LED : 255
LED : 0
JPG: 2381BLENS Value: 50
/anglerfish_35_Z_50
LED : 255
LED : 0
JPG: 2379BLENS Value: 75
/anglerfish_35_Z_75
LED : 255
LED : 0
JPG: 2391BLENS Value: 100
/anglerfish_35_Z_100
LED : 255
LED : 0
JPG: 2391BLENS Value: 125
/anglerfish_35_Z_125
LED : 255
LED : 0
JPG: 2391BLENS Value: 150
/anglerfish_35_Z_150
LED : 255
LED : 0
JPG: 2391BLENS Value: 175
/anglerfish_35_Z_175
LED : 255
LED : 0
JPG: 2391BLENS Value: 200
/anglerfish_35_Z_200
LED : 255
LED : 0
JPG: 2388BLENS Value: 225
/anglerfish_35_Z_225
LED : 255
LED : 0
JPG: 2388BLENS Value: 250
/anglerfish_35_Z_250
LED : 255
LED : 0
JPG: 2388BAppending to file: /debug.txt
Failed to open file for appending
Sleeping for 600 s
ets Jun  8 2016 00:22:57

rst:0x1 (POWERON_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:1184
load:0x40078000,len:12812
load:0x40080400,len:3032
entry 0x400805e4

Wakeup was not caused by deep sleep: 0
Stored date:
Oct  1 2022 20:49:38
Compiled date:
Oct  1 2022 20:49:38
First run? no
Camera init success!Camera is working
E (1172) sdmmc_common: sdmmc_init_ocr: send_op_cond (1) returned 0x107
E (1173) vfs_fat_sdmmc: sdmmc_card_init failed (0x107).
SD Card Mount Failed
Set pin 4 high to wake up the ESP32 from deepsleep
exposureTime : 547
gain : 0
timelapseInterval : 0
Appending to file: /debug.txt
Failed to open file for appending
Appending to file: /debug.txt
Failed to open file for appending
Appending to file: /debug.txt
Failed to open file for appending
LED : 255
LED : 255
LED : 0
LED : 0
LENS Value: 255
LENS Value: 0
Appending to file: /debug.txt
Failed to open file for appending
Appending to file: /debug.txt
Failed to open file for appending
In timelapse mode.
Appending to file: /debug.txt
Failed to open file for appending
LENS Value: 0
/anglerfish_35_Z_0
LED : 255
LED : 0
JPG: 6906BLENS Value: 25
/anglerfish_35_Z_25
LED : 255
LED : 0
JPG: 2353BLENS Value: 50
/anglerfish_35_Z_50
LED : 255
LED : 0
JPG: 2353BLENS Value: 75
/anglerfish_35_Z_75
LED : 255
LED : 0
JPG: 2350BLENS Value: 100
/anglerfish_35_Z_100
LED : 255
LED : 0
JPG: 2339BLENS Value: 125
/anglerfish_35_Z_125
LED : 255
LED : 0
JPG: 2339BLENS Value: 150
/anglerfish_35_Z_150
LED : 255
LED : 0
JPG: 2355BLENS Value: 175
/anglerfish_35_Z_175
LED : 255
LED : 0
JPG: 2355BLENS Value: 200
/anglerfish_35_Z_200
LED : 255
LED : 0
JPG: 2355BLENS Value: 225
/anglerfish_35_Z_225
LED : 255
LED : 0
JPG: 2355BLENS Value: 250
/anglerfish_35_Z_250
LED : 255
LED : 0
JPG: 2355BAppending to file: /debug.txt
Failed to open file for appending
Sleeping for 600 s

*/