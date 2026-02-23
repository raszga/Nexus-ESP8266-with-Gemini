import paho.mqtt.client as paho
import ssl
import os
import random
import sys
BROKER = "4ae0ff064ac54e2d9a978133ce70718f.s1.eu.hivemq.cloud"
PORT = 8883
TOPIC = "Test"
USERNAME = "CR22081"
PASSWORD = "Timisoara1900"

# -------------------------------------------------------------------
# Callbacks updated for API v2 signatures
def on_connect(client, userdata, flags, reasonCode, properties):
    print(f"[INFO] Connected with reason code {reasonCode}")
    client.subscribe(TOPIC, qos=0)
# -------------------------------------------------------------------
def on_message(client, userdata, message):
    payload = message.payload.decode("utf-8")
    if "cls" in str(payload):
        os.system("cls")
    if "quit" in str(payload):
        os.system("taskkill /F /PID %d" % os.getppid())
        sys.exit(0)

    print(f"[RX] {message.topic}: {payload}")

# -------------------------------------------------------------------
def on_subscribe(client, userdata, mid, reasonCode, properties):
    print(f"[INFO] Subscribed (mid={mid}, reason={reasonCode})")

# -------------------------------------------------------------------
# Create client with Callback API v2

os.system("cls")

client = paho.Client(
    client_id="ReceiverClient"+str(random.randint(0,100)),
    protocol=paho.MQTTv5,
    transport="tcp",
    callback_api_version=paho.CallbackAPIVersion.VERSION2
)

client.on_connect = on_connect
client.on_message = on_message
client.on_subscribe = on_subscribe

# TLS setup with modern API
client.tls_set(tls_version=ssl.PROTOCOL_TLS_CLIENT)
client.username_pw_set(USERNAME, PASSWORD)

# Connect with keepalive
client.connect(BROKER, PORT, keepalive=60)

# Blocking loop for receiver
client.loop_forever()
