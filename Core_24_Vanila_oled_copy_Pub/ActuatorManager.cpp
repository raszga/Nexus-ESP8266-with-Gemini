/* Developed with Gemini
           /|\
          --+--
           \|/
Thursday, February 19, 2026 11:15 PM
*/
#include "ActuatorManager.h"
/*============================================================================*/
void Actuator::Init() {
  if (!isBooted) {
    Name0 = Name;
    Name += ESPid;
    isBooted = true;
  }
  active = true;
  if (pin != -1) {
    pinMode(pin, OUTPUT); /*enable pin as output*/
    if (pwm) {
      analogWriteFreq(freq);
      analogWriteRange(dutyMax);
    }
    PowON(0);
    active = false;
  }
}

/*============================================================================*/
void Actuator::Activate() {
  active = true;
}
/*============================================================================*/
void Actuator::deActivate() {
  active = false;
}
/*============================================================================*/
void Actuator::PowON(int dt, bool dbg) {
  if (pin == -1) return;
  
  if (dbg) {
    Serial.print(Name + ":PowON:");
    Serial.println(dt);
  }

  if (pwm) {
    analogWrite(pin, abs(dt));
  } else {
    // Determine pin state: dt > 0 is ON (action), dt <= 0 is OFF (!action)
    bool state;
    if (dt > 0) {
      state = action;   
    } else {
      state = !action;  
    }
    
    digitalWrite(pin, state);

    if (dbg) {
      Serial.print(Name + ":recorded duty:" + String(dt));
      Serial.println(" -> Pin State: " + String(state));
    }
  }

  this->lastDuty = dt; 
  if (subActuator != nullptr) {
    subActuator->PowON(dt);
  }
}

/*============================================================================*/
void Actuator ::PowOFF() {
  PowON(0);
}
/*============================================================================*/
void Actuator ::parseCommand(const char* cmd) {
  if (debug) Serial.println(cmd);
  // 1. Look for Duty (*F)
  const char* fMarker = std::strstr(cmd, "*F");
  if (fMarker != nullptr) {
    int tempDuty = 0;
    if (std::sscanf(fMarker + 2, "%d", &tempDuty) == 1) {
      this->duty = tempDuty;  
    }
  }
  const char* iMarker = std::strstr(cmd, "*I");
  if (iMarker != nullptr) {
    int tempDuty = 0;
    if (std::sscanf(iMarker + 2, "%d", &tempDuty) == 1) {
      this->duty = tempDuty;  
    }
  }

  // 2. Look for Frequency (*Q)
  const char* qMarker = std::strstr(cmd, "*Q");
  if (qMarker != nullptr) {
    long tempFreq = 0;
    if (std::sscanf(qMarker + 2, "%ld", &tempFreq) == 1) {
      if (tempFreq <= 50000) {
        this->freq = (int)tempFreq;
      } else {
        this->freq = freqSet0;
      }
      if (tempFreq <= 0) {
        this->freq = freqSet0;
      }
    }
  }
}

/*============================================================================*/
void Actuator::Reset() {
  duty = 0;
  freq = freqSet0;
  Name = Name0;
  isBooted = false;
  PowOFF();
  deActivate();
}
/*============================================================================*/
void Actuator::MotoStart() {
  if (this->duty == 0) {
    this->lastDuty = 0;
    this->PowOFF();  
    return;
  }
  if (this->lastDuty == 0) {
    Serial.println("Kickstarting...");
    this->PowON(sgn(this->duty) * (this->dutyMax));  
    delay(1000);
  }
  Serial.println("Cruise Mode Active...");
  this->PowON(this->duty);
  this->lastDuty = this->duty;
}
/*============================================================================*/
int Actuator::reverse() {
  int tempDuty = lastDuty;

  if (((lastDuty * duty) < 0)) {
    Serial.println(String(lastDuty) + "to" + String(duty));
    Serial.println("Bit-Shifting to Zero...");
    while (abs(tempDuty) > 0) {
      tempDuty = tempDuty / 2;  
      PowON(tempDuty);
      delay(50);
    }
  }
  if ((sgn(lastDuty) == sgn(duty) && (abs(lastDuty) > abs(duty)))) {
    while (abs(tempDuty) >= abs(duty)) {
      tempDuty = tempDuty / 2;  
      PowON(tempDuty);
      delay(50);
    }
  }
  PowON(tempDuty);
  lastDuty = tempDuty;  
  return lastDuty;
}
void Actuator::Beep(int dt, int dy) {
  PowON(0);
  PowON(dy);
  delay(dt);
  PowON(0);
}
/******************************************************************************/
void initActuators() {
  for (int i = 0; AllAct[i] != nullptr; i++) {
    AllAct[i]->Init();
    AllAct[i]->Activate();
    AllAct[i]->PowON(0);
  }
}
/*===========================EOF==============================================*/