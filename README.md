#  🚗 🅿️ Smart Park: An Arduino-based Smart Parking System

Welcome to the Smart Park project! This repository contains the code and documentation for a smart parking system developed as part of the Edge Computing course. The system uses Arduino, time-of-flight sensors, and MQTT to create an efficient, real-time parking management solution.

## Overview

The Smart Park system monitors parking spots and provides real-time status updates. It uses:

- Time-of-flight sensors: To detect if a car is occupying a parking spot.

- LED indicators to visually display the parking status:

  - 🅿️ Green LED 🟢: Parking spot is available

  - ♿️ Yellow LED 🟡: disabled Parking spot is available 

  - 🔋 Violet LED 🟣: E-charge station is available 

  - 🚫 Red LED 🔴: Parking spot is occupied 

- MQTT protocol: To enable communication between multiple Arduinos in the system.

This project showcases edge computing principles by processing parking spot data locally on the Arduino devices, while coordinating state updates across the system using MQTT.

<p align="center">
  <img src="SmartPark/images/IoT.png" width="500">
</p>
