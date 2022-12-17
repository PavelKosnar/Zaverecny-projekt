#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <PubSubClient.h>
#include <string.h>
#include <stdlib.h>
#include <ArduinoHA.h>
#include <JC_Button.h>

#include <vars.hpp>

#define BROKER_ADDR IPAddress(#,#,#,#)

// nahrani slozky data: "pio run -t uploadfs"

AsyncWebServer server(80);
WiFiClient client;

HADevice device(mac, sizeof(mac));
HAMqtt mqtt(client, device);
/*
HADeviceTrigger shortPressTriggerUp(HADeviceTrigger::ButtonShortPressType, btnUpName);
HADeviceTrigger longPressTriggerUp(HADeviceTrigger::ButtonLongPressType, btnUpName);
HADeviceTrigger shortPressTriggerDown(HADeviceTrigger::ButtonShortPressType, btnDownName);
HADeviceTrigger longPressTriggerDown(HADeviceTrigger::ButtonLongPressType, btnDownName);
Button btnUP(UP_PIN), btnDOWN(DOWN_PIN);*/

HASwitch switch1("switchUp");
HASwitch switch2("switchDown");
HASwitch switch3("switchStop");

void reconnectHA() {
  while (!mqtt.isConnected()) {
    Serial.print("Connecting to MQTT...");
    if (mqtt.begin(BROKER_ADDR, mqtt_username, mqtt_password)) {
      Serial.println("Connected");
    } else {
      Serial.print("Failed to connect to the HA");
      Serial.println(" trying again in 5 seconds");
      delay(5000);
    }
  }
}

void connected() {
  Serial.println("Connected");
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
  mqtt.publish(movement_topic, topic, true);
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
      mqtt.publish(movement_topic, topic, true);
    }
    else if (topic == "down") {
      goDown = direction;
      goUp = otherDirection;
      digitalWrite(ledUp, LOW);
      digitalWrite(ledDown, HIGH);
      mqtt.publish(movement_topic, topic, true);
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
      mqtt.publish(state_topic, state_message, true);
    }
  } else {
    if (millis() > state_time + 10000) {
      state_time = millis();
      state_message = (itoa(current_position, buffer, 10));
      mqtt.publish(state_topic, state_message, true);
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

void onSwitchCommand(bool state, HASwitch* sender) {
    if (sender == &switch1) {
      movement(goUp, goDown, "up");
    } else if (sender == &switch2) {
      movement(goDown, goUp, "down");
    } else if (sender == &switch3) {
      stopMovement("stop");
    }
    sender->setState(state);
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
    Serial.print(++i);
    Serial.print(' ');
  }

  Serial.println('\n');
  Serial.println("Connected");  
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());

  device.setName("Blinds");
  device.setSoftwareVersion("1.0.0");

  switch1.setName("Blinds Up");
  switch1.setIcon("mdi:chevron-double-up");
  switch1.onCommand(onSwitchCommand);
  switch2.setName("Blinds Down");
  switch2.setIcon("mdi:chevron-double-down");
  switch2.onCommand(onSwitchCommand);
  switch3.setName("Blinds Stop");
  switch3.setIcon("mdi:pause");
  switch3.onCommand(onSwitchCommand);
  
  mqtt.onConnected(connected);
  mqtt.begin(BROKER_ADDR, mqtt_username, mqtt_password);
  mqtt.subscribe("commands");

  /*btnUP.begin();
  btnDOWN.begin();*/

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
  mqtt.loop();
  /*btnUP.read();
  btnDOWN.read();
  if (btnUP.pressedFor(3000) && !holdingBtn) {
    longPressTriggerUp.trigger();
    holdingBtn = true;
  } else if (btnUP.wasReleased()) {
    if (holdingBtn) {
      holdingBtn = false;
    } else {
      shortPressTriggerUp.trigger();
    }
  }*/
  state();
}