# [Návod](https://github.com/PavelKosnar/Zaverecny-projekt/blob/main/INSTRUCTIONS.md)
# Dálkové ovládání žaluzií
- ovladač
- vlastní model
- propojení s Home Assistantem

# Součástky a služby
- ESP8266
- Motor
- Žaluzie
- 2x relé
- MQTT server
- Home Assistant
- Platformio
- Async webserver

V mém projektu bych se chtěl věnovat dálkovému ovládání domácnosti pomocí esp8266.\
Projekt si představuji tak, že bych měl malý model žaluzií a k tomu bych sestrojil ovládání.\
Kód si představuji psát v jazyce c++.


# TO DO
- ~~koupit součástky (esp8266, relé...)~~
- ~~napsat kód na rozsvícení ledek (simulace motoru)~~
- ~~připojit k home assistantu (MQTT)~~
- naprogramovat:
  - ~~žaluzie dolů~~
  - ~~žaluzie nahoru~~
  - ~~naklopení žaluzií~~
- všechno sestavit a zprovoznit

# Problémy
### ESP se mi nedaří připojit k Wi-Fi
- Fix:
  - Připojoval jsem se k 5G WiFi, kterou esp nepodporuje
  - Stačilo se připojit k normální 2.4GHZ WiFi
### Nefunguje build
- Fix:
  - Smazat složku ./pio
### ArduinoHA se nechce připojit k HA
- Fix:
  - Snažil jsem se k MQTT připojit přes 2 různé knihovny
  - Musel jsem se tedy připojit pouze přes 1 knihovnu

## Zdroje
- [Instalace, zprovoznění a add-ons Home Assistanta](https://www.youtube.com/watch?v=sVqyDtEjudk&ab_channel=TheHookUp)
- [ArduinoHA knihovna a její použití](https://dawidchyrzynski.github.io/arduino-home-assistant/index.html)
- [Home Assistant](https://github.com/codingPear/home-assistant-docker-compose)
