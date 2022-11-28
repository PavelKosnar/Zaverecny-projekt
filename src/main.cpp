#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <PubSubClient.h>
#include <string.h>
#include <stdlib.h>

#include <vars.hpp>

// nahrani slozky data: "pio run -t uploadfs"

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
    start_time = millis();
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

void state() {
  if (goUp == true) {
    if (millis() > start_time + 1000) {
      current_position += 100 / path_length;
      if (current_position > 100) {
        current_position = 100;
      }
      if (current_position == 100) {
        stopMovement("top");
      } else {
        start_time = millis();
      }
    }
  }
  else if (goDown == true) {
    if (millis() > start_time + 1000) {
      current_position -= 100 / path_length;
      if (current_position < 0) {
        current_position = 0;
      }
      if (current_position == 0) {
        stopMovement("bottom");
      } else {
        start_time = millis();
      }
    }
  }
  if (goUp == true || goDown == true) {
    if (millis() > state_time + 1000) {
      state_time = millis();
      state_message = (itoa(current_position, buffer, 10));
      client.publish(state_topic, state_message, true);
    }
  } else {
    if (millis() > state_time + 10000) {
      state_time = millis();
      state_message = (itoa(current_position, buffer, 10));
      client.publish(state_topic, state_message, true);
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
  state();
}