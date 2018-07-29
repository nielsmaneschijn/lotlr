/*
 Basic ESP8266 MQTT example

 This sketch demonstrates the capabilities of the pubsub library in combination
 with the ESP8266 board/library.

 It connects to an MQTT server then:
  - publishes "hello world" to the topic "outTopic" every two seconds
  - subscribes to the topic "inTopic", printing out any messages
    it receives. NB - it assumes the received payloads are strings not binary
  - If the first character of the topic "inTopic" is an 1, switch ON the ESP Led,
    else switch it off

 It will reconnect to the server if the connection is lost using a blocking
 reconnect function. See the 'mqtt_reconnect_nonblocking' example for how to
 achieve the same result without blocking the main loop.

 To install the ESP8266 board, (using Arduino 1.6.4+):
  - Add the following 3rd party board manager under "File -> Preferences -> Additional Boards Manager URLs":
       http://arduino.esp8266.com/stable/package_esp8266com_index.json
  - Open the "Tools -> Board -> Board Manager" and click install for the ESP8266"
  - Select your ESP8266 in "Tools -> Board"

*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>  
// Update these with values suitable for your network.

// const char* ssid = "*";
// const char* password = "*";
const char* mqtt_server = "192.168.0.3";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

float temp;
float solar;

void setup_wifi() {
  WiFiManager wifiManager;
  wifiManager.autoConnect("Ledringklok");
  
//   delay(10);
//   // We start by connecting to a WiFi network
//   Serial.println();
//   Serial.print("Connecting to ");
//   Serial.println(ssid);

//   WiFi.begin(ssid, password);

//   while (WiFi.status() != WL_CONNECTED) {
//     delay(500);
//     Serial.print(".");
//   }

//   Serial.println("");
//   Serial.println("WiFi connected");
//   Serial.println("IP address: ");
//   Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  
  String message = "";
  String topics = topic;

  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    message += (char)payload[i];
  }
  Serial.println();
  Serial.println(message);
  Serial.println(topics);
  if (topic[0] == 's') {
      solar = message.toFloat();
  }

  if (topic[0] =='/') {
      temp = message.toFloat();
  }

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is acive low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }

Serial.println(temp);
Serial.println(solar);
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("test", "hello world");
      // ... and resubscribe
      client.subscribe("/temp/picaxe");
      client.subscribe("solar/power");
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
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  client.publish("test", "ledringklok online");
}
void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

//   long now = millis();
//   if (now - lastMsg > 2000) {
//     lastMsg = now;
//     ++value;
//     snprintf (msg, 75, "hello world #%ld", value);
//     Serial.print("Publish message: ");
//     Serial.println(msg);
//     client.publish("test", msg);
//   }
}

// #include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

// //needed for library
// #include <DNSServer.h>
// #include <ESP8266WebServer.h>
// #include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager

// //mqtt

// #include <SPI.h>
// #include <Ethernet.h>
// #include <PubSubClient.h>

// // Update these with values suitable for your network.
// byte mac[]    = {  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED };
// IPAddress ip(192, 168, 0, 148);
// IPAddress server(192, 168, 0, 3);

// void callback(char* topic, byte* payload, unsigned int length) {
//   // handle message arrived********
//   Serial.println("mqtt bericht");

// }

// EthernetClient ethClient;
// PubSubClient client(server, 1883, ethClient);


// void setup() {
//     // put your setup code here, to run once:
//     Serial.begin(115200);
// Serial.println("moi eem'");
//     //WiFiManager
//     //Local intialization. Once its business is done, there is no need to keep it around
//     WiFiManager wifiManager;
//     //reset saved settings
//     //wifiManager.resetSettings();
    
//     //set custom ip for portal
//     //wifiManager.setAPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));

//     //fetches ssid and pass from eeprom and tries to connect
//     //if it does not connect it starts an access point with the specified name
//     //here  "AutoConnectAP"
//     //and goes into a blocking loop awaiting configuration
//     wifiManager.autoConnect("Ledringklok");
//     //or use this for auto generated name ESP + ChipID
//     //wifiManager.autoConnect();

    
//     //if you get here you have connected to the WiFi
//     Serial.println("connected...yeey :)");

//     //mqtt
//       Ethernet.begin(mac, ip);
//   // Note - the default maximum packet size is 128 bytes. If the
//   // combined length of clientId, username and password exceed this,
//   // you will need to increase the value of MQTT_MAX_PACKET_SIZE in
//   // PubSubClient.h
  
//   if (client.connect("ledringklok")) {
//       Serial.println("subscribed");
//     client.publish("test","hello world");
//     client.subscribe("test");
//   }
// Serial.println("dit was de setup!");
// }

// /*
//  Basic MQTT example with Authentication

//   - connects to an MQTT server, providing username
//     and password
//   - publishes "hello world" to the topic "outTopic"
//   - subscribes to the topic "inTopic"
// */


// void loop()
// {
//   client.loop();
//   Serial.println(client.state());
// }
