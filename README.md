# esp_billy
DIY Motion and Temperature sensor using an Attiny85 and ESP8366 (Esp12-e).


Key Features
 - Reads PIR sensor output to detect motion
 - Reads ambient room temperature
 - Periodically publishes status over MQTT for easy integration with Home Assistant
 - Low power consumption (battery operated) with cutoff to avoid hurting batteries

Key Details
 - For Arduino IDE and uses standard Attinycore, WiFi manager, esp8366, MQTT libraries
 - Low power tricks include sleep and PCI/WDT wake up), mosfet switching off ESP8266, and Fast connect more for WiFi (eg static IP)
 - Can last 4-6 weeks between battery changes using 4 x AAA Nimh 

Read more on the Project Write-up [here](https://www.nyctinker.com/post/attiny85-projects-diy-wireless-room-sensor)
