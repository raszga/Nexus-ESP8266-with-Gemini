# Nexus-ESP8266-with-Gemini
A modular ESP8266 IoT framework featuring an adaptable I2C sensor engine and secure MQTT orchestration. Developed in collaboration with Gemini, Claude, and Copilot.
# ESP8266 Elastic MQTT Framework

/* Developed with Gemini, Claude & Copilot
           /|\
          --+--
           \|/
   Monday, February 23, 2026
*/

## 🚀 Overview
A modular, object-oriented IoT framework for the ESP8266 (NodeMCU/Wemos D1 Mini). This system features an "Adaptable Sensor" architecture that dynamically manages I2C, Analog, and Digital sensors, combined with a robust MQTT command parser and a Python-based desktop controller.

## 🤖 Multi-AI Collaboration
This project represents a sophisticated Human-AI partnership:
* **Phase 1 (Copilot):** Initial prototyping and early logic exploration.
* **Phase 2 (Gemini):** Engineered the core "Adaptable" architecture, Actuator Managers, and the modular C++ structure.
* **Phase 3 (Claude Sonnet 4.5):** Implemented secure credential management (`config.ini` / `credentials.h` logic) and the curses-based Python `MQTT_sender_.py`.

## 🛠 Features
* **Elastic Sensor Management:** Automatically detects hardware on the I2C bus and "shifts" active sensors to the front of the polling loop, gracefully handling missing hardware.
* **Unified Actuator Control:** Supports PWM (Fans/LEDs) and Digital outputs with soft-start (Kickstart) and safety-reverse logic.
* **OLED Visuals:** Dynamic UI with real-time progress bars, QR code generation for device IDs, and system status inverted-flashes.
* **Command Parsing:** Advanced string parsing (via `aux_`) allows complex command chains like `*F500*D100` to be executed in one MQTT message.
* **Desktop Remote:** A Python terminal UI for monitoring the mesh and sending global or targeted commands.

## 📁 Repository Structure
* `Core_24_Vanila_oled.ino`: Main firmware entry point.
* `SensorHandler.h/cpp`: The Elastic polling engine.
* `MqttHandler.h/cpp`: Secure WiFi & MQTT connection management.
* `MQTT_sender_.py`: Desktop controller (Python).
* `credentials.h.example`: Template for your WiFi/MQTT secrets.

## ⚙️ Installation
1.  **Hardware:** Connect ESP8266 SDA (GPI14) and SCL (GPI12). fot this particular controller with oled integrated 
2.  **Credentials:** * Create `credentials.h` based on the template for the ESP8266.
    * Create `config.ini` for the Python script.
3.  **Libraries:** Required: `Adafruit_BMP280`, `Adafruit_MPU6050`, `PubSubClient`, `WiFiManager`, `qrcode`.
4.  **Flash:** Upload to your device and run the Python script to connect.

## 📜 License
This project is open-source. Please maintain the AI-collaboration headers in the source files.
