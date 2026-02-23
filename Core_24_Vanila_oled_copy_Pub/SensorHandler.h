/*
Board Label	GPIO Number	Function	Boot State	Recommendation
D0	GPIO 16	Wake / User	High	    OK (No PWM/Interrupts)
D1	GPIO 5	SCL (I2C)	Float	    BEST (General Purpose)
D2	GPIO 4	SDA (I2C)	Float	    BEST (General Purpose)
D3	GPIO 0	Flash Mode	Pull-UP	    CAUTION (Must be HIGH on boot)
D4	GPIO 2	TXD1 / LED	Pull-UP	    CAUTION (Must be HIGH on boot)
D5	GPIO 14	SCK (SPI)	Float	    GOOD (General Purpose)
D6	GPIO 12	MISO (SPI)	Float	    GOOD (General Purpose)
D7	GPIO 13	MOSI (SPI)	Float	    GOOD (General Purpose)
D8	GPIO 15	CS (SPI)	Pull-DOWN	CAUTION (Must be LOW on boot)
RX	GPIO 3	Serial RX	High	    Avoid (Used for USB Serial)
TX	GPIO 1	Serial TX	High	    Avoid (Used for USB Serial)
A0	ADC0	Analog Input	        N/A	OK (Max 3.3V on NodeMCU)
*/


/* Developed with Gemini
           /|\
          --+--
           \|/
2026-02-21 16:55:00
*/
#ifndef SENSORHANDLER_H
#define SENSORHANDLER_H

#include <Arduino.h>
#include "aux_.h"
#include <Wire.h>
// Define standard I2C pins for ESP8266 (14/12 sometimes)
#define SDA 14
#define SCL 12
// --- Global Constants and Shared Variables ---
extern const String ESPid;
extern unsigned long int pulsarTime;
extern bool silentMode;
extern bool debug;
// --- Elastic Function Pointer Types ---
// These allow passing either I2C addresses (uint8_t) or Soft thresholds (int) via void*
typedef int (*SensorReadFunction)(void* arg);
typedef bool (*SensorInitFunction)(void* arg);
// --- Core Sensor Structure ---
struct Sensor {
  String Name = "-1XXX";
  bool isBooted = false;
  int Pin = -1;
  int Set = INPUT_PULLUP;
  int Norm = 1;
  bool analogue = false;
  int Vmin = 0;
  int Vmax = 1023;
  int V = -1;
  int Vbatt = -2;
  int Pow = -1;
  int normPow = 0;
  int active = false;
  bool soft = false;

  bool softStage = true;
  String sMSG = "-1XXX";
  String Name0 = Name;

  bool i2c = false;
  uint8_t i2cAddress = 0x00;
  
  bool Publish = true;  //reserve for non pub sensors

  // Unified Elastic Pointers
  SensorReadFunction pointerRead = nullptr;
  SensorInitFunction pointerInit = nullptr;

  // Lifecycle and Operation Methods
  void Init();
  void Reset();
  bool Check();
  void Read();
  void ReadBatt();
  void PowON();
  void PowOFF();
  void Activate();
  void deActivate();
  String Response();
};  // end of structure-------------------

// --- System Management Functions I2C ---
bool I2C_deviceExists(uint8_t address);
void I2C_scanBus();
/*****General sensor group funtions*/
void initSensors();
void checkSensors();
void respondSensors();
void ChkI2CSensHardw();

extern Sensor* AllSensors[];  //all sensors group

// --- Logic Prototypes (Bridging Logic to Elastic System) ---
bool checkProto(void* arg);
bool CheckPulsar(void* arg);
int CheckPulsarRead(void* arg);
#endif