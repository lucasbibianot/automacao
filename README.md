# Automação - POC

Para a POC utilizamos uma ESP-826612E com o [firmware NodeMCU](https://blogmasterwalkershop.com.br/arquivos/firmware/nodemcu_integer_0.9.6-dev_20150704.bin)
Para trocar o firmware da ESP utilizamos o [Tasmotizer](https://github.com/tasmota/tasmotizer#installation-and-how-to-run) **O tasmotizer instala qualquer firware em ESP e NodeMCU**

## Configurações do Arduino IDE

To write code for an Arduino you first need to install the proprietary Arduino IDE. After installation, enable support for the ESP8266 chip, which provides the WiFi functionality. To do this, start the Arduino IDE and open the Preferences window.
Enter https://arduino.esp8266.com/stable/package_esp8266com_index.json into the File>Preferences>Additional Boards Manager URLs field of the Arduino IDE. You can add multiple URLs, by separating them with commas. Afterwards, open Tools>Board>Boards Manager and install the esp8266 platform by searching for it. Then you can select your ESP8266 board in Tools>Board. In testing of this example, the Wemos D1 mini was used, but any board with an ESP8266 should work.

## Bibliotecas utilizadas no Sketch

**PubSubClient library**
To use HiveMQ Cloud an MQTT library has to be added first. In this example the PubSubClient is used but there are a lot of other alternatives that provide similar functionalities. Download the PubSubClient from Github as a .zip file. Then go to the Arduino IDE, open Sketch>Include Library>Add .ZIP Library and select the downloaded .zip file.

**NTPClient library**
The next library that is required is the NTPClient. Install this library by opening Tools>Manage Libraries... and searching for NTPClient. This library provides functionality for date and time. Here it is used to update the internal clock of the ESP8266 board, in order to use TLS certificates.

**LittleFS Filesystem Uploader**
The LittleFS Filesystem Uploader is needed to upload certificates to your board. This enables you to connect securely with HiveMQ Cloud. Download the ESP8266LittleFS-X.zip. Go to the Arduino IDE directory and open the tools folder. Unzip the downloaded .zip folder to the tools folder. The result will be a folder called ESP8266LittleFS-2.6.0 or it can have a different version. This folder contains the ESP8266LittleFS folder. Cut the ESP8266LittleFS folder and paste it into the tools folder. Afterwards, you can delete the ESP8266LittleFS-2.6.0 folder. The resulting folder structure should look like this:
`<home_dir>/Arduino-<version>/tools/ESP8266LittleFS/tool/esp8266littlefs.jar`
On OS X create the tools directory in ~/Documents/Arduino/ and unpack the files there. Now restart Arduino IDE and ESP8266 LittleFS Data Upload will appear when opening the tools menu.

**Upload certificates to the board**
The next step is to get the certificates that are needed for an encrypted connection to HiveMQ Cloud. For this purpose use this script by downloading the repository as a .zip folder. Only the file certs-from-mozilla.py is needed, you can delete the rest. Open this script in a Python IDE of your choice, for example PyCharm or Visual Studio Code. Execute the script in your IDE. A file will be generated, that is called certs.ar. Move this file into the directory of your Arduino IDE sketch. To find it you can go to Sketch>Show Sketch Folder in Arduino IDE. The folder where your sketch is saved should open. Inside this folder, create a new folder called data. Put your generated certs.ar into this data folder. In the Arduino IDE, in the Tools menu, you may want to select a bigger flash size, this depends on the size of your files. Then open Tools>ESP8266 LittleFS Data Upload. This will upload the files located in data to your board's flash memory. After a few seconds, you should get the message LittleFS Image Uploaded.

**Configuração DHT11/DHT22**
To read from the DHT sensor, we’ll use the DHT library from Adafruit. To use this library you also need to install the Adafruit Unified Sensor library. Follow the next steps to install those libraries.

1. Open your Arduino IDE and go to Sketch > Include Library > Manage Libraries. The Library Manager should open.
2. Search for “DHT” on the Search box and install the DHT library from Adafruit.
3. After installing the DHT library from Adafruit, type “Adafruit Unified Sensor” in the search box. Scroll all the way down to find the library and install it.

**ESP8266 Asynchronous Web Server**
The ESPAsyncWebServer library is not available to install in the Arduino IDE Library Manager. So, you need to install it manually.
Follow the next steps to install the ESPAsyncWebServer library:
[Click here](https://github.com/me-no-dev/ESPAsyncWebServer/archive/master.zip) to download the ESPAsyncWebServer library. You should have a .zip folder in your Downloads folder
Unzip the .zip folder and you should get ESPAsyncWebServer-master folder
Rename your folder from ESPAsyncWebServer-master to ESPAsyncWebServer
Move the ESPAsyncWebServer folder to your Arduino IDE installation libraries folde

**Installing the ESPAsync TCP Library**
The ESPAsyncWebServer library requires the ESPAsyncTCP library to work. Follow the next steps to install that library:

[Click here](https://github.com/me-no-dev/ESPAsyncTCP/archive/master.zip) to download the ESPAsyncTCP library. You should have a .zip folder in your Downloads folder
Unzip the .zip folder and you should get ESPAsyncTCP-master folder
Rename your folder from ESPAsyncTCP-master to ESPAsyncTCP
Move the ESPAsyncTCP folder to your Arduino IDE installation libraries folder
Finally, re-open your Arduino IDE

**Referências:**

- https://randomnerdtutorials.com/esp8266-dht11dht22-temperature-and-humidity-web-server-with-arduino-ide/
- https://blogmasterwalkershop.com.br/embarcados/esp8266/instalando-o-firmware-nodemcu-no-esp8266-esp-01
- https://console.hivemq.cloud/
