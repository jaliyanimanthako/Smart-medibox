# Medibox Simulation Project

## Overview

This project involves creating a Medibox simulation using the Wokwi simulator and Node-RED. The Medibox assists users in managing their medication schedules effectively. It includes functionality for setting alarms, monitoring environmental conditions, and dynamically adjusting light intensity to ensure optimal storage conditions for sensitive medicines.

## Features

### Phase 1

1. **Menu Options**
    - **Set Time Zone**: Allows the user to set the time zone by taking the offset from UTC as input.
    - **Set Alarms**: Users can set up to three alarms.
    - **Disable Alarms**: Users can disable all alarms.

2. **Time Synchronization**
    - Fetch the current time in the selected time zone from an NTP server over Wi-Fi.
    - Display the current time on an OLED screen.

3. **Alarm Functionality**
    - Ring the alarm with proper indication when the set alarm times are reached.
    - Stop the alarm using a push button.

4. **Environmental Monitoring**
    - Monitor temperature and humidity levels.
    - Provide warnings using proper indication when temperature or humidity exceeds healthy limits.
    - Healthy Temperature: 26°C ≤ Temperature ≤ 32°C
    - Healthy Humidity: 60% ≤ Humidity ≤ 80%

### Phase 2

1. **Light Intensity Monitoring**
    - Use two Light Dependent Resistors (LDRs) to measure the light intensity.
    - Display the highest light intensity on the Node-RED dashboard using a gauge and plot past variations.
    - Indicate which LDR (left or right) has the highest light intensity.
    - Normalize light intensity values to a range of 0 to 1.

2. **Shaded Sliding Window Control**
    - Adjust light intensity entering the Medibox using a servo motor connected to a shaded sliding window.
    - The motor angle is dynamically adjusted based on the light intensity using the following equation:
      ```
      θ = min{θoffset × D + (180 − θoffset) × I × γ, 180}
      ```
      where,
      - θ: motor angle
      - θoffset: minimum angle (default 30 degrees)
      - I: max intensity of light (0 to 1)
      - γ: controlling factor (default 0.75)
      - D: 0.5 if right LDR gives max intensity, 1.5 if left LDR gives max intensity

3. **User Controls on Node-RED Dashboard**
    - Adjust minimum angle and controlling factor using sliders.
    - Select commonly used medicines from a dropdown menu or use a custom option to manually set values for minimum angle and controlling factor.

## Getting Started

### Prerequisites

- Wokwi Simulator
- Node-RED

### Installation

1. **Clone the Repository**

   ```bash
   git clone https://github.com/yourusername/medibox-simulation.git
   cd medibox-simulation
   ```

2. **Setup Wokwi Simulator**

   - Open the Wokwi simulator and load the provided ESP32 project file.
   - Ensure the connections for the OLED, buzzer, push button, temperature, and humidity sensors are correctly set up.

3. **Setup Node-RED**

   - Install Node-RED on your system if not already installed.
   - Import the provided Node-RED flow JSON file into Node-RED.
   - Ensure the dashboard is configured with the necessary UI components (gauges, plots, sliders, dropdown menu).

## Usage

1. **Start the Simulation**

   - Run the Wokwi simulation to initialize the ESP32 and its peripherals.
   - Open the Node-RED dashboard to interact with the Medibox.

2. **Setting Time Zone and Alarms**

   - Use the menu on the OLED to set the time zone and alarms.
   - Alarms will ring at the set times with an indication on the OLED and buzzer.

3. **Monitoring Environmental Conditions**

   - Temperature and humidity levels are continuously monitored.
   - Warnings are displayed on the OLED if values exceed healthy limits.

4. **Light Intensity Control**

   - The Node-RED dashboard displays the highest light intensity and its source.
   - Adjust the shaded sliding window using the sliders to set the minimum angle and controlling factor.

![Design](https://github.com/jaliyanimanthako/Smart-medibox/assets/161110418/6ba12f58-fbbd-4b25-991a-dcaa55abeb87)
![Design](https://github.com/jaliyanimanthako/Smart-medibox/assets/161110418/89fc9780-eb77-40e9-b5fa-a9099fc217d3)
![DashBoard](https://github.com/jaliyanimanthako/Smart-medibox/assets/161110418/d5849a12-715d-4d25-9627-c5b2c9bb7af9)
![Flow](https://github.com/jaliyanimanthako/Smart-medibox/assets/161110418/8321e8b4-9bcb-4d78-82a3-a799482366d9)



