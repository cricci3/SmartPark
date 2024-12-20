# 🚗 SmartPark - Intelligent Parking Management System

## 📝 Project Overview

SmartPark is an advanced IoT-based parking management system that leverages edge computing principles to provide real-time parking space monitoring and management. Developed using Arduino technology, the system offers an efficient solution for modern parking facilities.

### Key Features

- Real-time parking spot occupancy detection
- Multi-floor parking management
- Visual status indicators for different types of parking spots
- Web-based monitoring dashboard
- MQTT-based communication architecture
- Fault-tolerant design

<p align="center">
  <img src="https://github.com/cricci3/SmartPark/blob/main/images/Iot.png" width="800">
</p>

## 🔧 Components and Hardware

### Core Components

1. **Arduino Portenta H7**
   - Main controller for each floor and entrance
   - Features:
     - Dual-core processor (Cortex-M7 and M4)
     - Built-in WiFi module
     - Real-time operating system support
     - Multiple I/O interfaces

2. **VL53L0X Time-of-Flight Sensor**
   - Used for vehicle detection
   - Specifications:
     - Operating range: up to 2m
     - Accuracy: ±3%
     - Response time: 23ms
     - I2C interface
     - Low power consumption

3. **TCA9548A I2C Multiplexer**
   - Manages multiple ToF sensors
   - Features:
     - 8 bidirectional switches
     - 8 configurable addresses
     - Hot-swappable I2C connections
     - Low standby current

4. **RGB LEDs**
   - Status indicators for parking spots
   - Specifications:
     - Common cathode configuration
     - Operating voltage: 3.3V
     - Color states:
       - 🟢 Green: Regular spots
       - 🟡 Yellow: Handicap spots
       - 🟣 Violet: E-charge stations
       - 🔴 Red: Occupied (any type)

5. **OLED Display**
   - Entrance information display
   - Specifications:
     - Resolution: 128x64 pixels
     - I2C interface
     - Screen size: 0.96 inches
     - High contrast ratio

### Additional Hardware

- **Power Supply**: 5V DC power supply for each floor controller
- **Connection Cables**: 
  - I2C cables for sensors
  - Power distribution cables
  - LED connection wires
- **Mounting Hardware**:
  - Sensor mounting brackets
  - LED holders
  - Controller enclosures
- **Network Infrastructure**:
  - WiFi access points
  - Network switches
  - Ethernet cables (optional)

## 🏗 System Architecture

### Hardware Components Organization

- **Floor Level**:
  - 3x ToF sensors per floor
  - 3x RGB LEDs per floor
  - 1x Arduino Portenta H7
  - 1x I2C multiplexer
  - Power distribution system

- **Entrance Level**:
  - 1x Arduino Portenta H7
  - 1x OLED display
  - Network connection components

### Software Architecture

The system consists of three main components:

1. **Floor Controllers**
   - Monitor parking spot sensors
   - Manage LED status indicators
   - Publish status updates via MQTT
   - Handle fault detection and recovery

2. **Entrance Display Controller**
   - Subscribes to floor status updates
   - Manages OLED display
   - Provides real-time availability information

3. **Web Application Interface**
   - Real-time monitoring dashboard
   - WebSocket-based MQTT communication
   - Visual representation of parking status

## 🛠 Technical Implementation

### Communication Protocol

- MQTT broker: test.mosquitto.org:1883
- Topics structure:  ```parking/floorX ``` (where X is the floor number)
- Message format: comma-separated values representing stall states
- Update frequency: 2.5 seconds

### Sensor Implementation

- Detection range: 10mm - 50mm
- I2C communication with multiplexer
- Continuous monitoring with error checking

### Fault Tolerance

- Watchdog timer implementation (5000ms timeout)
- Automatic system recovery
- Independent floor operation
- Graceful degradation support

## 🚀 Getting Started

### Prerequisites

- Arduino IDE with Portenta H7 board support
- Required Libraries:
  - MQTT Client
  - Adafruit_VL53L0X
  - Adafruit_SSD1306
  - mbed RTOS

### Installation

1. Clone the repository:
   ```bash
   git clone https://github.com/cricci3/SmartPark
   ```

2. Install required libraries through Arduino Library Manager

3. Configure network settings:
   - Update WiFi credentials
   - Set MQTT broker address if needed

4. Upload the code:
   - Floor controller code to floor Arduinos
   - Display controller code to entrance Arduino
   - Deploy web application

## 📊 System Status and Monitoring

The system provides multiple ways to monitor parking status:

1. **Physical Indicators**
   - LED status lights at each parking spot
   - OLED display at entrance showing availability

2. **Web Dashboard**
   - Real-time status updates
   - Floor-wise availability view
   - Spot type differentiation

## 📄 License

This project is licensed under the MIT License - see the LICENSE file for details.

## ✍️ Authors

- Giacomo Bersani
- Alessandro Cassani
- Damiano Ficara
- Claudio Ricci

