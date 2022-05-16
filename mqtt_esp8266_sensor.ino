/*
 ESP8266 Billy code based off of sample ESP8266 sketch for pubsub function
 Basic MQTT pub functionality based on Serial messages sent from Attiny85 Billy
*/

//#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiManager.h>

// HARDCODE TO START TODO: REMOVE

//const char* ssid = "shanghaischav";  // TODO: REPLACE WITH WIFI MANAGER
//const char* password = "jujujuju";
//  Static IP address
IPAddress local_IP(192, 168, 86, 29);
// Set your Gateway IP address
IPAddress gateway(192, 168, 86, 1);
IPAddress subnet(255, 255, 255, 0);
const char* mqtt_server = "192.168.86.230";
const char* mqtt_username = "mqtt_client";
const char* mqtt_password = "Mqtt3dl3p";
const char* outTopic = "billies/billy1/recent_presence";
const char* detectedTrue = "true";
const char* detectedFalse = "false";
WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;
long startTime = 0;

void setup_wifi() {
  long connectedTime;
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Billy Connecting... ");
  //Serial.println(ssid);

  // Configures static IP address
  /*if (!WiFi.config(local_IP, gateway, subnet)) {
    Serial.println("STA Failed to configure");
  }*/

  WiFi.mode(WIFI_STA);
  WiFiManager wm;
  //WiFi.begin(ssid, password);
  // Automatically connect using saved credentials,
  // if connection fails, it starts an access point with the specified name ( "AutoConnectAP"),
  // if empty will auto generate SSID, if password is blank it will be anonymous AP (wm.autoConnect())
  // then goes into a blocking loop awaiting configuration and will return success result

  bool res;
  // res = wm.autoConnect(); // auto generated AP name from chipid
  // res = wm.autoConnect("AutoConnectAP"); // anonymous ap
  // Set static IP for faster connect time
  wm.setSTAStaticIPConfig(local_IP, gateway, subnet); // optional DNS 4th argument

  res = wm.autoConnect("AutoConnectAP", "password");  // password protected ap
  if (!res) {
    Serial.println("Failed to connect");

  } else {
    //if you get here you have connected to the WiFi
    Serial.println("connected...yeey :)");
    connectedTime = millis() - startTime;
    randomSeed(micros());
    Serial.print("Connected in: ");
    Serial.println(connectedTime);  // Typical time = 6200 ms (DHCP), 4100 ms (static IP), Wifimanger DHCP (2100 MS); Wifi manager static (250 ms)
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  }
  /*while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("."); Serial.println(WiFi.status());
  }
  connectedTime = millis() - startTime;
*/
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  /*if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }*/
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish(outTopic, "hello");
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  //pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  startTime = millis();
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Received command from Tiny Billy
  if (Serial.available()) {
    Serial.println("received Tiny Billy command");
    int readLength = Serial.readBytes(msg, MSG_BUFFER_SIZE);
    /*if (tinyBillyCommand == 1) {
     // publish Detect message
     strcpy(msg, "true");
   } else if (!tinyBillyCommand) {
     // publish not detect message
     strcpy(msg, "false");
   }*/
    client.publish(outTopic, msg);
  }
  /* unsigned long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    ++value;
    snprintf (msg, MSG_BUFFER_SIZE, "1", value);
    Serial.print("Publish message: ");*/
}
