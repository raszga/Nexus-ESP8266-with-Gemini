/* Developed with Gemini
           /|\
          --+--
           \|/
*/
#include "I2C_Sensors.h"

// Static instances for library-based sensors
static Adafruit_BMP280 _bmp;
static Adafruit_HMC5883_Unified _mag = Adafruit_HMC5883_Unified(12345);
static Adafruit_MPU6050 _mpu;

/*==============================================================================
 * UTILITIES
 *============================================================================*/


/*==============================================================================
 * ADAFRUIT ELASTIC WRAPPERS
 *============================================================================*/

bool Adafruit_BMP280_Init(void* arg) {
    uint8_t addr = *(uint8_t*)arg;
    return _bmp.begin(addr);
}

int Adafruit_BMP280_ReadTemp(void* arg) { 
    return (int)_bmp.readTemperature(); 
}

int Adafruit_BMP280_ReadPress(void* arg) { 
    return (int)(_bmp.readPressure() / 100); 
}

bool Adafruit_HMC5883_Init(void* arg) { 
    return _mag.begin(); 
}

int Adafruit_HMC5883_ReadHeading(void* arg) {
    sensors_event_t event;
    _mag.getEvent(&event);
    float heading = atan2(event.magnetic.y, event.magnetic.x);
    if(heading < 0) heading += 2 * PI;
    return (int)(heading * 180 / M_PI);
}

bool Adafruit_MPU6050_Init(void* arg) {
    uint8_t addr = *(uint8_t*)arg;
    return _mpu.begin(addr);
}

int Adafruit_MPU6050_ReadTemp(void* arg) {
    sensors_event_t a, g, temp;
    _mpu.getEvent(&a, &g, &temp);
    return (int)temp.temperature;
}

/*==============================================================================
 * LM75A LOGIC (ELASTIC)
 *============================================================================*/

bool LM75A_Init_Elastic(void* arg) {
    return I2C_deviceExists(*(uint8_t*)arg);
}

int LM75A_ReadTemp_Elastic(void* arg) {
    uint8_t addr = *(uint8_t*)arg;
    uint8_t data[2];
    Wire.beginTransmission(addr);
    Wire.write(0x00); // Access Temperature Register
    if (Wire.endTransmission() != 0) return -1;
    
    Wire.requestFrom(addr, (uint8_t)2);
    if (Wire.available() < 2) return -1;
    
    // Combine high and low bytes, then shift for 11-bit resolution
    int16_t raw = ((int16_t)Wire.read() << 8) | Wire.read();
    raw >>= 5;
    if (raw & 0x0400) raw |= 0xF800; // Sign extend for negative values
    
    return int((raw * 125) / 10); // Return result scaled for integer use
}

/*==============================================================================
 * BME280 LOGIC (ELASTIC)
 *============================================================================*/

bool BME280_Init_Elastic(void* arg) {
    uint8_t addr = *(uint8_t*)arg;
    if (!I2C_deviceExists(addr)) return false;
    Wire.beginTransmission(addr);
    Wire.write(0xF4); Wire.write(0x27); // Set to normal mode
    return (Wire.endTransmission() == 0);
}

int BME280_ReadTemp_Elastic(void* arg) {
    uint8_t addr = *(uint8_t*)arg;
    uint8_t data[3];
    Wire.beginTransmission(addr);
    Wire.write(0xFA); // Temperature MSB register
    Wire.endTransmission();
    
    Wire.requestFrom(addr, (uint8_t)3);
    for(int i=0; i<3; i++) data[i] = Wire.read();
    
    // Combine 20-bit raw temperature
    int32_t raw = ((int32_t)data[0] << 12) | ((int32_t)data[1] << 4) | (data[2] >> 4);
    return int(((raw / 1638)) + 2000) / 100; // Return integer representation
}