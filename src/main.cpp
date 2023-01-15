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
HAButton buttonStepUp("buttonStepUp");
HAButton buttonStepDown("buttonStepDown");

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

void connected() {
  Serial.println("Connected to MQTT");
  mqtt.subscribe("commands");
}

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "404: Not found");
};

void stopMovement(const char* reason, bool message) {   //zastaví veškeré pohyby žaluzií
  goUp = false;
  goDown = false;
  stop = true;
  tiltDirection = "stop";
  digitalWrite(pinUp, LOW);
  digitalWrite(pinDown, LOW);
  if (message) {
    mqtt.publish(movement_topic, reason, true);
  }
}

void movement(bool direction, const char* topic) {
  if (direction == true) {
    stopMovement("stop", true);   //pokud už se žaluzie pohybují daným směrem, pohyb zastaví
  } else {
    start_time = millis();
    stop = false;
    if (topic == "up") {
      goUp = true;
      goDown = false;
      digitalWrite(pinDown, LOW);   //vypne směr dolů a zapne nahoru
      digitalWrite(pinUp, HIGH);
      tiltFully = "up";
      mqtt.publish(movement_topic, topic, true);    //pošle zprávu mqtt serveru se směrem nahoru
    }
    else if (topic == "down") {
      goDown = true;
      goUp = false;
      digitalWrite(pinUp, LOW);     //vypne směr nahoru a zapne dolů
      digitalWrite(pinDown, HIGH);
      tiltFully = "down";
      mqtt.publish(movement_topic, topic, true);    //pošle zprávu mqtt serveru se směrem dolů
    }
  }
}

void startTilting(String direction) {     //nastaví potřebné proměnné k naklápění
  tiltDirection = direction;
  stop = true;
  tiltTime = millis() + 400;
  pauseTime = tiltTime + 800;
  if (direction == "up") {
    digitalWrite(pinUp, HIGH);
  }
  else if (direction == "down") {
    digitalWrite(pinDown, HIGH);
  }
}

void tiltMovement(String tilt) {
  if (tilt != "stop" && stop) {
    if (tiltTime < millis() && pauseTime > millis()) {  //zastaví naklápění po 300ms
      tiltTime = pauseTime + 300;
      if (tilt == "up") {
        digitalWrite(pinUp, LOW);
        tilt_position += 17;
      }
      else if (tilt == "down") {      //aktualizuje polohu naklopení
        digitalWrite(pinDown, LOW);
        tilt_position -= 17;
      }
      if (tilt_position >= 100) {
        tiltDirection = "stop";
        tilt_position = 100;
      }
      else if (tilt_position <= 0) {
        tiltDirection = "stop";
        tilt_position = 0;
      }
      tilt_message = itoa(tilt_position, buffer, 10);
      mqtt.publish(tilt_topic, tilt_message, true);
    }
    else if (pauseTime < millis()) {    //zapne naklápění po 600ms
      pauseTime = tiltTime + 600;
      if (tilt == "up") {
        digitalWrite(pinUp, HIGH);
      }
      else if (tilt == "down") {
        digitalWrite(pinDown, HIGH);
      }
    }
  }
}

void fullTilt() {   //rozhodne, na jakou stranu naklopit a poté akci provede
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
    if (tiltFully == "up" && fullTiltTime + 200 < millis()) {
      tilt_position += 10;
      if (tilt_position > 100) {
        tilt_position = 100;
      }
      fullTiltTime = millis();
    }
    else if (tiltFully == "down" && fullTiltTime + 200 < millis()) {
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

void startStepTilt(String direction) {    //nastavení potřebných proměnných
  stepTiltDirection = direction;
  stepTiltTime = millis() + 400;
  if (direction == "up") {                //zapne naklápění na správnou stranu
    digitalWrite(pinUp, HIGH);
  }
  else if (direction == "down") {
    digitalWrite(pinDown, HIGH);
  }
}

void stepTilt(String direction) {     //po 400ms naklápění vypne
  if (direction != "none") {
    if (stepTiltTime < millis()) {
      stepTiltDirection = "none";
      if (direction == "up") {
        tilt_position += 25;
        if (tilt_position > 100) {
          tilt_position = 100;
        }
        digitalWrite(pinUp, LOW);
      }
      if (direction == "down") {
        tilt_position -= 25;
        if (tilt_position < 0) {
          tilt_position = 0;
        }
        digitalWrite(pinDown, LOW);
      }
      tilt_message = itoa(tilt_position, buffer, 10);
      mqtt.publish(tilt_topic, tilt_message, true);
    }
  }
}

void state() {
  if (!stop) {
    if (goUp == true) {
      if (millis() > start_time + 220) {    //každých 100 milisekund aktualizuje polohu žaluzií
        current_position += 20 / path_length;
        if (current_position > 100) {
          current_position = 100;
        }
        if (current_position == 100) {      //v případě, že žaluzie dojedou 
          stopMovement("top", true);
        } else {
          start_time = millis();
        }
      }
    }
    else if (goDown == true) {
      if (millis() > start_time + 200) {      //stejná jako minulá, akorát dolů
        current_position -= 20 / path_length;
        if (current_position < 0) {
          current_position = 0;
        }
        if (current_position == 0) {
          stopMovement("bottom", true);
        } else {
          start_time = millis();
        }
      }
    }
    if (goUp == true || goDown == true) {
      if (millis() > state_time + 200) {
        state_time = millis();
        state_message = itoa(current_position, buffer, 10);
        mqtt.publish(state_topic, state_message, true);
      }   //pošle zprávu mqtt serveru s aktuální polohou
    } else {
      if (millis() > state_time + 300000) {
        state_time = millis();
        state_message = itoa(current_position, buffer, 10);
        mqtt.publish(state_topic, state_message, true);
      }   //každých 5 minut pošle zprávu mqtt serveru s aktuální polohou
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
      movement(goUp, "up");
    }
    else if (message == "down") {
      movement(goDown, "down");
    }
    else if (message == "stop") {
      stopMovement("stop", true);
    }
    else if (message == "tilt up") {
      if (tiltDirection == "up") {
        tiltDirection = "stop";
        digitalWrite(pinUp,LOW);
      } else {
        startTilting("up");
      }
    }
    else if (message == "tilt down") {
      if (tiltDirection == "down") {
        tiltDirection = "stop";
        digitalWrite(pinDown,LOW);
      } else {
        startTilting("down");
      }
    }
    else if (message == "step up") {
      stopMovement("", false);
      startStepTilt("up");
    }
    else if (message == "step down") {
      stopMovement("", false);
      startStepTilt("down");
    }
  }
}

void onButtonCommand(HAButton* sender) {
  if (sender == &buttonUp) {
    movement(goUp, "up");
  }
  else if (sender == &buttonDown) {
    movement(goDown, "down");
  }
  else if (sender == &buttonStop) {
    stopMovement("stop", true);
  }
  else if (sender == &buttonTiltUp) {
    if (tiltDirection == "up") {
      tiltDirection = "stop";
      digitalWrite(pinUp,LOW);
    } else {
      startTilting("up");
    }
  }
  else if (sender == &buttonTiltDown) {
    if (tiltDirection == "down") {
      tiltDirection = "stop";
      digitalWrite(pinDown, LOW);
    } else {
      startTilting("down");
    }
  }
  else if (sender == &buttonStepUp) {
    stopMovement("", false);
    startStepTilt("up");
  }
  else if (sender == &buttonStepDown) {
    stopMovement("", false);
    startStepTilt("down");
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
  buttonTiltUp.setIcon("mdi:transfer-up");
  buttonTiltDown.setName("Tilt Down");
  buttonTiltDown.setIcon("mdi:transfer-down");
  buttonStepUp.setName("Step Up");
  buttonStepUp.setIcon("mdi:chevron-up");
  buttonStepDown.setName("Step Down");
  buttonStepDown.setIcon("mdi:chevron-down");

  buttonUp.onCommand(onButtonCommand);
  buttonDown.onCommand(onButtonCommand);
  buttonStop.onCommand(onButtonCommand);
  buttonTiltUp.onCommand(onButtonCommand);
  buttonTiltDown.onCommand(onButtonCommand);
  buttonStepUp.onCommand(onButtonCommand);
  buttonStepDown.onCommand(onButtonCommand);
}

void setup() {
  pinMode(pinUp, OUTPUT);
  digitalWrite(pinUp, LOW);
  pinMode(pinDown, OUTPUT);
  digitalWrite(pinDown, LOW);

  LittleFS.begin();
  Serial.begin(115200);
  delay(10);
  Serial.println('\n');

  connectWifi();

  setDevice();
  
  

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    stop = true;
    digitalWrite(pinUp, LOW);
    digitalWrite(pinDown, LOW);
    request->send(LittleFS, "/index.html", String());
  });

  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/style.css","text/css");
  });
  server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/script.js","text/javascript");
  });

  server.on("/up", HTTP_GET, [](AsyncWebServerRequest *request){
    movement(goUp, "up");
    request->send(LittleFS, "/index.html", String());
  });

  server.on("/down", HTTP_GET, [](AsyncWebServerRequest *request){
    movement(goDown, "down");
    request->send(LittleFS, "/index.html", String());
  });
  
  server.on("/stop", HTTP_GET, [](AsyncWebServerRequest *request){
    stopMovement("stop", true);
    request->send(LittleFS, "/index.html", String());
  });

  server.on("/tilt-up", HTTP_GET, [](AsyncWebServerRequest *request){
    if (tiltDirection == "up") {
      tiltDirection = "stop";
      digitalWrite(pinUp,LOW);
    } else {
      startTilting("up");
    }
    request->send(LittleFS, "/index.html", String());
  });

  server.on("/tilt-down", HTTP_GET, [](AsyncWebServerRequest *request){
    if (tiltDirection == "down") {
      tiltDirection = "stop";
      digitalWrite(pinUp,LOW);
    } else {
      startTilting("down");
    }
    request->send(LittleFS, "/index.html", String());
  });

  server.onNotFound(notFound);
  server.begin();

  mqtt.onMessage(onMessage);
  mqtt.onConnected(connected);
  mqtt.begin(mqtt_broker, mqtt_username, mqtt_password);
}

void loop() {
  mqtt.loop();
  fullTilt();
  state();
  tiltMovement(tiltDirection);
  stepTilt(stepTiltDirection);
}