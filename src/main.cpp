#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <string.h>
#include <stdlib.h>
#include <ArduinoHA.h>
#include <LittleFS.h>

#include <vars.hpp>
#include <config.hpp>

// nahrani slozky data: "pio run -t uploadfs"

AsyncWebServer server(80);
WiFiClient client;

HADevice device(mac, sizeof(mac));
HAMqtt mqtt(client, device);

HAButton buttonUp("buttonUp");
HAButton buttonDown("buttonDown");
HAButton buttonStop("buttonStop");
HAButton buttonTiltUp("buttonTiltUp");
HAButton buttonTiltDown("buttonTiltDown");

void connectWifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to ");
  Serial.print(ssid);
  Serial.println(" ...");

  int i = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(++i);
    Serial.print(' ');
  }

  Serial.print("\n");
  Serial.println("Connected");  
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());
}

void reconnectHA() {
  while (!mqtt.isConnected()) {
    Serial.print("Connecting to MQTT...");
    if (mqtt.begin(mqtt_broker, mqtt_username, mqtt_password)) {
      Serial.println("Connected to MQTT");
    } else {
      Serial.print("Failed to connect to the HA");
      Serial.println(" trying again in 5 seconds");
      delay(5000);
    }
  }
}

void connected() {
  Serial.println("Connected to MQTT");
  mqtt.subscribe("commands");
}

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "404: Not found");
};

void stopMovement(const char* topic) {
  goUp = false;
  goDown = false;
  stop = true;
  tiltDirection = "stop";
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
      tiltFully = "up";
      mqtt.publish(movement_topic, topic, true);
    }
    else if (topic == "down") {
      goDown = direction;
      goUp = otherDirection;
      digitalWrite(ledUp, LOW);
      digitalWrite(ledDown, HIGH);
      tiltFully = "down";
      mqtt.publish(movement_topic, topic, true);
    }
  }
}

void startTilting(String direction) {
  tiltDirection = direction;
  tiltTime = millis() + 400;
  pauseTime = tiltTime + 800;
  if (direction == "up") {
    digitalWrite(ledUp, HIGH);
  }
  else if (direction == "down") {
    digitalWrite(ledDown, HIGH);
  }
}

void tiltMovement(String tilt) {
  if (tilt != "stop" && stop) {
    if (tiltTime < millis() && pauseTime > millis()) {
      tiltTime = pauseTime + 400;
      if (tilt == "up") {
        digitalWrite(ledUp, LOW);
        tilt_position += 10;
        tilt_message = itoa(tilt_position, buffer, 10);
      }
      else if (tilt == "down") {
        digitalWrite(ledDown, LOW);
        tilt_position -= 10;
        tilt_message = itoa(tilt_position, buffer, 10);
      }
      if (tilt_position >= 100) {
        tiltDirection = "stop";
        tilt_position = 100;
      }
      else if (tilt_position <= 0) {
        tiltDirection = "stop";
        tilt_position = 0;
      }
      mqtt.publish(tilt_topic, tilt_message, true);
    }
    else if (pauseTime < millis()) {
      pauseTime = tiltTime + 800;
      if (tilt == "up") {
        digitalWrite(ledUp, HIGH);
      }
      else if (tilt == "down") {
        digitalWrite(ledDown, HIGH);
      }
    }
  }
}

void fullTilt() {
  if (tiltFully == "up" && current_position >= 100) {
    tiltFully = "no";
  }
  else if (tiltFully == "down" && current_position <= 0) {
    tiltFully = "no";
  }
  if (tiltFully == "up") {
    if (!stop && tilt_position < 100) {
      stop = true;
      fullTiltTime = millis();
      mqtt.publish(movement_topic, "Tilting up...", true);
    }
  }
  else if (tiltFully == "down") {
    if (!stop && tilt_position > 0) {
      stop = true;
      fullTiltTime = millis();
      mqtt.publish(movement_topic, "Tilting down...", true);
    }
  }
  if (stop) {
    if (tiltFully == "up" && fullTiltTime + 400 < millis()) {
      tilt_position += 10;
      if (tilt_position > 100) {
        tilt_position = 100;
      }
      fullTiltTime = millis();
    }
    else if (tiltFully == "down" && fullTiltTime + 400 < millis()) {
      tilt_position -= 10;
      if (tilt_position < 0) {
        tilt_position = 0;
      }
      fullTiltTime = millis();
    }
    if (tilt_position >= 100 && tiltFully == "up" || tilt_position <= 0 && tiltFully == "down") {
      stop = false;
      tiltFully = "no";
      mqtt.publish(movement_topic, "Done", true);
    }
  }
}

void state() {
  if (!stop) {
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
        state_message = itoa(current_position, buffer, 10);
        mqtt.publish(state_topic, state_message, true);
      }
    } else {
      if (millis() > state_time + 10000) {
        state_time = millis();
        state_message = itoa(current_position, buffer, 10);
        mqtt.publish(state_topic, state_message, true);
      }
    }
  }
}

void onMessage(const char* topic, const uint8_t* payload, uint16_t length) {
  String message;
  String topic_str;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  for (unsigned int i = 0; i < strlen(topic); i++) {
    topic_str += topic[i];
  }
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
    else if (message == "tilt up") {
      if (tiltDirection == "up") {
        tiltDirection = "stop";
        digitalWrite(ledUp,LOW);
      } else {
        startTilting("up");
      }
    }
    else if (message == "tilt down") {
      if (tiltDirection == "down") {
        tiltDirection = "stop";
        digitalWrite(ledDown,LOW);
      } else {
        startTilting("down");
      }
    }
  }
}

void onButtonCommand(HAButton* sender) {
  if (sender == &buttonUp) {
    movement(goUp, goDown, "up");
  }
  else if (sender == &buttonDown) {
    movement(goDown, goUp, "down");
  }
  else if (sender == &buttonStop) {
    stopMovement("stop");
  }
  else if (sender == &buttonTiltUp) {
    if (tiltDirection == "up") {
      tiltDirection = "stop";
      digitalWrite(ledUp,LOW);
    } else {
      startTilting("up");
    }
  }
  else if (sender == &buttonTiltDown) {
    if (tiltDirection == "down") {
      tiltDirection = "stop";
      digitalWrite(ledDown, LOW);
    } else {
      startTilting("down");
    }
  }
}

void setDevice() {
  device.setName("Blinds");
  device.setSoftwareVersion("1.0.0");

  buttonUp.setName("Blinds Up");
  buttonUp.setIcon("mdi:chevron-double-up");
  buttonDown.setName("Blinds Down");
  buttonDown.setIcon("mdi:chevron-double-down");
  buttonStop.setName("Blinds Stop");
  buttonStop.setIcon("mdi:pause");
  buttonTiltUp.setName("Tilt Up");
  buttonTiltUp.setIcon("mdi:chevron-up");
  buttonTiltDown.setName("Tilt Down");
  buttonTiltDown.setIcon("mdi:chevron-down");

  buttonUp.onCommand(onButtonCommand);
  buttonDown.onCommand(onButtonCommand);
  buttonStop.onCommand(onButtonCommand);
  buttonTiltUp.onCommand(onButtonCommand);
  buttonTiltDown.onCommand(onButtonCommand);
}

void setup() {
  pinMode(ledUp, OUTPUT);
  digitalWrite(ledUp, LOW);
  pinMode(ledDown, OUTPUT);
  digitalWrite(ledDown, LOW);

  LittleFS.begin();
  Serial.begin(115200);
  delay(10);
  Serial.println('\n');

  connectWifi();

  setDevice();
  
  mqtt.onMessage(onMessage);
  mqtt.onConnected(connected);
  mqtt.begin(mqtt_broker, mqtt_username, mqtt_password);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    stop = true;
    digitalWrite(ledUp, LOW);
    digitalWrite(ledDown, LOW);
    request->send(LittleFS, "/index.html", String());
  });

  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/style.css","text/css");
  });
  server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/script.js","text/javascript");
  });

  server.on("/up", HTTP_GET, [](AsyncWebServerRequest *request){
    movement(goUp, goDown, "up");
    request->send(LittleFS, "/index.html", String());
  });

  server.on("/down", HTTP_GET, [](AsyncWebServerRequest *request){
    movement(goDown, goUp, "down");
    request->send(LittleFS, "/index.html", String());
  });
  
  server.on("/stop", HTTP_GET, [](AsyncWebServerRequest *request){
    stopMovement("stop");
    request->send(LittleFS, "/index.html", String());
  });

  server.onNotFound(notFound);
  server.begin();
}

void loop() {
  mqtt.loop();
  fullTilt();
  state();
  tiltMovement(tiltDirection);
}