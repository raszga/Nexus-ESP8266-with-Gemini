/* 
Developed with Gemini
Modified by Claude for secure credential management
           ---
           )|(
          <-o->
           )|(
           ---
*/
#include "MqttHandler.h"
//#include "OLED_Utilities.h"
#include <WiFiManager.h>

// --- MQTT CONFIGURATION: Using values from credentials.h ---
const char* MQTT_SERVER = MQTT_SERVER_ADDR;
const int MQTT_PORT = MQTT_PORT_NUM;
const char* MQTT_USER = MQTT_USERNAME;
const char* MQTT_PASSWORD = MQTT_USER_PASSWORD;
const char* MQTT_TOPIC = MQTT_TOPIC_NAME;
const char* MQTT_CLIENT_ID = ESPid.c_str();
const char* MY_WIFI_SSID = WIFI_NETWORK_SSID;
const char* MY_WIFI_PASSWORD = WIFI_NETWORK_PASSWORD;

String message = "";
MqttHandler mqttHandler;

// 2. CONSTRUCTOR
MqttHandler::MqttHandler()
  : _client(_espClient) {}

// 3. BEGIN (With WiFiManager for on-demand configuration)
void MqttHandler::begin(const char* ssid, const char* password) {
  _espClient.setInsecure();  // Required for HiveMQ 8883 (you mentioned this works for you)

  WiFiManager wm;
  wm.setConfigPortalTimeout(180);  // 3 minutes timeout

  String portalName = "Config_" + ESPid;
  //oled.clear();
  //oled.printLine("WIFI CONFIG", 3, 1, false);
  //oled.printLine(portalName.c_str(), 4, 1, true);
  //oled.printLine("Power OFF/ON after connect", 5, 1, false);

  // Attempt connection. If it fails, it opens the portal.
  if (!wm.autoConnect(portalName.c_str(), "01140257561358")) {
    Serial.println("Portal Timeout - Offline Mode");
  }
  _client.setServer(MQTT_SERVER, MQTT_PORT);
  _client.setCallback(_callback);
}

// 4. THE MQTT LOOP (Non-blocking)
void MqttHandler::handleMqttLoop() {
  if (WiFi.status() != WL_CONNECTED) {
    static unsigned long lastWiFiCheck = 0;
    if (millis() - lastWiFiCheck > 10000) {
      lastWiFiCheck = millis();
      WiFi.begin();  // Attempt background reconnect to saved creds
    }
    return;
  }
  if (!_client.connected()) {
    _reconnect();
  } else {
    _client.loop();
  }
}

// 5. PUBLISH
void MqttHandler::publish(const String& payload) {
  if (_client.connected()) {
    _client.publish(MQTT_TOPIC, payload.c_str());
  }
}

// 6. RECONNECT (Private)
void MqttHandler::_reconnect() {
  static unsigned long lastReconnect = 0;
  if (millis() - lastReconnect < 5000) return;
  lastReconnect = millis();

  Serial.print("Attempting MQTT...");
  if (_client.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASSWORD)) {
    Serial.println("Success");
    _client.subscribe(MQTT_TOPIC);
  } else {
    Serial.print("Failed, rc=");
    Serial.println(_client.state());
  }
}

// 7. CALLBACK (Static)
void MqttHandler::_callback(char* topic, byte* payload, unsigned int length) {
  String incoming = "";
  for (unsigned int i = 0; i < length; i++) {
    incoming += (char)payload[i];
  }
  // Self-echo filter
  if (incoming.indexOf(ESPid) == -1) {
    message = incoming;
  }
}
