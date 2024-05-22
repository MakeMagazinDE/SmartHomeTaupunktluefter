
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>

const char* SSID = "WLAN_NAME";
const char* PSK = "WLAN_PASSWORT";
const char* MQTT_BROKER_IP = "MQTT.BROKER.IP.ADRESS";

int MQTT_BROKER_PORT = 1883;
const char* DEVICE_NAME = "DEVICE_NAME";
const char* BROKER_USER = "BROKER_USER";
const char* BROKER_PASSWORD = "BROKER_PASSWORD";

int pinRelais = D3;

WiFiClient espClient;
PubSubClient client(espClient);


void callback(char* topic, byte* payload, unsigned int length) {
  
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(payload);

  int ventilation = root["ventilation"];

  if (ventilation==1) {
    digitalWrite(pinRelais, HIGH);}
  else {
    digitalWrite(pinRelais, LOW);}}
    

void reconnect() {
    while (!client.connected()) {
        if (client.connect(DEVICE_NAME, BROKER_USER, BROKER_PASSWORD)) {
            client.subscribe("/ventilation");
            delay(5000);}}}


void setup_wifi() {
    WiFi.begin(SSID, PSK);
 
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");}}


void setup(){
  pinMode(pinRelais, OUTPUT);
  digitalWrite(pinRelais, LOW);
  
  setup_wifi();
  client.setServer(MQTT_BROKER_IP, MQTT_BROKER_PORT);
  client.setCallback(callback);
  delay(2000);}


void loop(){
  if (!client.connected()) {reconnect();}
  client.loop();}
