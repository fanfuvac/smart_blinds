# smart_blinds
Wifi smart blinds

Wifi controlled module to create smart blinds from ordinary blinds.
Supports 2 blinds with one controller, can be easily rewritten to support just 1 blind

**List of parts:**

l298n dual h-bridge motor driver

esp8266wifi (e.g. WeMos D1 Mini)

DC 12V Power supply (1A)

DC 12V Geared Motor (recommended around 30RPM) 

**Requirements:**

working MQTT server

**Guide:**

Alter blinds.ino, replace all CHANGE_ME_ statements, also change pinout and or blinds names as requested
Integrate into Home Assistant using HomeAssistant.yml script
