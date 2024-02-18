# MQTT Publish and Subscribe Example with AWS IoT on ESP32

This example demonstrates a simple MQTT publish and subscribe functionality on the same topic using the AWS IoT MQTT Platform with an ESP32 development board.

## Overview

The program establishes a connection to the AWS IoT MQTT Platform and performs both publishing and subscribing operations on the topic "sdk/test/js". It periodically publishes messages to the topic, demonstrating both Quality of Service (QoS) 0 and QoS 1 message publishing.

## Features

- Establishes WiFi connection using provided credentials.
- Connects to the AWS IoT MQTT Platform.
- Subscribes to the topic "sdk/test/js".
- Publishes messages to the same topic periodically.
- Demonstrates both QoS 0 and QoS 1 message publishing.

## Requirements

- ESP32 development board
- WiFi connection credentials (SSID and password)
- AWS IoT configuration (host URL, port, certificates)

## Setup

1. Configure WiFi credentials in the code:

   ```c
   #define EXAMPLE_WIFI_SSID "YourWiFiSSID"
   #define EXAMPLE_WIFI_PASS "YourWiFiPassword"
