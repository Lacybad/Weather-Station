EESchema Schematic File Version 4
LIBS:WeatherPCB-cache
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
L wemos_mini:WeMos_mini U1
U 1 1 5D654E45
P 3200 3500
F 0 "U1" H 3200 4000 60  0000 C CNN
F 1 "WeMos_mini" H 3200 3000 60  0000 C CNN
F 2 "" H 3750 2800 60  0000 C CNN
F 3 "https://wiki.wemos.cc/_media/products:d1:sch_d1_mini_v3.0.0.pdf" H 3200 4031 60  0001 C CNN
	1    3200 3500
	1    0    0    -1  
$EndComp
$Comp
L WeatherPCB_Components:TFTLCD_ST7735R U2
U 1 1 5D6562B8
P 5100 3550
F 0 "U2" H 5075 4115 50  0000 C CNN
F 1 "TFTLCD_ST7735R" H 5075 4024 50  0000 C CNN
F 2 "" H 5500 3950 50  0001 C CNN
F 3 "" H 5500 3950 50  0001 C CNN
F 4 "https://www.amazon.com/HiLetgo-ST7735R-128160-Display-Arduino/dp/B00LSG51MM" H 5450 4400 50  0001 C CNN "Where to Buy"
	1    5100 3550
	-1   0    0    -1  
$EndComp
NoConn ~ 3700 3850
NoConn ~ 2700 3750
NoConn ~ 2700 3650
NoConn ~ 2700 3550
NoConn ~ 2700 3850
Text Notes 2950 2750 0    50   ~ 0
PIN MAP\nLCD----ESP8266 (Old)\nRESET---D6 (D4)\nCS-----D8\nA0/DC--D0 (D3)\nSDA----D7\nSCL----D5
NoConn ~ 5750 3350
NoConn ~ 5750 3450
NoConn ~ 5750 3550
NoConn ~ 5750 3650
NoConn ~ 5750 3750
NoConn ~ 5750 3850
NoConn ~ 5750 3950
$Comp
L power:GND #PWR01
U 1 1 5D69B9AA
P 2300 3100
F 0 "#PWR01" H 2300 2850 50  0001 C CNN
F 1 "GND" H 2305 2927 50  0000 C CNN
F 2 "" H 2300 3100 50  0001 C CNN
F 3 "" H 2300 3100 50  0001 C CNN
	1    2300 3100
	1    0    0    -1  
$EndComp
$Comp
L power:+5V #PWR02
U 1 1 5D69BA3D
P 2600 3100
F 0 "#PWR02" H 2600 2950 50  0001 C CNN
F 1 "+5V" H 2615 3273 50  0000 C CNN
F 2 "" H 2600 3100 50  0001 C CNN
F 3 "" H 2600 3100 50  0001 C CNN
	1    2600 3100
	1    0    0    -1  
$EndComp
$Comp
L power:+3.3V #PWR03
U 1 1 5D69BAB3
P 3850 3100
F 0 "#PWR03" H 3850 2950 50  0001 C CNN
F 1 "+3.3V" H 3865 3273 50  0000 C CNN
F 2 "" H 3850 3100 50  0001 C CNN
F 3 "" H 3850 3100 50  0001 C CNN
	1    3850 3100
	1    0    0    -1  
$EndComp
Wire Wire Line
	3700 3150 3850 3150
Wire Wire Line
	3850 3150 3850 3100
Wire Wire Line
	2700 3150 2600 3150
Wire Wire Line
	2600 3150 2600 3100
Wire Wire Line
	2700 3250 2500 3250
Wire Wire Line
	2500 3250 2500 3050
Wire Wire Line
	2500 3050 2300 3050
Wire Wire Line
	2300 3050 2300 3100
Wire Wire Line
	3700 3250 4050 3250
Wire Wire Line
	4050 3250 4050 3850
Wire Wire Line
	4050 3850 4500 3850
Wire Wire Line
	4500 3750 3950 3750
Wire Wire Line
	3950 3750 3950 3550
Wire Wire Line
	3950 3550 3700 3550
Wire Wire Line
	3700 3650 4100 3650
Wire Wire Line
	4100 3650 4100 3550
Wire Wire Line
	4100 3550 4500 3550
Wire Wire Line
	3700 3450 4500 3450
Wire Wire Line
	3700 3350 4150 3350
Wire Wire Line
	4150 3350 4150 3650
Wire Wire Line
	4150 3650 4500 3650
$Comp
L power:GND #PWR04
U 1 1 5D69CD81
P 4250 3200
F 0 "#PWR04" H 4250 2950 50  0001 C CNN
F 1 "GND" H 4255 3027 50  0000 C CNN
F 2 "" H 4250 3200 50  0001 C CNN
F 3 "" H 4250 3200 50  0001 C CNN
	1    4250 3200
	1    0    0    -1  
$EndComp
$Comp
L power:+5V #PWR05
U 1 1 5D69CD90
P 4450 3200
F 0 "#PWR05" H 4450 3050 50  0001 C CNN
F 1 "+5V" H 4465 3373 50  0000 C CNN
F 2 "" H 4450 3200 50  0001 C CNN
F 3 "" H 4450 3200 50  0001 C CNN
	1    4450 3200
	1    0    0    -1  
$EndComp
Wire Wire Line
	4500 3250 4450 3250
Wire Wire Line
	4450 3250 4450 3200
Wire Wire Line
	4500 3350 4350 3350
Wire Wire Line
	4350 3350 4350 3150
Wire Wire Line
	4350 3150 4250 3150
Wire Wire Line
	4250 3150 4250 3200
$Comp
L power:GND #PWR06
U 1 1 5D69D756
P 4400 4150
F 0 "#PWR06" H 4400 3900 50  0001 C CNN
F 1 "GND" H 4405 3977 50  0000 C CNN
F 2 "" H 4400 4150 50  0001 C CNN
F 3 "" H 4400 4150 50  0001 C CNN
	1    4400 4150
	1    0    0    -1  
$EndComp
Wire Wire Line
	4400 4150 4400 4050
Wire Wire Line
	4400 4050 4500 4050
$Comp
L digikey-kicad-library:2N4401-ND Q2
U 1 1 5D69BEDE
P 2400 4900
F 0 "Q2" H 2587 4953 60  0000 L CNN
F 1 "2N4401-ND" H 1850 4700 60  0000 L CNN
F 2 "digikey-footprints:TO-92-3" H 2666 5278 60  0001 L CNN
F 3 "" H 2600 5200 60  0001 L CNN
	1    2400 4900
	1    0    0    -1  
$EndComp
NoConn ~ 2700 3350
Text Notes 2200 3400 0    50   ~ 0
BUILTIN_LED
Connection ~ 2300 3100
Wire Wire Line
	2300 3100 2300 3150
$Comp
L digikey-kicad-library:2N4403-ND Q1
U 1 1 5D69C8C7
P 3150 4700
F 0 "Q1" H 3337 4753 60  0000 L CNN
F 1 "2N4403-ND" H 2950 4400 60  0000 L CNN
F 2 "digikey-footprints:TO-92-3" H 3450 5000 60  0001 L CNN
F 3 "" H 3350 5000 60  0001 L CNN
	1    3150 4700
	1    0    0    -1  
$EndComp
$Comp
L power:+3.3V #PWR08
U 1 1 5D69E044
P 3250 4450
F 0 "#PWR08" H 3250 4300 50  0001 C CNN
F 1 "+3.3V" H 3265 4623 50  0000 C CNN
F 2 "" H 3250 4450 50  0001 C CNN
F 3 "" H 3250 4450 50  0001 C CNN
	1    3250 4450
	1    0    0    -1  
$EndComp
$Comp
L Device:R_US R123
U 1 1 5D69E708
P 2750 4700
F 0 "R123" V 2545 4700 50  0000 C CNN
F 1 "4.7k" V 2636 4700 50  0000 C CNN
F 2 "" V 2790 4690 50  0001 C CNN
F 3 "~" H 2750 4700 50  0001 C CNN
	1    2750 4700
	0    1    1    0   
$EndComp
$Comp
L Device:R_US R142
U 1 1 5D69E765
P 2150 4250
F 0 "R142" H 2082 4204 50  0000 R CNN
F 1 "4.7k" H 2082 4295 50  0000 R CNN
F 2 "" V 2190 4240 50  0001 C CNN
F 3 "~" H 2150 4250 50  0001 C CNN
	1    2150 4250
	-1   0    0    1   
$EndComp
$Comp
L power:GND #PWR09
U 1 1 5D69E835
P 2500 5150
F 0 "#PWR09" H 2500 4900 50  0001 C CNN
F 1 "GND" H 2505 4977 50  0000 C CNN
F 2 "" H 2500 5150 50  0001 C CNN
F 3 "" H 2500 5150 50  0001 C CNN
	1    2500 5150
	1    0    0    -1  
$EndComp
Wire Wire Line
	2500 5150 2500 5100
Wire Wire Line
	2900 4700 2950 4700
Wire Wire Line
	3250 4500 3250 4450
Wire Wire Line
	2150 4100 2150 3450
Wire Wire Line
	2150 4900 2150 4400
Text Notes 2050 5500 0    50   ~ 0
Could use only resistor, or single PNP\nFor safety, using both
Wire Wire Line
	2150 3450 2700 3450
Wire Wire Line
	2500 4700 2600 4700
Wire Wire Line
	3600 3950 4500 3950
$Comp
L Device:R_US R1
U 1 1 5D6B65AE
P 3800 4500
F 0 "R1" H 3868 4546 50  0000 L CNN
F 1 "220" H 3868 4455 50  0000 L CNN
F 2 "" V 3840 4490 50  0001 C CNN
F 3 "~" H 3800 4500 50  0001 C CNN
	1    3800 4500
	1    0    0    -1  
$EndComp
$Comp
L Device:R_US R3
U 1 1 5D6B6647
P 3800 5250
F 0 "R3" H 3868 5296 50  0000 L CNN
F 1 "10k" H 3868 5205 50  0000 L CNN
F 2 "" V 3840 5240 50  0001 C CNN
F 3 "~" H 3800 5250 50  0001 C CNN
	1    3800 5250
	1    0    0    -1  
$EndComp
$Comp
L Device:R_PHOTO R2
U 1 1 5D6B739F
P 3800 4850
F 0 "R2" H 3870 4896 50  0000 L CNN
F 1 "70-36k" H 3870 4805 50  0000 L CNN
F 2 "" V 3850 4600 50  0001 L CNN
F 3 "~" H 3800 4800 50  0001 C CNN
	1    3800 4850
	1    0    0    -1  
$EndComp
Wire Wire Line
	3800 4700 3800 4650
Wire Wire Line
	3800 5100 3800 5050
$Comp
L Device:C_Small C1
U 1 1 5D6B98B3
P 4200 5250
F 0 "C1" H 4292 5296 50  0000 L CNN
F 1 "0.1u" H 4292 5205 50  0000 L CNN
F 2 "" H 4200 5250 50  0001 C CNN
F 3 "~" H 4200 5250 50  0001 C CNN
	1    4200 5250
	1    0    0    -1  
$EndComp
Wire Wire Line
	3800 5050 4200 5050
Wire Wire Line
	4200 5050 4200 5150
Connection ~ 3800 5050
Wire Wire Line
	3800 5050 3800 5000
Wire Wire Line
	3800 3750 3700 3750
Connection ~ 4200 5050
$Comp
L power:+3.3V #PWR07
U 1 1 5D6BCE0A
P 3800 4300
F 0 "#PWR07" H 3800 4150 50  0001 C CNN
F 1 "+3.3V" H 3815 4473 50  0000 C CNN
F 2 "" H 3800 4300 50  0001 C CNN
F 3 "" H 3800 4300 50  0001 C CNN
	1    3800 4300
	1    0    0    -1  
$EndComp
Wire Wire Line
	3800 4350 3800 4300
$Comp
L power:GND #PWR010
U 1 1 5D6BE7BB
P 4000 5500
F 0 "#PWR010" H 4000 5250 50  0001 C CNN
F 1 "GND" H 4005 5327 50  0000 C CNN
F 2 "" H 4000 5500 50  0001 C CNN
F 3 "" H 4000 5500 50  0001 C CNN
	1    4000 5500
	1    0    0    -1  
$EndComp
Wire Wire Line
	4200 5350 4200 5450
Wire Wire Line
	4200 5450 4000 5450
Wire Wire Line
	3800 5450 3800 5400
Wire Wire Line
	4000 5500 4000 5450
Connection ~ 4000 5450
Wire Wire Line
	4000 5450 3800 5450
Text Notes 3600 5900 0    50   ~ 0
Resistor above R_Photo for safety\nCapacitor smooths out ADC voltage
Wire Wire Line
	2200 4900 2150 4900
Wire Wire Line
	4200 5050 4200 4050
Wire Wire Line
	4200 4050 3800 4050
Wire Wire Line
	3800 4050 3800 3750
Wire Wire Line
	3600 3950 3600 4150
Wire Wire Line
	3600 4150 3500 4150
Wire Wire Line
	3500 4150 3500 4950
Wire Wire Line
	3500 4950 3250 4950
Wire Wire Line
	3250 4950 3250 4900
$EndSCHEMATC