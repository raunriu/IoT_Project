
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "DHTesp.h"

#define DHTTYPE DHT22
#define DHTPIN 7
#define RELAY 6

//DHT dht(DHTPIN, DHTTYPE);
DHTesp dht;
String packet;

// Update these with values suitable for your network.

const char* ssid = "hotspot3883";
const char* password = "a1111111";
const char* mqtt_server = "piflask.iptime.org";
//const char* mqtt_server = "broker.mqtt-dashboard.com";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];

int value = 0;

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  String subscribe_data = "";
//  Serial.print("Message arrived [");
//  Serial.print(topic);
//  Serial.print("] ");
  for (int i = 0; i < length; i++) {
//    Serial.print((char)payload[i]);
    subscribe_data += (char)payload[i];
//    Serial.println(subscribe_data);
  }
//  Serial.print(subscribe_data);
  // Switch on the LED if an 1 was received as first character

  Serial.write((char*)subscribe_data.c_str());
  
//  if ((char)payload[0] == '1') {
//    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
//    // but actually the LED is on; this is because
//    // it is active low on the ESP-01)
//  } else {
//    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
//  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
//      client.publish("/shc/heater", "hello world");
      // ... and resubscribe
      client.subscribe("heater");
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
  //pinMode(RELAY, OUTPUT);
  //dht.setup(DHTPIN, DHTesp::DHT22);
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  String strTemp = "";
  float temp = 0;
  float humi = 0;

  if (now - lastMsg > 5000) {
    lastMsg = now;
    ++value;
    if(Serial.available())
    {
      strTemp = Serial.readString();

      temp = strTemp.substring(0,5).toFloat();
      humi = strTemp.substring(5,10).toFloat();
      
//      Serial.print("Publish Temp: ");
//      Serial.println(strTemp.substring(0,5));
//      Serial.print("Publish Humi: ");
//      Serial.println(strTemp.substring(5,10));
      
      snprintf (msg, MSG_BUFFER_SIZE, "{\"temp\" : %.2f, \"humi\" : %.2f}", temp, humi);
      client.publish("/shc/heater", msg);
    }
  }
}
