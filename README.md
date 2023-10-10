# esp_billy
DIY Motion and Temperature sensor using an Attiny85 and ESP8366 (Esp12-e).


Key Features
 - Reads PIR sensor output to detect motion
 - Reads ambient room temperature
 - Periodically publishes status over MQTT for easy integration with Home Assistant
 - Low power (battery operated) with cutoff to avoid hurting batteries

Key Details
 - For Arduino IDE and uses standard [Attinycore](https://github.com/SpenceKonde/ATTinyCore), [WiFi manager](https://github.com/tzapu/WiFiManager), MQTT [Pubsub](https://github.com/knolleary/pubsubclient) libraries
 - Low power tricks include Attiny sleep and PCI/WDT wake up, mosfet switching off ESP8266, and [Fastconnect](https://github.com/tzapu/WiFiManager/issues/1342) for reduced WiFi boot time (eg static IP)
 - Can last 4-6 weeks between battery changes using 4 x AAA Nimh


BOM
  - 1 x Attiny85
  - 1 x ESP8266: I used an ESP12-E with those [adapters](https://www.iot-experiments.com/fashing-the-esp12e-with-the-breadboard-adapter/))
  - 1 x [MCP1825s](https://www.microchip.com/en-us/product/mcp1825s): 500mA LDO to regulate 3.3V-needy ESP
  - 1 x [IRLZ44N](https://www.infineon.com/cms/en/product/power/mosfet/n-channel/irlz44n/): N channel logic-level Mosfet
  - Multiple ceramic capacitors: 4.7 uF, 1 uF for LDO output; 0.1 uF for decoupling cap on Attiny85
  - 1 x 10 uF electrolytic capacitor for ESP-12E
  - 3 x 10k resistors
  - 1 x PIR sensor (I ended up tapping into the one attached to this [light](https://www.amazon.com/Transolid-SA9030HDALWH-Battery-Operated-Aluminum/dp/B01BBMF8R0)

Read more on the Project Write-up [here](https://www.nyctinker.com/post/attiny85-projects-diy-wireless-room-sensor)
