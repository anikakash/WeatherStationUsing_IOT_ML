#include <LiquidCrystal_I2C.h>
#include <Wire.h> // Include the Wire library for I2C
#include <ESP8266WiFi.h>
#include "ThingSpeak.h"
#include <DHT.h>
#include <Adafruit_Sensor.h>

String apiKey = "QPQZNP1W5HA081WQ"; // Write your "Write API key"
const char* ssid = "iPhone"; // Write your "wifi name"
const char* password = "sanjida00"; // Write your "wifi password"
const char* server = "api.thingspeak.com";
WiFiClient client;

float h, t, soilMoistureValue;

// DHT11 sensor setup
#define DHTPIN D4    // Define the pin connected to the DHT11 data pin
#define DHTTYPE DHT11 // Define the type of DHT sensor

DHT dht(DHTPIN, DHTTYPE);

// I2C LCD setup
LiquidCrystal_I2C lcd(0x27, 16, 2); // Set the LCD I2C address (usually 0x27 or 0x3F for 16x2)

void setup() {
  Serial.begin(9600);
  Serial.println("Serial Begin");

  // Initialize the DHT11 sensor
  dht.begin();
  
  // Initialize the I2C display
  lcd.init();
  lcd.backlight();
  lcd.clear();

  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
        
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WiFi Connected");
}

void loop() {
  // Wait a few seconds between measurements.
  delay(2000);

  DHT_data();
  SoilMoisture_data();
  updateDisplay();

  if (client.connect(server, 80))
  {
    fwd_to_Thingspeak();
  }
  client.stop();
  Serial.println("Waiting");

  delay(2000);
}

void DHT_data()
{
  h = dht.readHumidity();
  t = dht.readTemperature(); 
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor");
    return;
  }                 
  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.println(" °C");
}

void SoilMoisture_data()
{
  soilMoistureValue = analogRead(A0);
  Serial.print("Soil Moisture: ");
  Serial.println(soilMoistureValue);
}

void fwd_to_Thingspeak()
{
  String postStr = apiKey;
  postStr +="&field1=";
  postStr += String(h);  
  postStr +="&field2=";
  postStr += String(t); 
  postStr +="&field3=";
  postStr += String(soilMoistureValue);
  postStr += "\r\n\r\n";
          
  client.print("POST /update HTTP/1.1\n");
  client.print("Host: api.thingspeak.com\n");
  client.print("Connection: close\n");
  client.print("X-THINGSPEAKAPIKEY: "+apiKey+"\n");
  client.print("Content-Type: application/x-www-form-urlencoded\n");
  client.print("Content-Length: ");
  client.print(postStr.length());
  client.print("\n\n");
  client.print(postStr);
          
  Serial.print("Send data to ThingSpeak. ");
  Serial.print("Content-Length: ");
  Serial.print(postStr.length());
  Serial.print(" Field-1: ");
  Serial.print(h); 
  Serial.print(" Field-2: ");
  Serial.print(t); 
  Serial.print(" Field-3: ");
  Serial.print(soilMoistureValue);
  Serial.println(" data sent");            
}

void updateDisplay()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(t);
  lcd.print("C");
  
  lcd.setCursor(0, 1);
  lcd.print("Soil: ");
  lcd.print(soilMoistureValue);
  Serial.println("Display");
}
