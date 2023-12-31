
![image](https://github.com/orcohen9826/Buzzer/assets/59200103/dffebd63-dbf8-47f4-922b-72c65b1f4354)

# Quiz Buzzer System

## Overview
This project is a wireless quiz buzzer system designed for quiz games where participants answer questions by pressing their respective buzzers. The system consists of multiple buzzers and a central controller that connects to an external speaker.

## Components

### Wireless Buzzers
- **Microcontroller:** Wemos D1 Mini
- **Power:** 9V battery with a voltage converter to 5V
- **Communication:** ESP-NOW
- **Enclosure:** Custom 3D printed cases

### Central Controller
- **Microcontroller:** NodeMCU ESP32
- **Audio:** DFPlayer Mini MP3 player, plays a unique sound corresponding to each buzzer when pressed
- **Communication:** Receives signals from the buzzers using ESP-NOW

## Software
The project is developed using Visual Studio Code with PlatformIO.

## Setup and Installation

### Prerequisites
- Visual Studio Code
- PlatformIO plugin for VSCode

### Steps
1. **Clone the Repository:**
    ```sh
    git clone https://github.com/orcohen9826/Buzzer.git
    ```
2. **Open in VSCode:** Navigate to the project folder and open it using VSCode.
3. **Install Dependencies:** Ensure all libraries and dependencies are installed through PlatformIO.
4. **Upload Code:** Upload the code to the Wemos D1 Mini devices for the buzzers and the NodeMCU ESP32 for the central controller.
5. **Copy Sound Files:** Copy the sound files to a memory card in a folder named "mp3". The first file will correspond to Buzzer 1, the second file to Buzzer 2, and so on.
6. **Initial System Boot:** On first boot, connect the central controller and read its serial monitor. When a buzzer is pressed for the first time, its MAC address will be printed to the console. Assign the MAC addresses to the respective buzzers in the central controller's code.
7. **Hardware Setup:** Connect the 9V batteries to the buzzers and set up the central controller with the DJ system.
8. **Test:** Ensure that the buzzers communicate with the central controller and that each buzzer press triggers a unique sound to be played through the DJ system.

## Usage
- **Buzzers:** Participants press the buzzer during the quiz. The signal is sent wirelessly to the central controller.
- **Central Controller:** Upon receiving a signal, the central controller plays a unique sound corresponding to the pressed buzzer through the DJ system.

## Contributing
Feel free to fork the project and submit your contributions through pull requests.
