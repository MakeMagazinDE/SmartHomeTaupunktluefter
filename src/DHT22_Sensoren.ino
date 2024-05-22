
#include <DHT.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <math.h>

const char* SSID = "WLAN_NAME";
const char* PSK = "WLAN_PASSWORT";
const char* MQTT_BROKER_IP = "MQTT.BROKER.IP.ADRESS";

int MQTT_BROKER_PORT = 1883;
const char* DEVICE_NAME = "DEVICE_NAME";
const char* BROKER_USER = "BROKER_USER";
const char* BROKER_PASSWORD = "BROKER_PASSWORD";

float hum_correction = 0.247;
float temp_correction = -0.136;

#define DHTPIN_INSIDE D1
#define DHTPIN_OUTSIDE D2

#define DHTTYPE DHT22

DHT dht_inside(DHTPIN_INSIDE, DHTTYPE);
DHT dht_outside(DHTPIN_OUTSIDE, DHTTYPE);

WiFiClient espClient;
PubSubClient client(espClient);

long lastMsg = 0;
char msg[50];
int value = 0;


void setup(){
  setup_wifi();
  client.setServer(MQTT_BROKER_IP, MQTT_BROKER_PORT);
  
  dht_inside.begin();
  dht_outside.begin();}


void setup_wifi() {
    WiFi.begin(SSID, PSK);
 
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");}}


void reconnect() {
    while (!client.connected()) {
        if (client.connect(DEVICE_NAME, BROKER_USER, BROKER_PASSWORD)) {
            delay(5000);}}}


float absoluteHumidity(float r, float T) {
  double a, b;

  if (T >= 0) {a = 7.5; b = 237.3;}
  else {a = 7.6; b = 240.7;}

  double TK = T + 273.15;
  double EXP = (a*T) / (b+T);
  double SDD = 6.1078 * pow(10, EXP);
  double DD = r / 100 * SDD;
  double AF = 100000 * 18.016 / 8314.3 * DD / TK;

  return AF;}


void loop(){

  int n = 5;

  float in_humidity = 0.0;
  float out_humidity = 0.0;
  
  float in_temperature = 0.0;
  float out_temperature = 0.0;

  delay(500);

  for (int i=0; i<n; i++){

    in_humidity += dht_inside.readHumidity();
    out_humidity += dht_outside.readHumidity();
    
    in_temperature += dht_inside.readTemperature();
    out_temperature += dht_outside.readTemperature();

    delay(200);}

  in_humidity = in_humidity/n + hum_correction;
  out_humidity = out_humidity/n - hum_correction;
  in_temperature = in_temperature/n + temp_correction;
  out_temperature = out_temperature/n - temp_correction;

  if (isnan(in_humidity)) {in_humidity = 0.0;}
  if (isnan(out_humidity)) {out_humidity = 0.0;}
  if (isnan(in_temperature)) {in_temperature = 0.0;}
  if (isnan(out_temperature)) {out_temperature = 0.0;}

  StaticJsonBuffer<300> JSONbuffer;
  JsonObject& JSONencoder = JSONbuffer.createObject();
  JSONencoder["in_hum"] = in_humidity;
  JSONencoder["out_hum"] = out_humidity;
  JSONencoder["in_temp"] = in_temperature;
  JSONencoder["out_temp"] = out_temperature;
  JSONencoder["in_water"] = absoluteHumidity(in_humidity, in_temperature);
  JSONencoder["out_water"] = absoluteHumidity(out_humidity, out_temperature);

  if (!client.connected()) {reconnect();}
  client.loop();

  char JSONmessageBuffer[120];
  JSONencoder.printTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
 
  client.publish("/inside_data", JSONmessageBuffer);
  
  delay(30000);
  ESP.restart();}
