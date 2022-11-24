#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <PubSubClient.h>
#include <string.h>

// nahrani slozky data: "pio run -t uploadfs"


const char* ssid     = "ESPNet";
const char* password = "";

const char* mqtt_broker = "#.#.#.#";
const int mqtt_port = 1883;
const char* mqtt_username = "homeassistant";
const char* mqtt_password = "";

const char* movement_topic = "movement";

const int ledUp = 5;
const int ledDown = 4;

bool goUp = false;
bool goDown = false;
bool stop = false;

unsigned long time_now = 0;

AsyncWebServer server(80);
WiFiClient espClient;
PubSubClient client(espClient);

void reconnect() {
  while (!client.connected()) {
    Serial.print("Connecting MQTT...");
    if (client.connect("core-mosquitto", mqtt_username, mqtt_password)) {
      Serial.println("Connected");
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" trying again in 5 seconds");
      delay(5000);
    }
  }
}

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "404: Not found");
};

void stopMovement(const char* topic) {
  goUp = false;
  goDown = false;
  stop = true;
  digitalWrite(ledUp, LOW);
  digitalWrite(ledDown, LOW);
  client.publish(movement_topic, topic, true);
}

void movement(bool direction, bool otherDirection, const char* topic) {
  if (direction == true) {
    stopMovement(topic);
  } else {
    time_now = millis();
    direction = true;
    otherDirection = false;
    stop = false;
    if (topic == "up") {
      goUp = direction;
      goDown = otherDirection;
      digitalWrite(ledDown, LOW);
      digitalWrite(ledUp, HIGH);
      client.publish(movement_topic, topic, true);
    }
    else if (topic == "down") {
      goDown = direction;
      goUp = otherDirection;
      digitalWrite(ledUp, LOW);
      digitalWrite(ledDown, HIGH);
      client.publish(movement_topic, topic, true);
    }
  }
}

void movementFinished() {
  if (goUp == true) {
    if (millis() > time_now + 10000) {
      stopMovement("top");
    }
  }
  else if (goDown == true) {
    if (millis() > time_now + 10000) {
      stopMovement("bottom");
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  String message;
  String topic_str;
  Serial.println();
  Serial.print("Message received in topic: ");
  Serial.print(topic);
  Serial.print("     message:");
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  for (int i = 0; i < strlen(topic); i++) {
    topic_str += topic[i];
  }
  Serial.print(message);
  Serial.println();
  Serial.println("-----------------------");
  Serial.println();

  if (topic_str == "commands") {
    if (message == "up") {
      movement(goUp, goDown, "up");
    }
    else if (message == "down") {
      movement(goDown, goUp, "down");
    }
    else if (message == "stop") {
      stopMovement("stop");
    }
  }
}

void setup() {
  pinMode(ledUp, OUTPUT);
  digitalWrite(ledUp, LOW);
  pinMode(ledDown, OUTPUT);
  digitalWrite(ledDown, LOW);

  SPIFFS.begin();
  Serial.begin(115200);
  delay(10);
  Serial.println('\n');

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

  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);
  Serial.println("Connecting to MQTT...");
  client.connect("core-mosquitto", mqtt_username, mqtt_password);
  client.subscribe("commands");
  

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
    movement(goUp, goDown, "up");
    request->send(SPIFFS, "/index.html", String());
  });

  server.on("/down", HTTP_GET, [](AsyncWebServerRequest *request){
    movement(goDown, goUp, "down");
    request->send(SPIFFS, "/index.html", String());
  });
  
  server.on("/stop", HTTP_GET, [](AsyncWebServerRequest *request){
    stopMovement("stop");
    request->send(SPIFFS, "/index.html", String());
  });

  server.onNotFound(notFound);
  server.begin();
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  movementFinished();
}