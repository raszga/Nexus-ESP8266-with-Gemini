@echo off
:: Sender Launch
start "SenderXX" cmd /k "mode con: cols=100 lines=30 && call C:\Users\raszg\anaconda3\Scripts\activate.bat && conda activate C:\Users\raszg\anaconda3\envs\Work_0 && python e:\_Arduino\_ESP32_8266_Common\Common\UI_Python\MQTT_sender_old.py && exit"
:: Panel Launch
start "Rec   XX" cmd /k "mode con: cols=100 lines=30 && call C:\Users\raszg\anaconda3\Scripts\activate.bat && conda activate C:\Users\raszg\anaconda3\envs\Work_0 && python e:\_Arduino\_ESP32_8266_Common\Common\UI_Python\MQTT_receiver_old.py && exit"