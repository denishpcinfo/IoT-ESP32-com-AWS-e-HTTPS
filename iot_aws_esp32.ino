#include "secrets.h"
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "WiFi.h"
#include <ESP32_Servo.h> 
#include "DHT.h"
#define DHTPIN 14        // Pino digital conectado ao sensor DHT
#define DHTTYPE DHT11   // DHT 11
 
#define AWS_IOT_PUBLISH_TOPIC   "/pub"
#define AWS_IOT_SUBSCRIBE_TOPIC "/sub"
 
float h;
float t;
int minUs = 500;
int maxUs = 2400;
int servo1Pin = 27;
int servo2Pin = 16;
int pos = 0; 

Servo servo1;
Servo servo2; 

DHT dht(DHTPIN, DHTTYPE);
 
WiFiClientSecure net = WiFiClientSecure();
PubSubClient client(net);
 
void connectAWS()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
 
  Serial.println("Conectando ao Wi-Fi");
 
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
 
  // Configure o WiFiClientSecure para usar as credenciais do dispositivo AWS IoT
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);
 
  // Conecte-se ao corretor MQTT no endpoint AWS que foi definido anteriormente
  client.setServer(AWS_IOT_ENDPOINT, 8883);
 
  // Crie um manipulador de mensagens
  client.setCallback(messageHandler);
 
  Serial.println("Conectando à AWS IOT");
 
  while (!client.connect(THINGNAME))
  {
    Serial.print(".");
    delay(100);
  }
 
  if (!client.connected())
  {
    Serial.println("AWS IoT está OffLine!");
    return;
  }
 
  // Subscribe para um tópico
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);
 
  Serial.println("AWS IoT Conectado!");
}
 
void publishMessage()
{
  StaticJsonDocument<200> doc;
  doc["umidade"] = h;
  doc["temperatura"] = t;
  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer); // mostrar para o cliente
 
  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
}
 
void messageHandler(char* topic, byte* payload, unsigned int length)
{
  Serial.print("entrada: ");
  Serial.println(topic);
 
  StaticJsonDocument<200> doc;
  deserializeJson(doc, payload);
  const char* message = doc["message"];
          Serial.print(message);
          Serial.print("   ");
          Serial.print(topic);

  String mystring(message);
  servo1.write(0);        
  if(mystring.equals("azul")){
    servo1.write(500);
      Serial.println("azul");
      Serial.print("   ");
  }
  servo2.write(0);
  if(mystring.equals("branco")){
    servo2.write(2400);
      Serial.println("branco");
      Serial.print("   ");
  }        
}
 
void setup()
{
  Serial.begin(115200);
  connectAWS();
  dht.begin();
  servo1.attach(servo1Pin, minUs, maxUs);
  servo2.attach(servo2Pin, minUs, maxUs);
}
 
void loop()
{
  h = dht.readHumidity();
  t = dht.readTemperature();
 
 
  if (isnan(h) || isnan(t) )  // Verifique se alguma leitura falhou e saia mais cedo (para tentar novamente).
  {
    Serial.println(F("Falhou a leitura do sensor DHT!"));
    return;
  }
 
  Serial.print(F("Humidade: "));
  Serial.print(h);
  Serial.print(F("%  Temperatura: "));
  Serial.print(t);
  Serial.println(F("°C "));
 
  publishMessage();
  client.loop();
  delay(1000);
}
