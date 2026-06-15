# Tron-Dance
# ESP32 Synchronized LED Suit System

## Overview

This project is a wireless LED costume control system designed using multiple ESP32 microcontrollers and WS2812B addressable LED strips. A master ESP32 sends commands via ESP-NOW to multiple receiver ESP32 boards embedded in separate suits, allowing all suits to perform synchronized lighting animations in real time.

The system was developed to create coordinated visual effects for performances, events, and stage shows without relying on wired communication.

## Features

* Wireless communication using **ESP-NOW**
* Master-slave architecture with **1 transmitter and 4 receiver suits**
* Real-time synchronized LED animations
* Multiple animation patterns and color effects
* Immediate start/stop control across all suits
* Non-blocking delay mechanism for responsive command handling
* Modular code structure for easy customization and expansion

## Hardware Used

* ESP32 Development Boards
* WS2812B Addressable LED Strips
* Portable power sources
* Connecting wires and costume framework

## Project Structure

* `Master.ino`
  Controls the entire system by transmitting commands to all receiver suits.

* `suit_1.ino`
  Receiver code for Suit 1.

* `suit_2.ino`
  Receiver code for Suit 2.

* `suit_3.ino`
  Receiver code for Suit 3.

* `suit_4.ino`
  Receiver code for Suit 4.

## Working Principle

1. The master ESP32 initializes ESP-NOW and registers all receiver ESP32 devices using their MAC addresses.
2. Commands are transmitted wirelessly to the receiver suits.
3. Each suit continuously listens for incoming commands.
4. Upon receiving a start command, all suits execute the predefined LED animations simultaneously.
5. A stop command immediately halts all ongoing effects.

## Software and Libraries

* Arduino IDE
* FastLED Library
* ESP-NOW Library
* WiFi Library (ESP32)

## Applications

* Dance performances
* Stage productions
* Event entertainment
* Interactive costume design
* Synchronized wearable technology demonstrations

## Future Improvements

* Mobile application integration
* Custom animation selection interface
* Music-reactive lighting effects
* Additional suits and scalability improvements
* Battery monitoring and power optimization

This project was developed as part of my exploration of embedded systems, wireless communication, and interactive wearable technology.
