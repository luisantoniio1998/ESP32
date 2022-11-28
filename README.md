# ESP32

IOT Project developed with ESP32 and UBIDOTS broker using ArduinoIDE
The program registers the temperature and humidity levels via the ESP32 board and proper sensors and sends it to the broker. 
Via the broker(UBIDOTS) we can turn off or on a LED attached to the ESP32, control the brightness of another one. And accordingly to the temperature levels the broker sends a signal to turn off or on the third LED attached to the ESP32.
Histerysis implemented(same concept used in AC)
