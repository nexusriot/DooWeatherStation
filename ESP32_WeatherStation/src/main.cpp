#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include "AsyncJson.h"
#include "ArduinoJson.h"

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME280 bme;
AsyncWebServer server(80);

const float mmhg_in_hpa = 0.75006375541921;
const char *ssid = "NetworkName";
const char *password = "NetworkPassword";

void notFound(AsyncWebServerRequest *request)
{
  request->send(404, "application/json", "{\"message\":\"not found\"}");
}

int getSensorData(float& temperature, float& humidity, float& pressure, float& altitude)
{
  temperature = bme.readTemperature();
  humidity = bme.readHumidity();
  pressure = bme.readPressure() / 100.0F;
  altitude =  bme.readAltitude(SEALEVELPRESSURE_HPA);
  return 0;
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
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Serial.printf("WiFi not connected\n");
    delay(500);
  }
  Serial.print("IP Addr: ");
  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    float temperature, humidity, pressure, altitude;
    getSensorData(temperature, humidity, pressure, altitude);
    
    request->send(200, "application/json", createResponse(temperature, humidity, pressure, altitude));
      });

  server.begin();
}

void loop() {
}
