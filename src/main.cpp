#include <ESP8266WiFi.h>
#define PIN 5

const char* ssid     = "KM";
const char* password = "Pavel9317";

void setup() {
  pinMode(PIN, OUTPUT);
  Serial.begin(115200);
  delay(10);
  Serial.println('\n');
  
  WiFi.begin(ssid, password);
  Serial.print("Pripojovani k siti ");
  Serial.print(ssid); Serial.println(" ...");

  int i = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(++i); Serial.print(' ');
  }

  Serial.println('\n');
  Serial.println("Pripojeno");  
  Serial.print("IP adresa:\t");
  Serial.println(WiFi.localIP());
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
      digitalWrite(PIN, HIGH);
  }
  else if (WiFi.status() != WL_CONNECTED) {
      digitalWrite(PIN, LOW);
  }
}