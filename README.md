# surfgate
Arduino GPS surfgate controller

An Arduino and GPS module controller to automatically deploy and retract your actuator controlled DIY wake gates/tabs at apprioriate speeds.

Depends on https://github.com/SlashDevin/NeoGPS/

If you are not using an Arduino Mega 2560 you'll also need either https://github.com/PaulStoffregen/AltSoftSerial or https://github.com/SlashDevin/NeoSWSerial

The Mega adds 3 additional hardware serial ports and a ton of extra memory so it is advised to use that if possible.

Suggested configuration change to arduino source: edit hardware/arduino/avr/cores/arduino/HardwareSerial.h change SERIAL_TX_BUFFER_SIZE and SERIAL_RX_BUFFER_SIZE to 256.  This will improve GPS buffers improving reliability.


# surfgate_simple
A simple start for the gate controls - 3 buttons (off, left, right) and controls for actuators via 4 pins.  Start here.  See simple_fritzing for wiring.


# surfgate_wifi
Adding on to surfgate_simple - adding an esp8266 module via serial running in SoftAP mode to configure and control your surfgate from your phone

# surfgate_complete
A kitchen-sink version including support for OLED screen for status reporting, pot for fine tuning, wifi module via serial for configuration/phone control and remote programming, and an RF receiver for a DIY surfband.
