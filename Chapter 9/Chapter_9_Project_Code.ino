/*
 * Packt Book - Raspberry Pi and MQTT 
 * Author - Dhairya Parikh
 * Description - This code is written for the NodeMCU Development Board. This will let you connect to 
 * AWS IoT Core and publish BMP280 sensor data on a ESP8266/publish topic. Moreover, it will let you subscribe
 * to the ESP8266/subscribe topic and print any messages on the Serial monitor that arrive on this topic.
 */

// ------------------ Importing the required Libraries -------------------------

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include <Wire.h>
#include <Adafruit_BMP280.h>
#include "secrets.h"      // We will create this file to store all our sensitive data. Just go to the down arrow button on the top right and press on "Create New Tab"

// ----------------------------------------------------------------------------

// --------------------- Variable, Object and constant Declarations -------------

// BMP 280 Sensor value variables
float temperature_C ;
float pressure ;
float altitude ;

// Variables to implement publish every 5 second logic
unsigned long lastMillis = 0;
unsigned long previousMillis = 0;
const long interval = 5000;

// variables to store the latest time fetched by the NTP Client.
time_t now;
time_t nowish = 1510592825;

// AWS Publish and subscribe topic mentioned in the AWS policy you created 
#define AWS_IOT_PUBLISH_TOPIC   "ESP8266/publish"
#define AWS_IOT_SUBSCRIBE_TOPIC "ESP8266/subscribe"

// WiFi SSL to add the certificates and key we retrived from AWS 
WiFiClientSecure net;
BearSSL::X509List cert(cacert);
BearSSL::X509List client_crt(client_cert);
BearSSL::PrivateKey key(privkey);

// Objects for pubsub and BMP sensor
PubSubClient client(net);
Adafruit_BMP280 bmp; 

// --------------------------------------------------------------------------

// NTPConnect : This function fetches the latest time. This is very important for authentication of the certificate and time is an essential factor. 

void NTPConnect(void)
{
  Serial.print("Setting time using SNTP");
  configTime(TIME_ZONE * 3600, 0 * 3600, "pool.ntp.org", "time.nist.gov");
  now = time(nullptr);
  while (now < nowish)
  {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("done!");
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.print("Current time: ");
  Serial.print(asctime(&timeinfo));
}

// ---------------------------------------------------------------------------------------------

// Callback Funtion to print any incoming messages on the subscribed topics

void messageReceived(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Received [");
  Serial.print(topic);
  Serial.print("]: ");
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}
 
// ---------------------------------------------------------------------------------------------

// Custom function for Wifi connection establishment

void Setup_WiFi() 
{
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println(String("Attempting to connect to SSID: ") + String(WIFI_SSID));
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }
  randomSeed(micros());
  Serial.println("");
  Serial.println("WiFi connected!");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

// ---------------------------------------------------------------------------------------------

// Function to connect to the AWS MQTT endpoint and subscribe to the specified topic

void connectAWS()
{
  delay(3000);

  Setup_WiFi();
  NTPConnect();
  
  net.setTrustAnchors(&cert);
  net.setClientRSACert(&client_crt, &key);
 
  client.setServer(MQTT_HOST, 8883);
  client.setCallback(messageReceived);
 
 
  Serial.println("Connecting to AWS IOT");
 
  while (!client.connect(THINGNAME))
  {
    Serial.print(".");
    delay(1000);
  }
 
  if (!client.connected()) {
    Serial.println("AWS IoT Timeout!");
    return;
  }
  // Subscribe to a topic
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);
 
  Serial.println("AWS IoT Connected!");
}

// ---------------------------------------------------------------------------------------------
 
// Function to publish the BMP sensor values on the ESP8266/publish topic
 
void publishMessage()
{
  StaticJsonDocument<200> doc;
  doc["time"] = millis();
  doc["temperature"] = temperature_C;
  doc["pressure"] = pressure;
  doc["altitude"] = altitude;
  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer); // print to client
 
  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
}

// ---------------------------------------------------------------------------------------------

 
// Arduino Setup Function
 
void setup()
{
  Serial.begin(115200);
  if (!bmp.begin(0x76)) {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring or "
                      "try a different address!"));
    while (1) delay(10);
  }

  // Default settings from datasheet
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     
                  Adafruit_BMP280::SAMPLING_X2,     
                  Adafruit_BMP280::SAMPLING_X16,    
                  Adafruit_BMP280::FILTER_X16,      
                  Adafruit_BMP280::STANDBY_MS_500); 

  connectAWS();
}
 
// ---------------------------------------------------------------------------------------------

// Arduino Loop Function 

void loop()
{
  temperature_C = bmp.readTemperature();
  pressure = bmp.readPressure();
  altitude = bmp.readAltitude(1013.25);
  Serial.print(F("Temperature : "));
  Serial.print(temperature_C);
  Serial.print(F("%  Pressure : "));
  Serial.print(pressure);
  Serial.print(F("   Altitude : "));
  Serial.println(altitude);
  delay(2000); 
  now = time(nullptr);
  if (!client.connected())
  {
    connectAWS();
  }
  else
  {
    client.loop();
    if (millis() - lastMillis > 5000)
    {
      lastMillis = millis();
      publishMessage();
    }
  }
}

// ---------------------------------------------------------------------------------------------
