EESchema Schematic File Version 2
LIBS:power
LIBS:device
LIBS:transistors
LIBS:conn
LIBS:linear
LIBS:regul
LIBS:74xx
LIBS:cmos4000
LIBS:adc-dac
LIBS:memory
LIBS:xilinx
LIBS:microcontrollers
LIBS:dsp
LIBS:microchip
LIBS:analog_switches
LIBS:motorola
LIBS:texas
LIBS:intel
LIBS:audio
LIBS:interface
LIBS:digital-audio
LIBS:philips
LIBS:display
LIBS:cypress
LIBS:siliconi
LIBS:opto
LIBS:atmel
LIBS:contrib
LIBS:valves
LIBS:pc_connectors
LIBS:osc
LIBS:cat644-cache
EELAYER 25 0
EELAYER END
$Descr USLetter 11000 8500
encoding utf-8
Sheet 4 4
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
L GND #PWR022
U 1 1 581479B3
P 9700 2000
F 0 "#PWR022" H 9700 1750 50  0001 C CNN
F 1 "GND" H 9700 1850 50  0000 C CNN
F 2 "" H 9700 2000 50  0000 C CNN
F 3 "" H 9700 2000 50  0000 C CNN
	1    9700 2000
	1    0    0    -1  
$EndComp
Text HLabel 8900 2700 0    60   Input ~ 0
VGA_RED
Text HLabel 8900 2900 0    60   Input ~ 0
VGA_BLUE
Text HLabel 8900 2800 0    60   Input ~ 0
VGA_GREEN
Text HLabel 8900 2600 0    60   Input ~ 0
HSYNC
Text HLabel 8900 2500 0    60   Input ~ 0
VSYNC
Text HLabel 3000 5250 0    60   Input ~ 0
AUDIO_OUT
$Comp
L C C6
U 1 1 58149D3D
P 8900 2150
F 0 "C6" H 8925 2250 50  0000 L CNN
F 1 "0.01uF" H 8925 2050 50  0000 L CNN
F 2 "Capacitors_ThroughHole:C_Disc_D6_P5" H 8938 2000 50  0001 C CNN
F 3 "" H 8900 2150 50  0000 C CNN
	1    8900 2150
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR023
U 1 1 5814AFAE
P 3350 4850
F 0 "#PWR023" H 3350 4600 50  0001 C CNN
F 1 "GND" H 3350 4700 50  0000 C CNN
F 2 "" H 3350 4850 50  0000 C CNN
F 3 "" H 3350 4850 50  0000 C CNN
	1    3350 4850
	1    0    0    -1  
$EndComp
Wire Wire Line
	8900 2700 9200 2700
Wire Wire Line
	8900 2800 9200 2800
Wire Wire Line
	8900 2900 9200 2900
$Comp
L CONN_01X06 P8
U 1 1 5818830C
P 9400 2650
F 0 "P8" H 9400 3000 50  0000 C CNN
F 1 "VGA" V 9500 2650 50  0000 C CNN
F 2 "Pin_Headers:Pin_Header_Angled_1x06" H 9400 2650 50  0001 C CNN
F 3 "" H 9400 2650 50  0000 C CNN
	1    9400 2650
	1    0    0    -1  
$EndComp
$Comp
L CONN_01X02 P7
U 1 1 581886A6
P 3850 5200
F 0 "P7" H 3850 5350 50  0000 C CNN
F 1 "AUD" V 3950 5200 50  0000 C CNN
F 2 "Pin_Headers:Pin_Header_Angled_1x02" H 3850 5200 50  0001 C CNN
F 3 "" H 3850 5200 50  0000 C CNN
	1    3850 5200
	1    0    0    -1  
$EndComp
Wire Wire Line
	3650 5250 3000 5250
Wire Wire Line
	3350 4850 3650 4850
Wire Wire Line
	3650 4850 3650 5150
Wire Wire Line
	8900 2000 9700 2000
Wire Wire Line
	9200 2000 9200 2400
Wire Wire Line
	8900 2500 9200 2500
Wire Wire Line
	9200 2600 8900 2600
Wire Wire Line
	8900 2300 8900 2400
Wire Wire Line
	8900 2400 9000 2400
Wire Wire Line
	9000 2400 9000 2500
Connection ~ 9000 2500
Connection ~ 9200 2000
$EndSCHEMATC
