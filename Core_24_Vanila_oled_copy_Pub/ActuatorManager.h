/* Developed with Gemini
           /|\
          --+--
           \|/
Thursday, February 19, 2026 11:15 PM
*/
#ifndef ACTUATORMANAGER_H
#define ACTUATORMANAGER_H
#include <Arduino.h>
#include <cstdio>
#include "SensorHandler.h"
// Platform detection
#if defined(ESP8266)
#define PLATFORM_ESP8266
#elif defined(ESP32)
#define PLATFORM_ESP32
#else
#error "Unsupported platform"
#endif
/***************************************************************************/
struct Actuator {
  String Name = "-AA";
  String Name0 = "-AA";
  bool isBooted = false;
  int pin = -1;

  bool pwm = false;
  int lastDuty = 0;  // State tracking
  int duty = 0;
  int dutyMax = 1023;

  int freqSet0 = 20000;
  int freqSetLow = 50;
  int freq = freqSet0;

  bool active = false;
  bool action = true;  // High = ON

  // --- SMART EXTENSIONS ---
  Actuator* subActuator = nullptr;   // e.g., L298 Enable or Relay
  Sensor* internalSensor = nullptr;  // e.g., Temp Sensor

  void Init();
  void Activate();
  void deActivate();
  void Reset();
  void PowON(int dt, bool dbg = false);
  void PowOFF();
  void parseCommand(const char* cmd);
  void apply();
  void MotoStart();
  void Monitor();  // Automatic safety check
  int reverse();
  void Beep(int dt = 500, int dy =512);
};
/*============Define pointer array to Actuators=================================*/
void initActuators();
//===============================================================================
extern Actuator* AllAct[];
//===============================================================================
#endif  //ACTUATORMANAGER_H=================EOF===================================