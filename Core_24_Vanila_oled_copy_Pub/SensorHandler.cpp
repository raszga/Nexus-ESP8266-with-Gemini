/* Developed with Gemini
           /|\
          --+--
           \|/
2026-02-21 16:55:00
*/
#include "SensorHandler.h"
// Initialize shared system constants
const String ESPid = "ESP" + String(ESP.getChipId());
unsigned long int pulsarTime = millis();
bool silentMode = false;
bool debug = false;
/*****************************************************************************/
void Sensor::Init() {
  if (!isBooted) {
    Name0 = Name;
    Name += " " + ESPid;
    isBooted = true;
  }
  V = -1;
  // Elastic Initialization: Pass I2C Address or Vmax as a generic pointer
  if (pointerInit != nullptr) {
    pointerInit(i2c ? (void*)&i2cAddress : (void*)&Vmax);
  }
  // Handle standard digital/analog GPIO setup
  if (!i2c && !soft) {
    if (Pow != -1) {
      pinMode(Pow, OUTPUT);
      digitalWrite(Pow, normPow);
    }
    if (ESP.getVcc() != 65535) {
      Vmax = int((Vmax * 3600) / 1023);  //rescale
    }
    if (!analogue) pinMode(Pin, Set);
  }
  active = false;
}
/*****************************************************************************/
void Sensor::PowON() {
  if (active && Pow != -1) digitalWrite(Pow, !normPow);
}
/*****************************************************************************/
void Sensor::PowOFF() {
  if (active && Pow != -1) digitalWrite(Pow, normPow);
}
/*****************************************************************************/
void Sensor::Activate() {
  active = true;
}
/*****************************************************************************/
void Sensor::deActivate() {
  PowOFF();
  active = false;
}
/*****************************************************************************/
void Sensor::Read() {
  if (!active) {

    V = -1;
    return;
  }
  // Execute via Elastic Pointer if defined
  if (pointerRead != nullptr) {
    V = pointerRead(i2c ? (void*)&i2cAddress : (void*)&Vmax);
    if (soft) softStage = (V == 1);  // Convention: 1 = Logic OK
    return;
  }
  // Fallback to standard GPIO Read
  PowON();
  delay(100);
  if (analogue) {
    // Check if high-sensitivity VCC mode is active in firmware
    uint16_t vccVal = ESP.getVcc();
    if (vccVal != 65535) {
      // MODE: ADC_MODE(ADC_VCC) is enabled.
      // V will hold millivolts (e.g. 3300 for 3.3V)
      ReadBatt();
    } else if ((Vbatt == -2) || (analogue)) {
      // MODE: Standard. Only read A0 if battery logic is not overriding
      // V will hold raw ticks (0-1023)
      V = analogRead(A0);
    }
  } else {
    // Digital Read fallback
    V = digitalRead(Pin);
  }
  PowOFF();
}
/*******************************************************************************/
void Sensor::ReadBatt() {
  if (Vbatt != -2) V = ESP.getVcc();
}
/*******************************************************************************/
bool Sensor::Check() {
  Read();
  if (i2c || analogue) return ((Vmin <= V) && (V <= Vmax));
  if (soft) return softStage;
  return (V == Norm);
}
/******************************************************************************/
String Sensor::Response() {
  if (!Publish) return sMSG = Name+" alarmNA";
  Read();
  bool isOk = Check();
  if (soft) V = int((millis() - pulsarTime) / 1000);
  buildMessage(sMSG, Name.c_str(), V, isOk);
  return sMSG;
}
/****************************************************************************/
void Sensor::Reset() {
  Name = Name0;
  PowOFF();
  active = false;
  V = -1;
}
/*****************************************************************************/
// --- Logic Helper Implementations ---
/*****************************************************************************/
bool checkProto(void* arg) {
  return true;
}
/*****************************************************************************/
bool CheckPulsar(void* arg) {
  int tMax = *(int*)arg;  // Cast back to integer threshold
  return (int((millis() - pulsarTime) / 1000) <= tMax);
}
/*****************************************************************************/
int CheckPulsarRead(void* arg) {
  return CheckPulsar(arg) ? 1 : 0;
}
/*****************************************************************************/
// --- Global Sensor Array Management ---
/*****************************************************************************/
void initSensors() {
  for (int i = 0; AllSensors[i] != nullptr; i++) {
    delay(100);
    AllSensors[i]->Init();
    AllSensors[i]->Activate();
  }
  ChkI2CSensHardw();
}
/*****************************************************************************/
void checkSensors() {
  for (int i = 0; AllSensors[i] != nullptr; i++) AllSensors[i]->Check();
}
/*****************************************************************************/
void respondSensors() {
  for (int i = 0; AllSensors[i] != nullptr; i++) AllSensors[i]->Response();
}
/*****************************************************************************/
void I2C_scanBus() {
  byte error, address;
  int nDevices = 0;
  Serial.println("Scanning I2C Bus...");
  for (address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address < 16) Serial.print("0");
      Serial.println(address, HEX);
      nDevices++;
    }
  }
  if (nDevices == 0) Serial.println("No I2C devices found\n");
  else Serial.println("Scan complete.\n");
}
/*****************************************************************************/
bool I2C_deviceExists(uint8_t address) {
  Wire.begin(SDA, SCL);
  Wire.beginTransmission(address);
  return (Wire.endTransmission() == 0);
}
/*****************************************************************************/
void ChkI2CSensHardw() {
  int activeCount = 0;
  int totalSensors = 0;

  // 1. Identify hardware presence and update active state
  for (int i = 0; AllSensors[i] != nullptr; i++) {
    totalSensors++;
    delay(100);
    if ((AllSensors[i]->i2c) && (!I2C_deviceExists(AllSensors[i]->i2cAddress))) {
      Serial.print("Sensor missing: 0x");
      Serial.print(AllSensors[i]->i2cAddress, HEX);
      Serial.print(" ");
      Serial.println(AllSensors[i]->Name);
      AllSensors[i]->active = false;
    } else {
      AllSensors[i]->active = true;
    }
  }

  // 2. Re-arrange: Shift pointers of active sensors to the start of the array
  for (int i = 0; i < totalSensors; i++) {
    if (AllSensors[i]->active) {
      // Swap to the front
      Sensor* temp = AllSensors[activeCount];
      AllSensors[activeCount] = AllSensors[i];
      AllSensors[i] = temp;
      activeCount++;
    }
  }

  // 3. Shrink: Fill the tail with nullptr to terminate future loops
  for (int i = activeCount; i < totalSensors; i++) {
    AllSensors[i] = nullptr;
  }

  Serial.print("Active sensors reorganized: ");
  Serial.println(activeCount);
}
/*****************************************************************************/