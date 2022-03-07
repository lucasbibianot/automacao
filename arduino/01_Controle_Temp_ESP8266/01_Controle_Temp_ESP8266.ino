#include <ArduinoECCX08.h>

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Adafruit_Sensor.h>
#include <PubSubClient.h>
#include <time.h>
#include <TZ.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <CertStoreBearSSL.h>
#include <FS.h>
#include <DHT.h>
#include <LiquidCrystal_I2C.h>

#define debug 1

DynamicJsonDocument config(2048);

//Funcoes
bool loadParam(DynamicJsonDocument &docParam);
void handleConfig(AsyncWebServerRequest * request);
void write_default_param(File &paramFile);
void write_default_config(File &configFile);
bool loadConfig(DynamicJsonDocument &doc);
bool saveParam(DynamicJsonDocument &docParam, String novo);
bool saveConfig(DynamicJsonDocument &doc, String novo);
void notFound(AsyncWebServerRequest *request);
void rotas_web_modo_1();
void rotas_web_default();
void display_temp_hum(float &fltTemp, float &fltHum);
void display_ip(String &strIP);
void display_vazio();
void display_temp_config(float fltTempConfig, String strQtdBoot);
bool updateConfig(DynamicJsonDocument &doc);
void display_error(String &strMsgErro);
void display_msg(String strMsg1, String strMsg2 = " ");
bool connectedWeb(String &strMsgErro);
void reconnect_mqtt(String &strMsgErro);
void connect_mqtt(String &strMsgErro);

unsigned int operation_mode = 0; //0 - AP, 1 - NORMAL
BearSSL::CertStore certStore;
PubSubClient *client;
#define MSG_BUFFER_SIZE (500)
char msg[MSG_BUFFER_SIZE];

#define enderecoLcd  0x27
#define colunasLcd   16
#define linhasLcd    2

#define pinDHT  12
#define typeDHT DHT22     // DHT22
#define pinRele 14
#define pinBotao 15

#define tempoDebounce 200

DHT dht(pinDHT, typeDHT);
LiquidCrystal_I2C lcd(enderecoLcd, colunasLcd, linhasLcd);
unsigned long lngdebounceBotao;
bool estadoBotao;
bool estadoBotaoAnt = LOW;
bool estadoRele = false;
int estadoAtual = 1;
float fltTemperatura = 0.0;
float fltHumidade = 0.0;
AsyncWebServer server(80);
WiFiClient clienteWeb;
unsigned long previousMillis = 0;
unsigned long previousMillisMqtt = 0;
unsigned long currentMillis = millis();
const char* PARAM_INPUT_CONFIGTEMP = "input1";
String LocalIpAdress;
String strPubMsgErro = "";
bool bPubConnectWeb = false;

void setup()
{
  #if debug == 1
    Serial.begin(115200);
    Serial.println("Inicio");
  #endif
  LittleFS.begin();
  if (loadConfig(config)) {
    #if debug == 1
        Serial.println("Carreguei as configurações");
    #endif
    int intQtdBoot = 0;
    intQtdBoot = int(config["qtd_boot"]) + 1;
    String strQtdBoot = String(intQtdBoot);
    config["qtd_boot"] = strQtdBoot;
    updateConfig(config);
  }
  #if debug == 1
    Serial.print("Usando o Display: ");
    Serial.println(int(config["usar_display"]));
  #endif
  dht.begin();
  server.begin();
  rotas_web_default();
  pinMode(pinRele, OUTPUT);
  pinMode(pinBotao, INPUT);
  if (int(config["usar_display"] == 1 )) {
    lcd.init();
    lcd.backlight();
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Inicio");
    if (strPubMsgErro != "") {
        display_error(strPubMsgErro);
    }
  }
  const String ssid = config["wifi-ssid"];
  const String wifi_key = config["wifi-key"];
  if (ssid == "") {
    const String soft_ap = config["soft-ap"];
    WiFi.softAP(soft_ap);
    LocalIpAdress = WiFi.softAPIP().toString();
    display_msg("Modo AP", LocalIpAdress);
    #if debug == 1
      Serial.println("Iniciar SoftAP");
      Serial.print("IP address: ");
      Serial.println(WiFi.softAPIP());
    #endif
  }
  else {
    operation_mode = 1;
    WiFi.begin(ssid, wifi_key);
    #if debug == 1
        Serial.println("Connecting to WiFi");
    #endif
    int iCont = 1;
    while ( (WiFi.status() != WL_CONNECTED) && (iCont < 16))
    {
        delay(1000);
        #if debug == 1
          Serial.print(iCont);
          Serial.println(" .");
        #endif
        display_msg("Conectando WIFI", String(iCont) );
        iCont++;
    }
    if (WiFi.status() != WL_CONNECTED) {
        strPubMsgErro = "WIFI no connected";
    }
    else
    {
        #if debug == 1
            Serial.println(WiFi.localIP());
        #endif
        LocalIpAdress = WiFi.localIP().toString();
        if (connectedWeb(strPubMsgErro)) {
            connect_mqtt(strPubMsgErro);
            bPubConnectWeb = true;
            client->subscribe(config["topic_subscribe"]);
            #if debug == 1
              String subscribed = config["topic_subscribe"];
              Serial.print("Subscribed: ");
              Serial.println(subscribed);
            #endif
        }
    }
    rotas_web_modo_1();
  }
  if (strPubMsgErro != "") {
    display_error(strPubMsgErro);
  }
  #if debug == 1
    Serial.println("Fim Setup");
  #endif
}

void loop()
{
  if (operation_mode == 1) {
    if ((LocalIpAdress == "") && (WiFi.status() == WL_CONNECTED)){
        #if debug == 1
          Serial.println("Entrou para conectar apos o WIFI retornar: ");
        #endif
        LocalIpAdress = WiFi.localIP().toString();
    }
    if (connectedWeb(strPubMsgErro) && !client->connected())
    {
        reconnect_mqtt(strPubMsgErro);
    }
    client->loop();
    currentMillis = millis();
    if (currentMillis < previousMillis) {
        previousMillis = 0;
    }
    if (currentMillis - previousMillis >= long(config["interval"]))
    {
      if (fltTemperatura < float(config["temp"])) {
        digitalWrite(pinRele, HIGH);
        estadoRele = true;
      } else {
        digitalWrite(pinRele, LOW);
        estadoRele = false;
      }
      float newT = dht.readTemperature();
      if (!isnan(newT))
      {
          fltTemperatura = newT;
          #if debug == 1
              Serial.print("fltTemperatura: ");
              Serial.println(fltTemperatura);
          #endif
      }
      else {
          fltTemperatura = 0;
      }
      float newH = dht.readHumidity();
      if (!isnan(newH))
      {
          fltHumidade = newH;
          #if debug == 1
              Serial.print("fltHumidade: ");
              Serial.println(fltHumidade);
          #endif
      }
      else {
          fltHumidade = 0;
      }
      if (int(config["usar_display"]) == 1) {
        if (currentMillis < lngdebounceBotao) {
            lngdebounceBotao = 0;
        }
        estadoBotao = digitalRead(pinBotao);
        if ( (currentMillis - lngdebounceBotao) > tempoDebounce) {
          if (!estadoBotao && estadoBotaoAnt) {
              estadoAtual++;
              if (estadoAtual > 4) {
                 estadoAtual = 1;
              }
              #if debug == 1
                Serial.print("estadoAtual: ");
                Serial.println(estadoAtual);
              #endif
              switch (estadoAtual) {
                case 1: {
                  display_temp_hum(fltTemperatura, fltHumidade);
                  break;
                }
                case 2: {
                    display_ip(LocalIpAdress);
                    break;
                 }
                 case 3 : {
                    display_temp_config(config["temp"], config["qtd_boot"]);
                    break;
                 }
                 case 4 : {
                    display_error(strPubMsgErro);
                    break;
                 }
              }
              lngdebounceBotao = millis();
          }
        }
        estadoBotaoAnt = estadoBotao;
      }
      previousMillis = currentMillis;
    }
    if (connectedWeb(strPubMsgErro)) {
      if (currentMillis - previousMillisMqtt >= long(config["interval_mqtt"])) {
        snprintf(msg, MSG_BUFFER_SIZE, "{\"temp\": %.2f, \"hum\": %.2f, \"temp_config\": %.2f, \"qtd_boot\": %s, \"topic_subscribe\": \"%s\", \"estadoRele\": %s }", fltTemperatura, fltHumidade, float(config["temp"]), String(config["qtd_boot"]).c_str(), String(config["topic_subscribe"]).c_str(), String(estadoRele));
        #if debug == 1
            Serial.print("client: ");
            Serial.println(client != 0);
        #endif
        if (client != 0) {
          client->publish(config["topic"], msg);
          #if debug == 1
              Serial.println(msg);
          #endif
        }
         previousMillisMqtt = currentMillis;
      }
    }
  }
}
