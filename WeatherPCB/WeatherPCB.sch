EESchema Schematic File Version 4
EELAYER 26 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title "Weather Station PCB"
Date ""
Rev "1.0"
Comp "github.com/mwyoung"
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L wemos_mini:WeMos_mini U?
U 1 1 5D654E45
P 3200 3500
F 0 "U?" H 3200 4000 60  0000 C CNN
F 1 "WeMos_mini" H 3200 3000 60  0000 C CNN
F 2 "" H 3750 2800 60  0000 C CNN
F 3 "https://wiki.wemos.cc/_media/products:d1:sch_d1_mini_v3.0.0.pdf" H 3200 4031 60  0001 C CNN
	1    3200 3500
	1    0    0    -1  
$EndComp
$Comp
L WeatherPCB_Components:TFTLCD_ST7735R U?
U 1 1 5D6562B8
P 1550 3400
F 0 "U?" H 1525 3965 50  0000 C CNN
F 1 "TFTLCD_ST7735R" H 1525 3874 50  0000 C CNN
F 2 "" H 1950 3800 50  0001 C CNN
F 3 "" H 1950 3800 50  0001 C CNN
F 4 "https://www.amazon.com/HiLetgo-ST7735R-128160-Display-Arduino/dp/B00LSG51MM" H 1900 4250 50  0001 C CNN "Where to Buy"
	1    1550 3400
	1    0    0    -1  
$EndComp
NoConn ~ 900  3200
NoConn ~ 900  3300
NoConn ~ 900  3400
NoConn ~ 900  3500
NoConn ~ 900  3600
NoConn ~ 900  3700
NoConn ~ 900  3800
Wire Wire Line
	3700 3150 3700 2750
Wire Wire Line
	3700 2750 2350 2750
Wire Wire Line
	2350 2750 2350 3200
Wire Wire Line
	2350 3200 2150 3200
Wire Wire Line
	2350 3200 2350 3800
Wire Wire Line
	2350 3800 2150 3800
Connection ~ 2350 3200
Wire Wire Line
	2700 3250 2450 3250
Wire Wire Line
	2450 3250 2450 3100
Wire Wire Line
	2450 3100 2150 3100
Wire Wire Line
	2150 3900 2450 3900
Wire Wire Line
	2450 3900 2450 3250
Connection ~ 2450 3250
Wire Wire Line
	2150 3700 2550 3700
Wire Wire Line
	2550 3700 2550 4250
Wire Wire Line
	2550 4250 3850 4250
Wire Wire Line
	3850 4250 3850 3250
Wire Wire Line
	3850 3250 3700 3250
Wire Wire Line
	2150 3500 2600 3500
Wire Wire Line
	2600 3500 2600 4200
Wire Wire Line
	2600 4200 3800 4200
Wire Wire Line
	3800 4200 3800 3350
Wire Wire Line
	3800 3350 3700 3350
Wire Wire Line
	3750 4150 2250 4150
Wire Wire Line
	2250 4150 2250 3600
Wire Wire Line
	2250 3600 2150 3600
Wire Wire Line
	2150 3300 2700 3300
Wire Wire Line
	2700 3300 2700 3350
Wire Wire Line
	2150 3400 2650 3400
Wire Wire Line
	2650 3400 2650 3450
Wire Wire Line
	2650 3450 2700 3450
NoConn ~ 3700 3450
NoConn ~ 3700 3750
NoConn ~ 3700 3850
NoConn ~ 2700 3750
NoConn ~ 2700 3650
NoConn ~ 2700 3550
NoConn ~ 2700 3150
NoConn ~ 2700 3850
Text Notes 1300 4750 0    50   ~ 0
PIN MAP\nLCD----ESP8266 (GPIO)\nRESET---D4 (2,LED)\nCS-----D8 (15,SS)\nA0/DC--D3 (0)\nSDA----D7 (13,MOSI)\nSCL----D5 (14,SCK)
Wire Wire Line
	3750 3550 3700 3550
Wire Wire Line
	3750 3550 3750 4150
NoConn ~ 3700 3650
$EndSCHEMATC
