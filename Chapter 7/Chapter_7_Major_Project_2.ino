 /*
 * Major Project 2 Code
 * Project Name : Smart Switching System
 * Developer : Dhairya Parikh
 * Description : This code enables you to control your Smart Home Relay system through your Node RED 
 * Dashboard.  
 */

// Importing the required Libraries
#include <WiFi.h>   // WiFi functionality access for ESP32
#include <PubSubClient.h>   // Enables the use of MQTT 

// WiFi and MQTT Credentials
const char* ssid = "wifi_ssid";
const char* password = "wifi_password";
const char* mqtt_server = "Pi's ip address";

// Other Variables and object declarations
int relay1 = 15;
int relay2 = 2;
int relay3 = 4;
int relay4 = 22;

WiFiClient espClient;
PubSubClient client(espClient);

// Custom function for Wifi connection establishment
void setup_wifi() 
{
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

// ########################################################################################

// ########################################################################################

// Callback function: Called every time a message is received on one of the subscribed topics.

void callback(char* topic, byte* message, unsigned int length) 
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  // Relay Control Logic starts here.
  
  if (String(topic) == "IoTSmartSwitches/Switch1") {
    Serial.print("Changing output to ");
    if(messageTemp == "1"){
      Serial.println("Switch 1 turned On");
      digitalWrite(relay1, HIGH);
    }
    else if(messageTemp == "0"){
      Serial.println("Switch 1 turned off");
      digitalWrite(relay1, LOW);
    }
  }

  else if (String(topic) == "IoTSmartSwitches/Switch2") {
    Serial.print("Changing output to ");
    if(messageTemp == "1"){
      Serial.println("Switch 2 turned On");
      digitalWrite(relay2, HIGH);
    }
    else if(messageTemp == "0"){
      Serial.println("Switch 2 turned off");
      digitalWrite(relay2, LOW);
    }
  }
  
  else if (String(topic) == "IoTSmartSwitches/Switch3") {
    Serial.print("Changing output to ");
    if(messageTemp == "1"){
      Serial.println("Switch 3 turned On");
      digitalWrite(relay3, HIGH);
    }
    else if(messageTemp == "0"){
      Serial.println("Switch 3 turned off");
      digitalWrite(relay3, LOW);
    }
  }

  else if (String(topic) == "IoTSmartSwitches/Switch4") {
    Serial.print("Changing output to ");
    if(messageTemp == "1"){
      Serial.println("Switch 4 turned On");
      digitalWrite(relay4, HIGH);
    }
    else if(messageTemp == "0"){
      Serial.println("Switch 4 turned off");
      digitalWrite(relay4, LOW);
    }
  }
}

// ########################################################################################

// ########################################################################################

// Reconnect Function : Called when the client disconnects from the MQTT Broker. 
// This function helps in reconnecting to it. 

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESPClient-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) 
    {
      Serial.println("connected");
      client.publish("outTopic", "Reconnected!");
      
      // Subscribe to all the relevant topics
      client.subscribe("IoTSmartSwitches/Switch1");
      client.subscribe("IoTSmartSwitches/Switch2");
      client.subscribe("IoTSmartSwitches/Switch3");
      client.subscribe("IoTSmartSwitches/Switch4");
      
    } 
    else 
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

// ########################################################################################

// ########################################################################################

// Arduino Setup Function

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  client.subscribe("IoTSmartSwitches/Switch1");
  client.subscribe("IoTSmartSwitches/Switch2");
  client.subscribe("IoTSmartSwitches/Switch3");
  client.subscribe("IoTSmartSwitches/Switch4");
      
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  pinMode(relay3, OUTPUT);
  pinMode(relay4, OUTPUT);
  
  digitalWrite(relay1, LOW);
  digitalWrite(relay2, LOW);
  digitalWrite(relay3, LOW);
  digitalWrite(relay4, LOW);
  
}

// ########################################################################################

// ########################################################################################

// Arduino Loop Function

void loop() {
  // put your main code here, to run repeatedly:
  if (!client.connected()) {
    reconnect();
  }
  client.loop(); 
}

// ########################################################################################
