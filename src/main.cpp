#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <AsyncMqttClient.h>

// nahrani slozky data: "pio run -t uploadfs"


const char* ssid     = "ESPNet";
const char* password = "";

const char *mqtt_broker = "broker.emqx.io";
const int mqtt_port = 1883;

const int ledUp = 5;
const int ledDown = 4;

bool goUp = false;
bool goDown = false;
bool stop = false;

unsigned long time_now = 0;

AsyncWebServer server(80);
AsyncMqttClient mqttClient;

void connectToMqtt() {
  Serial.println("Connecting to MQTT...");
  mqttClient.connect();
}

void onMqttConnect(bool sessionPresent) {
  Serial.println("Connected to MQTT.");
  Serial.print("Session present: ");
  Serial.println(sessionPresent);
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("Disconnected from MQTT.");
  if (WiFi.isConnected()) {
    connectToMqtt();
  }
}

void onMqttPublish(uint16_t packetId) {
  Serial.print("Publish acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "404: Not found");
};

void setup() {
  pinMode(ledUp, OUTPUT);
  digitalWrite(ledUp, LOW);
  pinMode(ledDown, OUTPUT);
  digitalWrite(ledDown, LOW);

  SPIFFS.begin();
  Serial.begin(115200);
  delay(10);
  Serial.println('\n');

  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.setServer(mqtt_broker, mqtt_port);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to ");
  Serial.print(ssid); Serial.println(" ...");

  int i = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(++i); Serial.print(' ');
  }

  Serial.println('\n');
  Serial.println("Connected");  
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());

  Serial.println("Connecting to MQTT...");
  mqttClient.connect();


  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    stop = true;
    digitalWrite(ledUp, LOW);
    digitalWrite(ledDown, LOW);
    request->send(SPIFFS, "/index.html", String());
  });

  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/style.css","text/css");
  });
  server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/script.js","text/javascript");
  });

  server.on("/up", HTTP_GET, [](AsyncWebServerRequest *request){
    if (goUp == false) {
      time_now = millis();
    }
    goUp = true;
    goDown = false;
    stop = false;
    request->send(SPIFFS, "/index.html", String());
  });

  server.on("/down", HTTP_GET, [](AsyncWebServerRequest *request){
    if (goDown == false) {
      time_now = millis();
    }
    goDown = true;
    goUp = false;
    stop = false;
    request->send(SPIFFS, "/index.html", String());
  });
  
  server.on("/stop", HTTP_GET, [](AsyncWebServerRequest *request){
    stop = true;
    request->send(SPIFFS, "/index.html", String());
  });

  server.onNotFound(notFound);
  server.begin();
}

void loop() {
  if (goUp == true) {
    if (stop == false) {
      digitalWrite(ledDown, LOW);
      digitalWrite(ledUp, HIGH);
      if (millis() > time_now + 10000) {
        digitalWrite(ledUp, LOW);
        goUp = false;
      }
    }
    else {
      digitalWrite(ledUp, LOW);
    }
  }
  else if (goDown == true) {
    if (stop == false) {
      digitalWrite(ledUp, LOW);
      digitalWrite(ledDown, HIGH);
      if (millis() > time_now + 10000) {
        digitalWrite(ledDown, LOW);
        goDown = false;
      }
    }
    else {
      digitalWrite(ledDown, LOW);
    }
  }
  
}