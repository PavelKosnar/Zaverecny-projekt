# Dálkové ovládání domácnosti
- ovladač
- vlastní model
- aplikace (pokud zbyde čas)

# Součástky a služby
- esp8266 / 4 relay esp8266
- mqtt server
- home assistent
- platformio

V mém projektu bych se chtěl věnovat dálkovému ovládání domácnosti pomocí esp8266.\
Projekt si představuji tak, že bych měl malý model žaluzií a k tomu bych sestrojil ovládání.\
Kód si představuji psát v jazyce c++.


# TO DO
- ~~koupit součástky (esp8266, relé...)~~
- ~~napsat kód na rozsvícení ledek (simulace motoru)~~
- ~~připojit k home assistantu (MQTT)~~
- naprogramovat:
  - žaluzie dolů
  - žaluzie nahoru
  - naklopení žaluzií
- všechno sestavit a zprovoznit

# Problémy
### ESP se mi nedaří připojit k wi-fi pomocí ESPHome
- Fix:
  - Připojoval jsem se k 5G WiFi, kterou esp nepodporuje
  - Stačilo se připojit k normální 2.4GHZ WiFi
### Nefunguje build
- Fix:
  - Smazat složku ./pio

## Zdroje
1. MicroPython esp8266 tutorial:
   - https://docs.micropython.org/en/latest/esp8266/tutorial
2. Instalace, zprovoznění a add-ons home assistanta
   - https://www.youtube.com/watch?v=sVqyDtEjudk&ab_channel=TheHookUp
3. Připojení esp8266 k home assistantovi pomocí MQTT a ESPHome
   - https://youtu.be/NGCUfZhxY9U
