/* Developed with Gemini
           /|\
          --+--
           \|/
*/
#ifndef I2C_SENSORS_H
#define I2C_SENSORS_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_HMC5883_U.h>
#include <Adafruit_MPU6050.h>
#include "SensorHandler.h"


/*==============================================================================
 * ELASTIC ADAFRUIT WRAPPERS (void* arg)
 *============================================================================*/
bool Adafruit_BMP280_Init(void* arg);
int Adafruit_BMP280_ReadTemp(void* arg);
int Adafruit_BMP280_ReadPress(void* arg);

bool Adafruit_HMC5883_Init(void* arg);
int Adafruit_HMC5883_ReadHeading(void* arg);

bool Adafruit_MPU6050_Init(void* arg);
int Adafruit_MPU6050_ReadTemp(void* arg);

/*==============================================================================
 * ELASTIC IN-HOUSE LOGIC (void* arg)
 *============================================================================*/
bool LM75A_Init_Elastic(void* arg);
int LM75A_ReadTemp_Elastic(void* arg);

bool BME280_Init_Elastic(void* arg);
int BME280_ReadTemp_Elastic(void* arg);

#endif