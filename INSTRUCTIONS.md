# Instructions

### 1. Download Visual Studio Code
```bash
sudo apt-get install code
```
### 2. Download my project
```bash
git clone https://github.com/pavelkosnar/zaverecny-projekt
cd zaverecny-projekt/
code .
```
### 3. Install Platformio
- Press Ctrl+Shift+X in VSCode
- Type "platformio"
- Install Platformio IDE extension
### 4. Install and run Home Assistant
```bash
git clone https://github.com/pavelkosnar/ha
sudo apt-get install docker
sudo apt-get install docker-compose
docker-compose up -d
```
### 5. Configure MQTT
- In your HomeAssistant, go to Settings/Devices & Services
- Press "ADD INTEGRATION" in the bottom right corner and choose MQTT
- Configure your MQTT like following:
  - Broker -> "eclipse-mosquitto"
  - Port -> "1883"
  - Username -> "homeassistant"
- Password leave blank and press "Submit"
### 6. Configure esp and run it
```bash
ifconfig -a
```
- Copy your ip address and go back to VSCode
- In VSCode, go to "src" folder, open the "config.hpp" file and change the followings:
```bash
const char* ssid        = "WIFI_NAME";            *Type your Wi-Fi name instead
const char* password    = "WIFI_PASSWORD";        *Type your Wi-Fi password instead
const char* mqtt_broker = "YOUR_IP_ADDRESS";      *Paste your computers ip address
```
:warning: **Note that the esp and your pc must be connected to the same network** :warning:
![build and upload](https://dle-dev.com/wp-content/uploads/2020/10/PlatformIO_bluid_icon.png)
- Now just press **Build**, when build is done press **Upload**
- If everything went right you should be able to see your IP and messages with successful connections in the Terminal