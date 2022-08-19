/*
 * Project 2 : Node MCU LED control using Node RED and MQTT
 * Developer : Dhairya Parikh
 */

// Import the required Libraries
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Update these with credentials suitable for your network.
const char* ssid = "wifi_ssid";
const char* password = "wifi_password";
const char* mqtt_server = "Pi's ip address";

// Variable and Object Declaration
WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

// Custom function to connect to the specified WiFi network
void setup_wifi() 
{
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

// Callback Function to print out the payload arriving on the subscribed topics.
void callback(char* topic, byte* payload, unsigned int length) 
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
// Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') 
  {
    digitalWrite(BUILTIN_LED, LOW);       
  }
 else 
 {
    digitalWrite(BUILTIN_LED, HIGH);  
 }
}

// Function to reconnect to MQTT server if it gets disconnected for some reason. 
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
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("inTopic/LED");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

// Main setup function
void setup() {
  
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);      // Serial port opened with baud rate 115200
  setup_wifi();
  client.setServer(mqtt_server, 1883);   // Connect to the MQTT Server
  client.setCallback(callback);     // Define the Callback Function 
  client.subscribe("inTopic/LED");      // Subscribe to this topic
}

// Main Loop Function
void loop() {

  if (!client.connected()) {
    reconnect();   // Run the reconnect function till a connection to the MQTT server is established
  }
  client.loop();
  unsigned long now = millis();
  if (now - lastMsg > 2000) {
  lastMsg = now;
  ++value;
  snprintf (msg, MSG_BUFFER_SIZE, "hello world #%ld",
  value);
  Serial.print("Publish message: ");
  Serial.println(msg);
  client.publish("outTopic", msg);
  }
}

// --------------- End of Code -----------------
