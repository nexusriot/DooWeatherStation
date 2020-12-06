#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include "AsyncJson.h"
#include "ArduinoJson.h"

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME280 bme;
TFT_eSPI tft = TFT_eSPI();
AsyncWebServer server(80);


const float mmhg_in_hpa = 0.75006375541921;
const char *ssid = "NetworkName";
const char *password = "NetworkPassword";
float temperature, humidity, pressure, altitude;
String localIp;


void notFound(AsyncWebServerRequest *request)
{
  request->send(404, "application/json", "{\"message\":\"not found\"}");
}

void prepareScreen() {
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 0, 4);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
}

int getSensorData(float& temperature, float& humidity, float& pressure, float& altitude)
{
  temperature = bme.readTemperature();
  humidity = bme.readHumidity();
  pressure = bme.readPressure() / 100.0F;
  altitude =  bme.readAltitude(SEALEVELPRESSURE_HPA);
  return 0;
}

void updateScreen(const String& localIp, const float& temperature, const float& humidity, const float& pressure, const float& altitude)
{
  prepareScreen();
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.print("Ip: ");
  tft.setTextColor(TFT_RED, TFT_BLACK);
  tft.println(localIp);
  tft.println();
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.print("Temp: ");
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.print(temperature);
  tft.println(" C");
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.print("Humid: ");
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.print(humidity);
  tft.println(" %");
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.print("Press: ");
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.print(pressure * mmhg_in_hpa);
  tft.println(" mmHg");
}

String createResponse(const float& temperature, const float& humidity, const float& pressure, const float& altitude)
{
    DynamicJsonDocument root(1024);
    root["temperature"] = temperature;    
    root["humidity"] = humidity;
    root["pressure_hpa"] = pressure;
    root["pressure_mmhg"] = pressure * mmhg_in_hpa;
    root["altitude"] = altitude;
    String response;
    serializeJson(root, response);
    return response;  
}

void setup() {
  Wire.begin(33, 32); //(I2C_SDA, I2C_SCL)
  Serial.begin(115200);
  delay(100);
  bme.begin(0x76, &Wire);  
  tft.init();
  tft.setRotation(5);
  tft.setTextFont(1);
  tft.setTextSize(1);
  prepareScreen();
  tft.println("Initializing...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Serial.printf("WiFi not connected\n");
    delay(500);
  }
  Serial.print("IP Addr: ");
  localIp = WiFi.localIP().toString();
  Serial.println(localIp);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
   
    request->send(200, "application/json", createResponse(temperature, humidity, pressure, altitude));
      });

  server.begin();
}

void loop() {
  getSensorData(temperature, humidity, pressure, altitude);
  updateScreen(localIp, temperature, humidity, pressure, altitude);
  delay(50000);
}
