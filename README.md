# smart_blinds
Wifi smart blinds

Wifi controlled module to create smart blinds from ordinary blinds.

List of parts: 
l298n dual h-bridge motor driver
esp8266wifi
DC 12V Power supply
DC 12V Geared Motor (recommended around 30RPM) 

Requirements:
working MQTT server

Guide:
Alter blinds.ino, replace all CHANGE_ME_ statements, also change pinout and or blinds names as requested
Integrate into Home Assistant using HomeAssistant.yml script
