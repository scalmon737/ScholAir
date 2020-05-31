/*

Code to read CO2, temperature and humidity, using MQ and DHT-11 sensors controlled by 
an ESP32 micro-controller 

*/

#include "ThingSpeak.h"
#include "secrets.h"
#include <WiFi.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Wi-Fi defitions
char ssid[] = SECRET_SSID;   // network SSID
char pass[] = SECRET_PASS;   // network password
int keyIndex = 0;            // network key Index number (needed only for WEP)
WiFiClient  client;

// Thing Speak definitions
unsigned long myChannelNumber = SECRET_CH_ID;
const char * myWriteAPIKey = SECRET_WRITE_APIKEY;

// Display OLED
Adafruit_SSD1306 display(128, 64, &Wire, 4);

// DHT11 definitions
#define DHTPIN 13
#define DHTTYPE DHT11 // define o tipo de sensor, no caso DHT11
DHT dht(DHTPIN, DHTTYPE);
float humidity;
float temperature;

// CO2 definitions
#define MQ_analog 2  // MQ analog PIN
#define MQ_dig 7     // MQ digital PIN
long co2;

// LED pin
const int LED = 12;

void setup() {
  Serial.begin(115200);  // Monitor serial initialization
  Serial.println("Beginning...");
  pinMode(LED, OUTPUT);
  WiFi.mode(WIFI_STA);   // Set Wi-Fi mode as station
  ThingSpeak.begin(client);  // ThingSpeak  initialization
  dht.begin();  // DHT-11  initialization
  pinMode(MQ_analog, INPUT);
  pinMode(MQ_dig, INPUT);
  // Display initialization and configuration
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setTextWrap(false);
}

void loop() {
  // Connect or reconnect to WiFi
  if(WiFi.status() != WL_CONNECTED){
    Serial.print("Trying to connect: ");
    Serial.println(SECRET_SSID);
    while(WiFi.status() != WL_CONNECTED){
      WiFi.begin(ssid, pass); // Connect to WPA/WPA2 network. Change this line if using open or WEP network
      Serial.print(".");
      delay(5000);     
    } 
    Serial.println("\nConected.");
  }
  // CO2 reading
  co2 = analogRead(MQ_analog); 
  
  // Humidity and temperature reading
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
  // test reading values
  if (isnan(temperature) || isnan(humidity)) 
  {
    Serial.println("Failure to read DHT");
  }
  else
  {
    digitalWrite(LED, HIGH);
    Serial.print("Temperature: ");
    Serial.print(temperature, 1);
    Serial.print("*C ");
    Serial.print("Humidity: ");
    Serial.print(humidity, 1);
    Serial.print("% ");
    Serial.print("CO2: ");
    Serial.print(co2);
    Serial.println("ppm");
    displayScroll();
  }
  
  // Send data to ThingSpeak: Temperature field 1, Humidity field 2, CO2 field 3

  ThingSpeak.setField(1, temperature);
  ThingSpeak.setField(2, humidity);
  ThingSpeak.setField(3, co2);
  
  int httpResponseCode = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if(httpResponseCode == 200){
    Serial.println("Channel uptaded.");
  }
  else{
    Serial.println("Failed to update channel. HTTP error code " + String(httpResponseCode));
  }
  
  delay(10000); // Wait 10 seconds 
  digitalWrite(LED, LOW); // Turn LED off
  delay(1000); // Wait 1 second
}

void displayScroll() 
{
  display.clearDisplay(); 
  display.setCursor(0,7);
  display.print("ScholAir");
  display.setCursor(1,20);
  display.setTextSize(1);
  display.print("Temp:"+String(temperature,0)+"C");
  display.setCursor(2,30);
  display.print("Humid:"+String(humidity,0)+"%");
  display.setCursor(3,40);
  display.print("CO2:"+String(co2)+"ppm");
  display.display(); // Escreve as informações da lista de escrita no display
}
