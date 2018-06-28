# Surf Actuator
Arduino GPS Surf Actuator controller 

![Surfgate](https://raw.githubusercontent.com/jonthompson/surfgate/master/surfgate.png)

An Arduino/GPS controller to automatically deploy and retract your actuator controlled DIY wake gates/tabs at apprioriate speeds.

Depends on https://github.com/SlashDevin/NeoGPS/

If you are not using an Arduino Mega 2560 you'll also need either https://github.com/PaulStoffregen/AltSoftSerial or https://github.com/SlashDevin/NeoSWSerial

# Parts List (Simple):
Pick an Arduino:
* Uno R3 (https://www.amazon.com/ATmega328P-Development-Compatible-Develope-Microcontroller/dp/B01AR7YJ3O)
OR
* Nano: https://www.amazon.com/HiLetgo-ATmega328P-Micro-controller-Development-Compatible/dp/B00E87VWY4/

If you want a screw shield instead of soldering (recommended), the nano is likely a better option (https://www.amazon.com/Aideepen-Terminal-Adapter-Expansion-ATMEGA328P-AU/dp/B0788MLRLK/ref=sr_1_1?ie=UTF8&qid=1527559143&sr=8-1&keywords=nano+screw+shield)

* L298N Motor Bridge (https://www.amazon.com/Stepper-Controller-Mega2560-Duemilanove-IFANCY-TECH/dp/B01GZ1QUHO  (only need 1, but a backup doesn't hurt)

* LM2596 Buck Converter (https://www.amazon.com/eBoot-LM2596-Converter-3-0-40V-1-5-35V/dp/B01GJ0SC2C (only need 1 but they are super handy and cheap)

* GPS Module with antenna (https://www.amazon.com/dp/B01MRNN3YZ) 

* Any 3 way switch you want (SPDT - not momentary) - IE: https://www.amazon.com/Blue-Sea-Systems-WeatherDeck-Toggle/dp/B000MMFJ02

* If you want a status LED grab one you like (9v - 12v)


# Wiring Instructions
## Wiring
Since everything except the actuators is very low draw all wiring can be 18-20AWG

## Power:
REMOVE 5V REGULATOR JUMPER ON L298N.  This is a small jumper by itself on the board.
Wire power to the LM2596 "in" solder pads.  Wire this to something that is not always powered or you'll kill your battery.
Also solder a lead onto the LM2596 "in" solder pads to go directly to the L298N 12V in.
Connect your LM2596 to a 12V source after turning the gold dial counter-clockwise 15-20 times.  Use a multimeter to measure out pads until you get to 7-8V out.
Solder wire on the LM2596 "out" solder pads to run to VIN/GND on arduino

## GPS
Solder or connect 3v3 (+) and GND (-) to arduino pins.
Connect RX/TX to pin D8/D9 on arduino

## L298N
Connect 5v and GND to the arduino
Connect the 4 IN1/IN2/IN3/IN4 to A2/A3/A4/A5
Connect the OUT1 and OUT2 to the actuator relay at the back of the boat.
Connect the OUT3 and OUT4 to the actuator relay at the back of the boat.

## Wiring controls:
### Encoder:
Wire D5 to DT 
D4 to CLK
D2 to ROTBUTTON (Might be SWT/SWTCH on yours, its the open pin)

### LED:
D3 - LED (I used a 12V LED but am feeding it 7v and its fine.)

### SWITCH:
D10 - LEFT DEPLOY
D11 - RIGHT DEPLOY
GND (and GND for encoder/light)

# Install




<div>Icons made by <a href="http://www.freepik.com" title="Freepik">Freepik</a> from <a href="https://www.flaticon.com/" title="Flaticon">www.flaticon.com</a> is licensed by <a href="http://creativecommons.org/licenses/by/3.0/" title="Creative Commons BY 3.0" target="_blank">CC 3.0 BY</a></div>
