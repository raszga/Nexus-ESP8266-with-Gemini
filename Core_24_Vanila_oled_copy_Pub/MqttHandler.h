/* 
Developed with Gemini
Modified by Claude for secure credential management
           ---
           )|(
          <-o->
           )|(
           ---
*/

#ifndef MQTTHANDLER_H
#define MQTTHANDLER_H

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "SensorHandler.h"
#include "credentials.h"  // ← ONLY CHANGE: Include external credentials
// Platform detection
#if defined(ESP8266)
    #define PLATFORM_ESP8266
#elif defined(ESP32)
    #define PLATFORM_ESP32
#else
    #error "Unsupported platform"
#endif

// --- MQTT CONFIGURATION: Now using credentials.h ---
extern const char* MQTT_SERVER;
extern const int MQTT_PORT;
extern const char* MQTT_USER;
extern const char* MQTT_PASSWORD;
extern const char* MQTT_TOPIC;
extern const char* MQTT_CLIENT_ID;
extern const char* MY_WIFI_SSID;
extern const char* MY_WIFI_PASSWORD;
extern String message;

class MqttHandler {
public:
  MqttHandler();
  void begin(const char* ssid, const char* password);
  void handleMqttLoop();
  void publish(const String& payload);

private:
  WiFiClientSecure _espClient;
  PubSubClient _client;

  unsigned long _lastPublish = 0;
  const unsigned long PUBLISH_INTERVAL_MS = 10000;

  void _reconnect();
  static void _callback(char* topic, byte* payload, unsigned int length);
};

extern MqttHandler mqttHandler;

#endif  // MQTTHANDLER_H
