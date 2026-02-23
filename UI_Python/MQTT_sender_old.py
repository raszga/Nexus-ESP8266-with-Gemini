# -*- coding: utf-8 -*-
import paho.mqtt.client as paho
import ssl
from paho import mqtt
import wmi
import os
import sys
import qrcode
import random
# =============================================================================
# Standard Command list for the Telex Sender:
# =============================================================================
cmd = """====================================================
Standard Command list for the Telex Sender:
====================================================
o Check       - Check Response  all sensors
o Reset       - Reset all sensors
o Silent      - sensors ignore alarm/needs Reset
o R_ON        - relay engage
o R_OFF       - relay disengage
o Diff_OFF    - stop Diff
o Diff_FULL   - full power diff (512)
o *FXXXX      - Main fan speed 0-1023 PWM
o *IXXXX      - Internal fan speed 0-1023 PWM
o >>PULSE-ER  - Reset Pulse Timer
o QRcode      - Generate a specific QR 
o cls         - clear screens
0 cnt         - List Controllers
o quit        - exit telex
o ?           - list
"""
cnt="""o 2345360
o 14706265
o 1056332
o 737757
o 1000075
o 991347
0 3068210     - Fan Controller
o 15466192    - ESP32 Test Rig
o 8741776     - ESP32 Test Rig
o 871864      - Fan Controller
o 00000000dc7a9d7f:30.4        - RPI
o BFEBFBFF000A06529982    - LPTP"""

BROKER = "4ae0ff064ac54e2d9a978133ce70718f.s1.eu.hivemq.cloud"
PORT = 8883
TOPIC = "Test"
USERNAME = "CR22081"
PASSWORD = "Timisoara1900"

# =============================================================================
# MQTT Functions
# =============================================================================
def on_connect(client, userdata, flags, reasonCode, properties):
    print(f"[INFO] Connected with reason code {reasonCode}")
    client.subscribe("Test", qos=0)


def on_message(client, userdata, message):
    payload = message.payload.decode("utf-8")
    print(f"[RX] {message.topic}: {payload}")

# =============================================================================
# Generate QR Code
# =============================================================================
def generate_qr_code(data: str, filename: str = "qrcode.png",
                     fill_color: str = "black", back_color: str = "white") -> None:
    """
    Generate a QR code from text and save it as an image.
    Parameters:
        data (str): The text or URL to encode in the QR code.
        filename (str): The output file name (default: 'qrcode.png').
        fill_color (str): Foreground color of the QR code (default: 'black').
        back_color (str): Background color of the QR code (default: 'white').
        Example usage:
        generate_qr_code("Hello Calin, this is reusable!", "E:/Photos/hello_qr.png")
    """
    qr = qrcode.QRCode(
        version=4,
        error_correction=qrcode.constants.ERROR_CORRECT_L,
        box_size=16,
        border=2,
    )
    qr.add_data(data)
    qr.make(fit=True)

    img = qr.make_image(fill_color=fill_color, back_color=back_color)
    img.save(filename)
    print(f"QR code saved as {filename}")

# =============================================================================
# CPU id
# =============================================================================
def CPUid():
    c = wmi.WMI()
    for cpu in c.Win32_Processor():
        return cpu.ProcessorId.strip()
    return "UnknownCPU"

# =============================================================================
# Start Settings
# =============================================================================
CompName = "ACER:" + CPUid() #+str(random.randint(0,10000))

client = paho.Client(
    client_id=CompName,
    protocol=paho.MQTTv5,
    userdata=None,
    transport="tcp",
    callback_api_version=paho.CallbackAPIVersion.VERSION2,
)

client.tls_set(tls_version=mqtt.client.ssl.PROTOCOL_TLS)
client.username_pw_set(USERNAME, PASSWORD)
client.connect(BROKER, PORT, keepalive=60)
client.loop_start()

# =============================================================================
# prepare command list
# =============================================================================
_cmd=cmd.split(chr(10))[3:-1]
cmd=""
for i in range(len(_cmd)):
    line_without_o = " " + _cmd[i][1:]
    _cmd[i] = f"{i:<3d}{line_without_o}"
    cmd += _cmd[i] + chr(10)
# =============================================================================
# QR code generator
# =============================================================================

os.system("cls")
print(cmd,end="")
#print()
print("*****************************************************************")
print("MQTT Telex Sender (Type messages and press Enter.)")
print("*****************************************************************")
print("Type the command(s) or number(s) separated by [space]")
print("Type Serial[ space]command for specific controller)")
print("*****************************************************************")

msg="F_OFF"
payload = f"{CompName}:\n{msg}"
client.publish(TOPIC, payload)

try:
    while True:
        msg = input("TELEX> ").strip()
        if msg:
            try:
                M=_cmd[int(msg)]
                msg=M
                print(msg)
                payload = f"{CompName}:\n{msg}"
                client.publish(TOPIC, payload)
            except:
                pass
            msg = msg.replace(" ", " ")
            payload = f"{CompName}:\n{msg}"
            client.publish(TOPIC, payload)

        if  "quit" in msg:
            msg="F_OFF"
            payload = f"{CompName}:\n{msg}"
            client.publish(TOPIC, payload)
            os.system("taskkill /F /PID %d" % os.getppid())
            #sys.exit(0)
            exit()
            break

        if  "cls" in msg:
            os.system("cls")
            print("==========================================")
            print("MQTT Telex Sender")
            print("Type messages and press Enter.\nType 'quit' to exit.\nor Type the command  or number ")
            print("==========================================")
            print(cmd)

        if  "list" in msg:
            os.system("cls")
            print("==========================================")
            print("MQTT Telex Sender")
            print("Type messages and press Enter.\nType 'quit' to exit.\nor Type the command  or number ")
            print("==========================================")
            print(cmd)
        if msg.lower() == "?":
            print("==========================================")
            print("MQTT Telex Sender")
            print("Type messages and press Enter.\nType 'quit' to exit.\nor Type the command  or number ")
            print("==========================================")
            print(cmd)

        if  "cnt" in msg:
            #os.system("cls")
            print("============Controller serials ===========")
            print(cnt)


except KeyboardInterrupt:
    pass

client.loop_stop()
client.disconnect()


