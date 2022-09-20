// Než jsem s projektem začal, ověřil jsem si funkčnost jak HW, tak arduina IDE
// K esp8266 jsem přes resistor zapojil LEDku do pinu D1 (GCIO 5)
// esp jsem zapojil k notebooku, ve kterém jsem měl připraven kód (viz. níže)

#include <Arduino.h>
#define PIN 5

void setup() {
  pinMode(PIN, OUTPUT);
}

void loop() {
  digitalWrite(PIN, HIGH);
  delay(5000);
  digitalWrite(PIN, LOW);
  delay(5000);
}

// Všechno fungovalo bez problému a mohl jsem pokračovat