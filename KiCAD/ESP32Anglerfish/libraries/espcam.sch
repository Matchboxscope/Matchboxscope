EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L Connector_Generic:Conn_01x08 J1
U 1 1 600C0C13
P 4750 3800
F 0 "J1" H 4668 4320 50  0000 C CNN
F 1 "Conn1" H 4668 4227 50  0000 C CNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_1x08_P2.54mm_Vertical" H 4750 3800 50  0001 C CNN
F 3 "~" H 4750 3800 50  0001 C CNN
	1    4750 3800
	-1   0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_01x08 J2
U 1 1 600C0C77
P 6650 3800
F 0 "J2" H 6730 3792 50  0000 L CNN
F 1 "Conn2" H 6730 3699 50  0000 L CNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_1x08_P2.54mm_Vertical" H 6650 3800 50  0001 C CNN
F 3 "~" H 6650 3800 50  0001 C CNN
	1    6650 3800
	1    0    0    -1  
$EndComp
Wire Wire Line
	4950 4100 5100 4100
Wire Wire Line
	4950 4200 5000 4200
Wire Wire Line
	4950 4000 5150 4000
Wire Wire Line
	4950 3900 5150 3900
Wire Wire Line
	4950 3800 5150 3800
Wire Wire Line
	4950 3700 5150 3700
Wire Wire Line
	5150 3600 4950 3600
Wire Wire Line
	4950 3500 5150 3500
Wire Wire Line
	6250 3500 6400 3500
Wire Wire Line
	6250 3600 6450 3600
Wire Wire Line
	6250 3700 6450 3700
Wire Wire Line
	6250 3800 6450 3800
Wire Wire Line
	6250 3900 6400 3900
Wire Wire Line
	6250 4000 6450 4000
Wire Wire Line
	6250 4100 6450 4100
Wire Wire Line
	6250 4200 6300 4200
Wire Wire Line
	5100 4100 5100 4400
Wire Wire Line
	5100 4400 5700 4400
Wire Wire Line
	6400 4400 6400 3900
Connection ~ 5100 4100
Wire Wire Line
	5100 4100 5150 4100
Connection ~ 6400 3900
Wire Wire Line
	6400 3900 6450 3900
Wire Wire Line
	6400 3900 6400 3500
Connection ~ 6400 3500
Wire Wire Line
	6400 3500 6450 3500
$Comp
L power:GND #PWR0101
U 1 1 600C3097
P 5700 4450
F 0 "#PWR0101" H 5700 4200 50  0001 C CNN
F 1 "GND" H 5705 4273 50  0000 C CNN
F 2 "" H 5700 4450 50  0001 C CNN
F 3 "" H 5700 4450 50  0001 C CNN
	1    5700 4450
	1    0    0    -1  
$EndComp
$Comp
L power:+3.3V #PWR0102
U 1 1 600C30C9
P 900 700
F 0 "#PWR0102" H 900 550 50  0001 C CNN
F 1 "+3.3V" H 915 877 50  0000 C CNN
F 2 "" H 900 700 50  0001 C CNN
F 3 "" H 900 700 50  0001 C CNN
	1    900  700 
	1    0    0    -1  
$EndComp
$Comp
L power:+5V #PWR0103
U 1 1 600C3149
P 900 1250
F 0 "#PWR0103" H 900 1100 50  0001 C CNN
F 1 "+5V" H 915 1427 50  0000 C CNN
F 2 "" H 900 1250 50  0001 C CNN
F 3 "" H 900 1250 50  0001 C CNN
	1    900  1250
	1    0    0    -1  
$EndComp
Wire Wire Line
	6300 4200 6300 4300
Wire Wire Line
	900  800  900  750 
Connection ~ 6300 4200
Wire Wire Line
	6300 4200 6450 4200
Wire Wire Line
	900  1250 900  1300
Wire Wire Line
	5000 4300 5000 4200
Connection ~ 5000 4200
Wire Wire Line
	5000 4200 5150 4200
$Comp
L power:PWR_FLAG #FLG0101
U 1 1 600C39AF
P 900 1350
F 0 "#FLG0101" H 900 1425 50  0001 C CNN
F 1 "PWR_FLAG" H 900 1527 50  0000 C CNN
F 2 "" H 900 1350 50  0001 C CNN
F 3 "~" H 900 1350 50  0001 C CNN
	1    900  1350
	-1   0    0    1   
$EndComp
$Comp
L power:PWR_FLAG #FLG0102
U 1 1 600C39CD
P 900 800
F 0 "#FLG0102" H 900 875 50  0001 C CNN
F 1 "PWR_FLAG" H 900 977 50  0000 C CNN
F 2 "" H 900 800 50  0001 C CNN
F 3 "~" H 900 800 50  0001 C CNN
	1    900  800 
	-1   0    0    1   
$EndComp
Text GLabel 6300 4300 0    50   Input ~ 0
3.3V
Text GLabel 850  750  0    50   Input ~ 0
3.3V
Wire Wire Line
	850  750  900  750 
Connection ~ 900  750 
Wire Wire Line
	900  750  900  700 
Text GLabel 850  1300 0    50   Input ~ 0
5V
Text GLabel 5000 4300 0    50   Input ~ 0
5V
Wire Wire Line
	900  1300 850  1300
Connection ~ 900  1300
Wire Wire Line
	900  1300 900  1350
Wire Wire Line
	5700 4450 5700 4400
Connection ~ 5700 4400
Wire Wire Line
	5700 4400 6400 4400
$Comp
L espcam:ESPCAM U1
U 1 1 600F913F
P 5700 3400
F 0 "U1" H 5700 3821 50  0000 C CNN
F 1 "ESPCAM" H 5700 3728 50  0000 C CNN
F 2 "espcam:ESPCAM_bottom" H 5700 3400 50  0001 C CNN
F 3 "" H 5700 3400 50  0001 C CNN
	1    5700 3400
	1    0    0    -1  
$EndComp
$EndSCHEMATC
