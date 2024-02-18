# Modbus Load Cell Signal Conditioner

## Overview
This program implements a signal conditioner for 4 load cells capable of sharing data via Modbus protocol. It provides functionalities for calibration, taring, and logging.

## Features
- Signal conditioning for 4 load cells
- Data sharing via Modbus protocol
- Calibration and taring functionalities
- Multithreading logging on microSD card

## Usage
### Modpoll Commands
- **Directory**: `cd C:\Users\fabbr\Downloads\modpoll-3.10\win`
- **Data request**: `modpoll -b 9600 -p none -m rtu -a 1 -r 1 -c 13 COM4`
- **Writing calibration weight**: `modpoll -b 9600 -p none -m rtu -a 1 -r 12 COM4 5000`
- **Requesting tare**: `modpoll -b 9600 -p none -m rtu -a 1 -r 1 -t 0 COM4 1`
- **Requesting calibration**: `modpoll -b 9600 -p none -m rtu -a 1 -r 9 -t 0 COM4 1`

## Issues
- Every other data is discarded to set gain (possible alternate reading?)

## To Do
1. Differentiate disconnected cells from connected cells
2. Modify HX711 library for reading the two channels (not two HX711, but one HX711 with two channels)
3. Manage multithreading logging on microSD card
4. Understand any default situations

## Setup
1. Include necessary libraries
2. Define Modbus parameters
3. Initialize Modbus controller
4. Set up communication parameters
5. Set up register area
6. Start Modbus controller and stack
7. Initialize UART settings

## Protocol Routine
1. Set up signal conditioning for each load cell
2. Define filtering algorithm and memory saving system
3. Continuously read load cell data and update internal registers
4. Handle calibration and taring commands
5. Log requests on the display (optional)

## Authors
- **Simone Fabbri**

