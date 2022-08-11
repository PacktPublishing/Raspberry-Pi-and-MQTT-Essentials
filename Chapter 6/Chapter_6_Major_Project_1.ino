/*
 * Book - Raspberry Pi and MQTT
 * Major Project 1 : IoT Weather Station (Chapter 6)
 * Developer Name : Dhairya Parikh
 */

// ****************************************************

// Import the required Libraries
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_BMP280.h>
#include "SparkFunCCS811.h"
#include "DHT.h"

// ****************************************************

// ****************************************************

// Variable, Object and Constant Declarations

// Constants
#define CCS811_ADDR 0x5B

// Variables
const char* ssid = "wifi_ssid";
const char* password = "wifi_password";
const char* mqtt_server = "Pi's ip address";

// Objects
WiFiClient espClient;
PubSubClient client(espClient);
Adafruit_BMP280 bmp; // I2C
CCS811 mySensor(CCS811_ADDR);
DHT dht;

// ****************************************************

// ****************************************************

// Custom function for Wifi connection establishment
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

// ****************************************************

// ****************************************************

// Callback Function 
void callback(char* topic, byte* payload, unsigned int length) 
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

// ****************************************************

// ****************************************************

// Function to reconnect to MQTT server
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
      
      // resubscribe to the specific topic
      client.subscribe("IoTWeatherStation/temperature/celcius");
      client.subscribe("IoTWeatherStation/temperature/farenhiet");
      client.subscribe("IoTWeatherStation/humidity");
      client.subscribe("IoTWeatherStation/pressure");
      client.subscribe("IoTWeatherStation/altitude");
      client.subscribe("IoTWeatherStation/TVOC");
      client.subscribe("IoTWeatherStation/eCO2");
 client.subscribe("IoTWeatherStation/hic");

    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

// ****************************************************

float computeHeatIndex(float temperature, float percentHumidity) {
  float hi;

  temperature = 1.8*temperature+32; //convertion to *F

  hi = 0.5 * (temperature + 61.0 + ((temperature - 68.0) * 1.2) + (percentHumidity * 0.094));

  if (hi > 79) {
    hi = -42.379 +
             2.04901523 * temperature +
            10.14333127 * percentHumidity +
            -0.22475541 * temperature*percentHumidity +
            -0.00683783 * pow(temperature, 2) +
            -0.05481717 * pow(percentHumidity, 2) +
             0.00122874 * pow(temperature, 2) * percentHumidity +
             0.00085282 * temperature*pow(percentHumidity, 2) +
            -0.00000199 * pow(temperature, 2) * pow(percentHumidity, 2);

    if((percentHumidity < 13) && (temperature >= 80.0) && (temperature <= 112.0))
      hi -= ((13.0 - percentHumidity) * 0.25) * sqrt((17.0 - abs(temperature - 95.0)) * 0.05882);

    else if((percentHumidity > 85.0) && (temperature >= 80.0) && (temperature <= 87.0))
      hi += ((percentHumidity - 85.0) * 0.1) * ((87.0 - temperature) * 0.2);
  }

  hi = (hi-32)/1.8;
  return hi; //return Heat Index, in *C
}

// ****************************************************

void setup() 
{
  Serial.begin(115200);
  Wire.begin(); //Initialize I2C Hardware
  dht.setup(D4);

  if (mySensor.begin() == false)
  {
    Serial.print("CCS811 error. Please check wiring. Freezing...");
    while(1);
  }

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
                  
  setup_wifi();
  
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  client.subscribe("IoTWeatherStation/temperature/celcius");
  client.subscribe("IoTWeatherStation/temperature/farenhiet");
  client.subscribe("IoTWeatherStation/humidity");
  client.subscribe("IoTWeatherStation/pressure");
  client.subscribe("IoTWeatherStation/altitude");
  client.subscribe("IoTWeatherStation/TVOC");
  client.subscribe("IoTWeatherStation/eCO2");
 client.subscribe("IoTWeatherStation/hic");
}

// ****************************************************

//*****************************************************

void loop()
{
  // Reconnect to MQTT Broker Logic
  if (!client.connected()) {
    reconnect();   
  }

  //Variable Initialization and sensor value assignment
  float co2val;
  float tvocval;
  
  if (mySensor.dataAvailable())
  {
    mySensor.readAlgorithmResults();
    co2val = mySensor.getCO2();
    tvocval = mySensor.getTVOC();
  }

  float temperature_C = bmp.readTemperature();
  float pressureval = bmp.readPressure();
  float altitudeval = bmp.readAltitude(1013.25);

  float humidity = dht.getHumidity();
  float hi = computeHeatIndex(temperature_C, humidity);
  float temperature_F = dht.toFahrenheit(dht.getTemperature());
  delay(2000);

  static char temperatureC[7];
  static char temperatureF[7];
  static char humid[7];
  static char co2[7];
  static char tvoc[7];
  static char pressure[7];
  static char altitude[7];
  static char hic[7];

  // Convert Float values to String (in Character Array format)
  dtostrf(temperature_C, 6, 2, temperatureC);
  dtostrf(temperature_F, 6, 2, temperatureF);
  dtostrf(humidity, 6, 2, humid);
  dtostrf(co2val, 6, 2, co2);
  dtostrf(tvocval, 6, 2, tvoc);
  dtostrf(pressureval, 6, 2, pressure);
  dtostrf(altitudeval, 6, 2, altitude);
  dtostrf(hi, 6, 2, hic);  

  // Publish the sensor data on their particular MQTT topics
  client.publish("IoTWeatherStation/temperature/celcius", temperatureC);
  client.publish("IoTWeatherStation/temperature/farenhiet", temperatureF);
  client.publish("IoTWeatherStation/humidity", humid);
  client.publish("IoTWeatherStation/pressure", pressure);
  client.publish("IoTWeatherStation/altitude", altitude);
  client.publish("IoTWeatherStation/TVOC", tvoc);
  client.publish("IoTWeatherStation/eCO2", co2); 
  client.publish("IoTWeatherStation/hic", hic);

  // Printing the Sensor Values on Serial Monitor
  Serial.println("--------------------------------------------------------");
  Serial.print("Temperature: ");
  Serial.println(temperature_C);
  Serial.print("Humidity: ");
  Serial.println(humidity);
  Serial.print("Heat Index Value: ");
  Serial.println(hic);
  Serial.print("TVOC Value: ");
  Serial.println(tvocval);
  Serial.print("eCO2 Value: ");
  Serial.println(co2val);
  Serial.print("Pressure Value: ");
  Serial.println(pressureval);
  Serial.print("Altitude Value: ");
  Serial.println(altitudeval);
  Serial.println("--------------------------------------------------------");
}

// ****************************************************
