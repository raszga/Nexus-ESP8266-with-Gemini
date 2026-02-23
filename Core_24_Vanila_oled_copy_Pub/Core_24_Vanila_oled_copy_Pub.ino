/* Developed with Gemini
           /|\
          --+--
           \|/
Thursday, February 19, 2026 11:15 PM*/
// ============================================================================
#include "MqttHandler.h"
#include "SensorHandler.h"
#include "ActuatorManager.h"
#include "OLED_Utilities.h"
#include <iostream>
#include "aux_.h"
#include "I2C_Sensors.h"
// ============================================================================
/*****************************************************************************/
bool _check, _check0 = true;
String persistent = ESPid;
const char* _ESPid = persistent.c_str();
// Store ID as a String to ensure the memory remains valid for the life of the program
String persistent1 = String(ESP.getChipId());
const char* serESPid = persistent1.c_str();
// Define Sensors
// ============================================================================

//ADC_MODE(ADC_VCC);
Sensor WaterBas = {
  .Name = "WaterBas:",
  .Pin = A0,
  .analogue = true,
  .Vmax = 400,
  .Vbatt = -A0,
};

Sensor LM7575_0x48 = {
  .Name = "0x48_Temp: ",
  .Vmin = 100,
  .Vmax = 5000,
  .i2c = true,
  .i2cAddress = 0x48,
  .pointerRead = LM75A_ReadTemp_Elastic,  // Declarations now match void*
  .pointerInit = LM75A_Init_Elastic
};


Sensor Debug{
  .Name = "DebugPin",
  .Pin = 5,
  .Norm = 1,  // debug when pressed
  .Publish = false,
};

Sensor isInternet = {
  .Name = "Pulsar-",
  .Vmax = 700,
  .soft = true,
  .pointerRead = CheckPulsarRead  // Elastic software check,
};

//*****************************************************************************
Sensor* AllSensors[] = {
  &Debug,
  &WaterBas,
  &isInternet,
  nullptr,
};
//Actuators====================================================================
Actuator LED = {
  .Name = "LED-",
  .pin = 4,
  .pwm = true,
};
Actuator Buzz = {
  .Name = "BuzBas-",
  .pin = 13,
  .pwm = true,
  .freq = 1000,
  .action = true,
};
Actuator* AllAct[] = {
  &LED,
  &Buzz,
  nullptr,
};
// ============================================================================
using namespace aux;
// ============================================================================
// --- SETUP: Initialize System Components ---
// ============================================================================
void setup() {
  Wire.begin(SDA, SCL);
  Serial.begin(9600);
  if (!oled.begin()) {
    Serial.println(F("OLED failed!"));
    while (true)
      ;
  }
  LED.Beep();
  oled.printLine(ESPid.c_str(), 0, 1);
  oled.genQR(ESPid.c_str(), 2000);
  oled.clear();
  oled.printLine(ESPid.c_str(), 0, 1);

  Serial.println("");
  Serial.println("Init Sensors");
  initSensors();
  // kik out debug
  /****manualy exclude debug sensor*****/
  delay(250);
  LED.Beep();
  Serial.println("Init Actuators");
  initActuators();
  delay(250);
  LED.Beep();
  Buzz.Beep();
  Buzz.PowOFF();
  delay(250);
  mqttHandler.begin(MY_WIFI_SSID, MY_WIFI_PASSWORD);
  Serial.println("**********************************************");
  Serial.println(getBuildInfo().c_str());
  Serial.println("**********************************************");
  delay(1000);
}
/******************************************************************************/
// --- MAIN LOOP: Process Input, Commands, and Alarms ---
/******************************************************************************/
void loop() {
  debug = !Debug.Check();
  mqttHandler.handleMqttLoop();
  inv = !inv;
  oled.printLine(("Up[s]:" + String((millis() - pulsarTime) / 1000)).c_str(), 1, 1, inv);
  oled.drawProgressBar(8, 64, 60, 3, int(100 * (millis() - pulsarTime) / (isInternet.Vmax * 100)));
  // Sync with MQTT Broker
  /***************************************************************************/
  delay(500);
  _check = true;
  if (debug) {
    delay(100);
    mqttHandler.publish(serESPid + '/n');
    delay(100);
    mqttHandler.publish(getBuildInfo().c_str());
    Buzz.Beep();
    LED.Beep();
  }
  for (int i = 0; AllSensors[i] != nullptr; i++) {
    if (debug) Serial.println(ESPid + "*******************");
    if (debug) {
      Serial.print(AllSensors[i]->Response());
      Serial.print(" / Vmax= ");
      Serial.println(AllSensors[i]->Vmax);
      if ((AllSensors[i]->Publish) == true) mqttHandler.publish(AllSensors[i]->Response().c_str());
    }
    _check = _check * AllSensors[i]->Check();
    if ((!AllSensors[i]->Check()) && (AllSensors[i]->active)) {
      mqttHandler.publish(AllSensors[i]->Response().c_str());
      Buzz.Beep();
    }
    if ((_check) && (!_check)) {
      mqttHandler.publish(AllSensors[i]->Response().c_str());
      Buzz.PowOFF();
      delay(100);
      mqttHandler.publish((ESPid + ": All alarmOFF").c_str());
    }
    _check0 = _check;
  }

  /*************************************************************************/
  if (message != "") {
    string cleaned = "";
    LED.Beep(50, 1023);
    if (debug) Serial.println(ESPid + ": MQTT Received:" + message);

    if (message.indexOf(">>PULSE-ER") != -1) {
      if (debug) {
        Serial.println(millis() - pulsarTime);
        Serial.println(ESPid + ": Pulsar Reset");
      }
      pulsarTime = millis();
    }

    if ((message.indexOf(_ESPid) != -1) || (message.indexOf("All") != -1) || (message.indexOf(serESPid) != -1)) {
      if (message.indexOf(_ESPid) != -1) cleaned = removeSubstring(message.c_str(), _ESPid);
      if (message.indexOf(serESPid) != -1) cleaned = removeSubstring(message.c_str(), serESPid);
      if (message.indexOf("All") != -1) cleaned = removeSubstring(message.c_str(), "All");
      message = cleaned.c_str();
      if (debug) Serial.println(ESPid + " MSG passed  filter: " + message);

      if (message.indexOf("Check") != -1) {
        silentMode = false;
        if (debug) Serial.println(ESPid + "==================");
        for (int i = 0; AllSensors[i] != nullptr; i++) {
          mqttHandler.publish(AllSensors[i]->Response().c_str());
          if (debug) Serial.println(AllSensors[i]->Response());
          LED.Beep(250, 1023);
        }
        if (debug) Serial.println("========================================");
      }
      if (message.indexOf("Reset") != -1) {
        ESP.reset();
      }
      if (message.indexOf("alarmON") != -1) {
        LED.Beep(250, 1023);
        Buzz.Beep();
      }
    }
    message = "";
  }
}
