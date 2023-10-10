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

Read more on the Project Write-up [here](https://www.nyctinker.com/post/attiny85-projects-diy-wireless-room-sensor)
