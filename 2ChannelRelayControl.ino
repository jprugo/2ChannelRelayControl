#include <stdlib.h>
#include <ArduinoJson.h>
#include "CountdownLib.h"
#include <ESP8266WiFi.h>
//ESP Web Server Library to host a web page
#include <ESP8266WebServer.h>
#define DEBUG_ON

#define DEBUG(A) Serial.println(A);

const char *ssid = "Your_ssid";     // Set you WiFi SSID
const char *password = "your_password";

const uint8_t chargerpin = D5;
const uint8_t fanpin = D6;

const uint8_t valorEncendido = LOW;
const uint8_t valorApagado = HIGH;

const int timerValue=3600;//una hora

uint8_t charger = valorApagado;
uint8_t Fan = valorApagado;
bool CountdownActivated = false;

ESP8266WebServer server(80); //Server on port 80


void setup() {
  // put your setup code here, to run once:
  pinMode(chargerpin, OUTPUT);
  pinMode(fanpin, OUTPUT);
  digitalWrite(chargerpin, valorApagado);
  digitalWrite(fanpin, valorApagado);
  Serial.begin(115200);
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid, password);
  Serial.print("Conectando");
  while (WiFi.status() != WL_CONNECTED)
  {
    // not connected to the network
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.println("Conectado (Y)");
  Serial.println("Direccion IP: ");
  Serial.println(WiFi.localIP());
  server.on("/", handle_OnConnect);
  server.on("/chargerOn", handle_ChargerON);
  server.on("/chargerOff", handle_ChargerOFF);
  server.on("/fanOn", handle_FanON);
  server.on("/fanOff", handle_FanOFF);
  server.onNotFound(handle_NotFound);
  server.begin();
  Serial.println("Servidor HTTP listo");
}

Countdown countDown(timerValue, []() {
  DEBUG("Tumer Finished");
  countDown.Reset();
  CountdownActivated = false;
  handle_FanOFF();
});

void loop() {
  server.handleClient();
  if (CountdownActivated == true) {
    countDown.Tick();
    DEBUG(countDown.Value);
  }
  delay(1000);
}

void handle_OnConnect()
{
  server.send(200, "application/json", SendHTML());
}
void handle_FanON()
{
  String seconds=server.arg("seconds");
  Serial.println("query param: "+seconds);
  countDown.setStart(seconds.toInt());
  //Reset soluciona
  countDown.Reset();
  CountdownActivated = true;
  if (Fan != valorEncendido) {
    Fan = valorEncendido;
  }
  digitalWrite(fanpin, Fan);
  server.send(200, "application/json", SendHTML());
}
void handle_FanOFF()
{
  CountdownActivated = false;
  countDown.Reset();
  if (Fan != valorApagado) {
    Fan = valorApagado;
  }
  digitalWrite(fanpin, Fan);
  server.send(200, "application/json", SendHTML());
}
void handle_ChargerON()
{
  if (charger != valorEncendido) {
    charger = valorEncendido;
  }
  digitalWrite(chargerpin, charger);
  server.send(200, "application/json", SendHTML());
}
void handle_ChargerOFF()
{
  if (charger != valorApagado) {
    charger = valorApagado;
  }
  digitalWrite(chargerpin, charger);
  server.send(200, "application/json", SendHTML());
}
void handle_NotFound()
{
  server.send(404, "text/plain", "Not found");
}

String SendHTML()
{
  String strc;
  if (charger == valorEncendido) {
    strc = "ON";
  } else {
    strc = "OFF";
  }
  String strf;
  if (Fan == valorEncendido) {
    strf = "ON";
  } else {
    strf = "OFF";
  }
  String output;
  const int capacity = JSON_ARRAY_SIZE(3) + 3 * JSON_OBJECT_SIZE(3);
  StaticJsonDocument<capacity> doc;
  doc["chargerstatus"] = strc; doc["fanstatus"] = strf; doc["timer"]=countDown.Value;
  serializeJson(doc, output);
  return output;
}
