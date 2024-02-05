# Libraries
Most of the libraries provided are object-oriented implementations of specific Daisy features.
## HX711
Cpp library to communicate with the onboard ADCs, setting up the requested gain of the PGA and reading data from the connected load cells at 80 samples per second.
## Scale
Often, a weighing scale is made up of multiple load cells. For this reason, we created a library to manage this type of aggregate solution.
Cpp library to control several HX711 objects simultaneously, reading the load cells' values in an aggregate way, with the possibility of tare and calibration of the weighing scale.
## Modbus
C library to implement the Modbus protocol on an ESP32.
## SPIFFS
Cpp library that provides functions to write and read from the onboard permanent memory of the Daisy. 
This can be useful to store data, like calibration and offset values.
