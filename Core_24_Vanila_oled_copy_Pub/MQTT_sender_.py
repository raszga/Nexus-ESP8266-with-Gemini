# -*- coding: utf-8 -*-
r"""/* Developed with Gemini
     Modified by Claude (Sonnet 4.5) - 2026-01-30:
       - Added config.ini support for credentials
       - Credentials no longer hardcoded
           /|\
          --+--
           \|/
*/
"""
import curses
import time
import ssl
import random
import paho.mqtt.client as paho
import sys
import configparser
import os

# --- LOAD CONFIG FROM FILE ---
def load_config():
    """Load configuration from config.ini file"""
    config = configparser.ConfigParser()
    
    # Try to load config.ini
    if os.path.exists('config.ini'):
        config.read('config.ini')
    else:
        print("\n[ERROR] config.ini not found!")
        print("Please copy config.ini.template to config.ini and fill in your credentials.")
        sys.exit(1)
    
    return config

config = load_config()

# --- CONFIG FROM FILE ---
BROKER = config.get('mqtt', 'broker')
PORT = config.getint('mqtt', 'port')
TOPIC = config.get('mqtt', 'topic')
USERNAME = config.get('mqtt', 'username')
PASSWORD = config.get('mqtt', 'password')

MIN_COLS = config.getint('defaults', 'min_cols', fallback=100)
MIN_LINES = config.getint('defaults', 'min_lines', fallback=25)

COMMAND_DATA = [
    ("Check", "Response all"),
    ("Reset", "Reset all"),
    ("Silent", "Ignore alarm"),
    ("R_ON", "Relay ON"),
    ("R_OFF", "Relay OFF"),
    ("F_ON", "Fan ON"),
    ("F_OFF", "Fan OFF"),
    ("Diff_OFF", "Stop Diff"),
    ("Diff_FULL", "Full power"),
    ("*FXXXX", "PWM 0-1023"),
    ("*F0", "Stop Fan"),
    ("*Ixxx", "Internal  Fan 10 bit"),
    (">>PULSE-ER", "Pulse Reset"),
    ("QRcode", "Gen QR code"),
    ("cls", "Hard Redraw"),
    ("quit", "Exit")
]

SERIAL_DATA = [
    ("2345360", "Key/Hall"),
    ("14706265", "Power"),
    ("1056332", "Water 2"),
    ("737757", "Basement"),
    ("1000075", "Water 1"),
    ("991347", "Spare"),
    ("3068210", "Fan/Net"),
    ("15466192", "ESP32 Rig"),
    ("871864", "Fan/Net")
]

CompName = f"LPTP-CMD-{random.randint(100,999)}"

class TelexUI:
    def __init__(self, stdscr):
        self.stdscr = stdscr
        self.history = []
        self.target_serial = "All"
        self.pulse = False
        self.is_connected = False
        self.init_windows()

    def init_windows(self):
        self.h, self.w = self.stdscr.getmaxyx()
        self.col1_w, self.col2_w = 30, 30
        self.col3_w = max(10, self.w - self.col1_w - self.col2_w)
        
        self.head_win = curses.newwin(3, self.w, 0, 0)
        self.menu_win = curses.newwin(self.h-6, self.col1_w, 3, 0)
        self.ser_win  = curses.newwin(self.h-6, self.col2_w, 3, self.col1_w)
        self.log_win  = curses.newwin(self.h-6, self.col3_w, 3, self.col1_w + self.col2_w)
        self.in_win   = curses.newwin(3, self.w, self.h-3, 0)
        
        curses.use_default_colors()
        curses.init_pair(1, curses.COLOR_CYAN, -1)   # Title
        curses.init_pair(2, curses.COLOR_GREEN, -1)  # All/OK
        curses.init_pair(3, curses.COLOR_MAGENTA, -1)# Input
        curses.init_pair(4, curses.COLOR_YELLOW, -1) # Date/Time
        curses.init_pair(5, curses.COLOR_RED, -1)    # Target Locked / Error

    def hard_reset(self):
        self.stdscr.clear()
        curses.resize_term(*self.stdscr.getmaxyx())
        self.init_windows()
        self.redraw()

    def redraw(self):
        self.pulse = not self.pulse
        pulse_char = "*" if self.pulse else "o"
        
        # Header
        self.head_win.erase()
        self.head_win.box()
        try:
            self.head_win.addstr(1, 2, f"TELEX SENDER | Config: {BROKER[:20]}", curses.color_pair(1))
        except curses.error:
            pass
        
        # DateTime & Heartbeat
        dt_str = time.strftime('%Y-%m-%d %H:%M:%S')
        hb_text = f"[{pulse_char}] {dt_str}"
        try:
            self.head_win.addstr(1, self.w - len(hb_text) - 2, hb_text, curses.color_pair(4))
        except curses.error:
            pass
        
        # Connection Status & Target Lock
        conn_style = curses.color_pair(2) if self.is_connected else curses.color_pair(5)
        conn_text = "ONLINE" if self.is_connected else "OFFLINE"
        
        t_style = curses.color_pair(5) if self.target_serial != "All" else curses.color_pair(2)
        target_txt = f"LOCK: {self.target_serial}"
        
        if self.w > 85:
            status_line = f"[{conn_text}]  {target_txt}"
            try:
                self.head_win.addstr(1, self.w - len(hb_text) - len(status_line) - 10, status_line, t_style | curses.A_BOLD)
            except curses.error:
                pass
        
        self.head_win.refresh()

        # Columns
        self.menu_win.erase()
        self.menu_win.box()
        try:
            self.menu_win.addstr(0, 2, " [CMD] ")
        except curses.error:
            pass
            
        for i, (n, _) in enumerate(COMMAND_DATA):
            if i < self.h-8:
                try:
                    self.menu_win.addstr(i+1, 1, f"{i:>2}", curses.color_pair(4))
                    self.menu_win.addstr(i+1, 5, n)
                except curses.error:
                    pass
        self.menu_win.refresh()

        self.ser_win.erase()
        self.ser_win.box()
        try:
            self.ser_win.addstr(0, 2, " [TARGETS] ")
        except curses.error:
            pass
            
        for i, (s, _) in enumerate(SERIAL_DATA):
            if i < self.h-8:
                try:
                    self.ser_win.addstr(i+1, 1, f"{chr(97+i)})", curses.color_pair(4))
                    self.ser_win.addstr(i+1, 5, s, curses.color_pair(2))
                except curses.error:
                    pass
        self.ser_win.refresh()

        self.log_win.erase()
        self.log_win.box()
        try:
            self.log_win.addstr(0, 2, " [LOG] ")
        except curses.error:
            pass
            
        for i, entry in enumerate(self.history[-(self.h-8):]):
            try:
                self.log_win.addstr(i+1, 1, entry[:self.col3_w-2])
            except curses.error:
                pass
        self.log_win.refresh()

        self.in_win.erase()
        self.in_win.box()
        try:
            self.in_win.addstr(1, 2, "CMD > ", curses.color_pair(3))
        except curses.error:
            pass
        self.in_win.refresh()

def main_telex(stdscr):
    ui = TelexUI(stdscr)
    if ui.w < MIN_COLS or ui.h < MIN_LINES:
        curses.endwin()
        print(f"\n[ERROR] Terminal too small: {ui.w}x{ui.h}. Need at least {MIN_COLS}x{MIN_LINES}.")
        sys.exit(1)

    # MQTT Callbacks for Connection Status
    def on_connect(client, userdata, flags, rc, properties=None):
        ui.is_connected = (rc == 0)
        ui.history.append(f"MQTT: Connected (rc={rc})")
        ui.redraw()

    def on_disconnect(client, userdata, rc, properties=None):
        ui.is_connected = False
        ui.history.append(f"MQTT: Disconnected (rc={rc})")
        ui.redraw()

    client = paho.Client(client_id=CompName, protocol=paho.MQTTv5, callback_api_version=paho.CallbackAPIVersion.VERSION2)
    client.on_connect = on_connect
    client.on_disconnect = on_disconnect
    client.tls_set(tls_version=ssl.PROTOCOL_TLS_CLIENT)
    client.username_pw_set(USERNAME, PASSWORD)
    
    try:
        client.connect(BROKER, PORT)
        client.loop_start()
    except Exception as e:
        ui.is_connected = False
        ui.history.append(f"MQTT Error: {str(e)[:40]}")

    ui.redraw()
    
    while True:
        curses.echo()
        ui.in_win.move(1, 8)
        try: 
            user_in = ui.in_win.getstr(1, 8, 32).decode('utf-8', errors='ignore').strip()
        except:
            continue
        
        curses.noecho()
        
        if not user_in:
            ui.redraw() 
            continue
        
        # Handle target selection (a-z)
        if len(user_in) == 1 and 'a' <= user_in <= 'z':
            idx = ord(user_in) - 97
            if idx < len(SERIAL_DATA):
                ui.target_serial = SERIAL_DATA[idx][0]
                ui.history.append(f"Target: {ui.target_serial}")
            ui.redraw()
            continue
            
        # Handle "all" target reset
        if user_in.lower() == "all":
            ui.target_serial = "All"
            ui.history.append("Target: All")
            ui.redraw()
            continue
        
        # Process command
        cmd = user_in
        if user_in.isdigit() and int(user_in) < len(COMMAND_DATA): 
            cmd = COMMAND_DATA[int(user_in)][0]
        
        # Construct and publish payload
        payload = f"{ui.target_serial} {cmd}"
        full_message = f"{CompName}:\n{payload}"
        
        try:
            client.publish(TOPIC, full_message)
            ui.history.append(f"OUT: {payload}")
        except Exception as e:
            ui.history.append(f"ERR: {str(e)[:30]}")
        
        ui.redraw()

        # Handle special commands
        if user_in.lower() == "quit":
            break
        if user_in.lower() == "cls":
            ui.hard_reset()
            continue

    client.disconnect()
    client.loop_stop()

if __name__ == "__main__":
    try:
        curses.wrapper(main_telex)
    except KeyboardInterrupt:
        print("\n[EXITED] Ctrl+C detected. Goodbye!")
    except Exception as e:
        print(f"\n[ERROR] {e}")
        sys.exit(1)
