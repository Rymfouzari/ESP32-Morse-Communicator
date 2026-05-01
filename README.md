# Morse IoT Project

An ESP32-based project that sends and receives Morse code using a LED, LDR sensor, and buzzer, integrated with Arduino IoT Cloud.

## Features

* Send text messages as Morse code (LED + buzzer)
* Receive Morse code via LDR (light sensor)
* Real-time Morse decoding
* Keyword detection:

  * ANNIVERSAIRE → birthday melody
  * MARIAGE → wedding melody
  * FELICITATIONS → congratulations melody
  * NOEL → Christmas melody
* Automatic melody playback based on received message
* Remote interaction via Arduino IoT Cloud

## Hardware Requirements

* ESP32
* LED
* LDR (photoresistor)
* Buzzer
* Appropriate resistors

## How It Works

* Text messages are converted to Morse code and transmitted using light and sound signals
* The LDR detects incoming light pulses
* Signal duration determines dots and dashes
* Morse sequences are decoded into characters and reconstructed into full messages
* Keywords trigger specific melodies

## Notes

* Timing constants (DOT_TIME, DASH_TIME, etc.) are critical for proper decoding
* Light threshold (LIGHT_THRESHOLD) may require calibration depending on ambient light conditions

## Security

Do NOT upload sensitive credentials (Wi-Fi, Arduino Cloud keys).
Make sure files like `thingProperties.h` are excluded from version control if they contain secrets.
