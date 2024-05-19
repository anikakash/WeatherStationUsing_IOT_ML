#include <LiquidCrystal_I2C.h>
#include <Wire.h> // Include the Wire library for I2C
#include <ESP8266WiFi.h>
#include "ThingSpeak.h"
#include <DHT.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085.h>



String apiKey = "IIHXD3ISY26EJDW1"; // Write your "Write API key"
const char* ssid = "DIU_Daffodil Smart City"; // Write your "wifi name"
const char* password = "diu123456789"; // Write your "wifi password"
const char* server = "api.thingspeak.com";
WiFiClient client;

DHT dht(D5, DHT11);  // DHT pin number and DHT sensor type
Adafruit_BMP085 bmp;

const int rainSensorPin = D4; // Pin connected to the rain sensor
const int mq135Pin = A0; // Pin connected to the MQ135 sensor

float Hum, Temp, Pressure, Altitude;
int Rain;
float CO2_ppm;

// Calibration constants (replace with your calibration values)
float a = 1.0; // Slope of the calibration equation
float b = 0.0; // Intercept of the calibration equation

// Initialize the I2C display with the address 0x27 and 16x2 characters
LiquidCrystal_I2C lcd(0x77, 16, 2);

void setup() 
{
  Serial.begin(9600);
  Serial.println("Serial Begin");

  // Initialize the I2C display
  lcd.init();
  lcd.backlight();
  lcd.clear();
  
  dht.begin();
  pinMode(rainSensorPin, INPUT); // Set rain sensor pin as input
  
  if(!bmp.begin())
  {
    Serial.println("Could not find a valid BMP085 sensor, check wiring!");
    while (1);
  }
        
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
}

void loop() 
{
  DHT_data();
  BMP180_data();
  Rain_data();
  MQ135_data();
  updateDisplay(); // Update the display
  
  delay(1000);

  if (client.connect(server, 80))
  {
    fwd_to_Thingspeak();
  }
  client.stop();
  Serial.println("Waiting");
  delay(1000);
}

void DHT_data()
{
  Hum = dht.readHumidity();
  Temp = dht.readTemperature();                  
  Serial.print("Humidity: ");
  Serial.print(Hum);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(Temp);
  Serial.println(" °C");
}

void BMP180_data()
{
  Pressure = bmp.readPressure() / 100.0F; // Convert from Pa to hPa
  Altitude = bmp.readAltitude(1013.25); // Assuming sea level pressure is 1013.25 hPa

  Serial.print("Pressure: ");
  Serial.print(Pressure);
  Serial.print(" hPa\t");
  Serial.print("Altitude: ");
  Serial.print(Altitude);
  Serial.println(" m");
}

void Rain_data()
{
  Rain = digitalRead(rainSensorPin); // Read the digital signal from the rain sensor
  Serial.print("Rain: ");
  Serial.println(Rain ? "Yes" : "No"); // Assuming rain is detected when the pin is LOW
  
  if (Rain == HIGH) {
    Serial.println("Rain detected!");
  } else {
    Serial.println("No rain detected.");
  }
}

void MQ135_data()
{
  int MQ135Value = analogRead(mq135Pin); // Read the analog signal from the MQ135 sensor
  CO2_ppm = convertToCO2(MQ135Value); // Convert to CO2 concentration in ppm
  Serial.print("CO2 Level: ");
  Serial.print(CO2_ppm);
  Serial.println(" ppm");
}

// Function to convert MQ135 sensor output to CO2 concentration
float convertToCO2(int analogValue) {
  float CO2_ppm = a * analogValue + b; // Use the calibration equation
  return CO2_ppm;
}

void fwd_to_Thingspeak()
{
  String postStr = apiKey;
  postStr +="&field1=";
  postStr += String(Hum);  // Humidity data
  postStr +="&field2=";
  postStr += String(Temp); // Temperature Data
  postStr +="&field3=";
  postStr += String(Pressure); // Pressure Data
  postStr +="&field4=";
  postStr += String(Altitude); // Altitude Data
  postStr +="&field5=";
  postStr += String(Rain ? "1" : "0"); // Rain Data (1 for rain, 0 for no rain)
  postStr +="&field6=";
  postStr += String(CO2_ppm); // CO2 Level Data
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
          
  Serial.print("Send data to channel-1 ");
  Serial.print("Content-Length: ");
  Serial.print(postStr.length());
  Serial.print("Field-1: ");
  Serial.print(Hum); 
  Serial.print("Field-2: ");
  Serial.print(Temp); 
  Serial.print("Field-3: ");
  Serial.print(Pressure); 
  Serial.print("Field-4: ");
  Serial.print(Altitude); 
  Serial.print("Field-5: ");
  Serial.print(Rain ? "Yes" : "No"); // Assuming rain is detected when the pin is LOW
  Serial.print("Field-6: ");
  Serial.print(CO2_ppm); // CO2 Level Data
  Serial.println(" data send");            
}

void updateDisplay()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("T:");
  lcd.print(Temp);
  lcd.print("C");

  // lcd.setCursor(0, 1);
  // lcd.print("H:");
  // lcd.print(Hum);
  // lcd.print(" %");

  // lcd.setCursor(0, 2);
  // lcd.print("Rain: ");
  // lcd.print(Rain ? "Yes" : "No");
    delay(1000);

  Serial.println("Display Calling");
}
