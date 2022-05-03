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

// Import required libraries Components
#include <DHT.h>
#include <LiquidCrystal_I2C.h>

#define debug 0
#define MSG_BUFFER_SIZE (500)

DynamicJsonDocument config(2048);


//Funcoes
void handleConfig(AsyncWebServerRequest * request);
void write_default_config(File &configFile);
bool loadConfig(DynamicJsonDocument &doc);
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
void publish_msg_mqtt();
String Hash256(String InputString);
String build_msg_mqtt();

unsigned int operation_mode = 0; //0 - AP, 1 - NORMAL
BearSSL::CertStore certStore;
PubSubClient *client;

#define enderecoLcd  0x27
#define colunasLcd   16
#define linhasLcd    2

#define pinDHT  0
#define typeDHT DHT22     // DHT22
#define pinRele 14
#define pinBotaoMenu 15
#define pinBotaoReset 2 //pinBotaoReset 13

#define tempoDebounce 300

DHT dht(pinDHT, typeDHT);
LiquidCrystal_I2C lcd(enderecoLcd, colunasLcd, linhasLcd);

bool estadoBotaoMenu;
bool estadoBotaoReset;
bool estadoBotaoMenuAnt = LOW;
bool estadoBotaoResetAnt = HIGH;

int estadoAtual = 1;

float fltTemperatura = 0.0;
float fltHumidade = 0.0;

AsyncWebServer server(80);
WiFiClient clienteWeb;

unsigned long previousMillis = 0;
unsigned long previousMillisMqtt = 0;
unsigned long currentMillis = millis();
unsigned long lngdebounceBotao;
unsigned long previousMillisConnectedWeb = 0;

const char* PARAM_INPUT_CONFIGTEMP = "input1";
String LocalIpAdress;
String strPubMsgErro = "";
bool bPubConnectWeb = false;
String strPubModo = "a";
String strPubHashTopic = "";

void ligar_rele(){
  digitalWrite(pinRele, HIGH);
}

void desligar_rele() {
  digitalWrite(pinRele, LOW);
}

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

    //strPubModo = config["modo_operacao"];
    strPubModo =  String(config["modo_operacao"]);
  }
  #if debug == 1
    Serial.print("Usando o Display: ");
    Serial.println(int(config["usar_display"]));
  #endif

  dht.begin();
  server.begin();
  rotas_web_default();

  pinMode(pinRele, OUTPUT);
  pinMode(pinBotaoMenu, INPUT);
  pinMode(pinBotaoReset, INPUT_PULLUP);  

  if (byte(config["usar_display"] == "1" )) {
    lcd.init();
    lcd.backlight();
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Inicio");
    if (strPubMsgErro != "") {
        display_error(strPubMsgErro);
    }
  }
  const String ssid = config["wifi_ssid"];
  const String wifi_key = config["wifi_key"];

  if (ssid == "") {
    const String soft_ap = config["soft_ap"];
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
    WiFi.setAutoReconnect (true);

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
        //if (connectedWeb(strPubMsgErro)) {
        connect_mqtt(strPubMsgErro);
        
        client->subscribe(config["topic_subscribe"]);
        #if debug == 1
          String subscribed = config["topic_subscribe"];
          Serial.print("Subscribed: ");
          Serial.println(subscribed);
        #endif
        if (!client->connected()) {
            bPubConnectWeb = true;
        } else {
            bPubConnectWeb = false;
            strPubMsgErro = "No Connect Web";
        }
        //}
    }
    rotas_web_modo_1();
  }

  if (strPubMsgErro != "") {
    display_error(strPubMsgErro);
  }

  #if debug == 1
    //Serial.println("Hash256");    
    //Serial.println(Hash256("device/sensor/ronan/granja/1"));

    Serial.println("Topic");        
    strPubHashTopic = String(config["topic"]);
    Serial.println(strPubHashTopic);  
    strPubHashTopic = Hash256(strPubHashTopic);
    Serial.println(strPubHashTopic);

    Serial.println("Fim Setup");
  #endif
}

void loop()
{
    if (operation_mode == 1) {
    
        currentMillis = millis();
    
        if (currentMillis < previousMillisConnectedWeb) {
            previousMillisConnectedWeb = 0;
        }
    
        if (currentMillis - previousMillisConnectedWeb >= long(config["interval_mqtt"])) {
            if (LocalIpAdress != "") {

                #if debug == 1
                    Serial.print("client: ");
                    Serial.println(client->connected());
                #endif              
                //if (connectedWeb(strPubMsgErro)) {
                if (client->connected()) {
                  bPubConnectWeb = true;
                  strPubMsgErro = "";
                } else {
                  bPubConnectWeb = false;
                  strPubMsgErro = "No Connect Web";
                           
                  reconnect_mqtt(strPubMsgErro);        
                }
            } else {
                bPubConnectWeb = false;
                strPubMsgErro = "No Connect Web";
            }
      
            previousMillisConnectedWeb = currentMillis;
        }

        if ((LocalIpAdress == "") && (WiFi.status() == WL_CONNECTED)){
            #if debug == 1
              Serial.println("Entrou para conectar apos o WIFI retornar: ");
            #endif
            LocalIpAdress = WiFi.localIP().toString();
            
            connect_mqtt(strPubMsgErro);            
            client->subscribe(config["topic_subscribe"]);
            
            if (!client->connected()) {
                bPubConnectWeb = true;
            } else {
                bPubConnectWeb = false;
                strPubMsgErro = "No Connect Web";
            }            
        }

        //if (bPubConnectWeb && !client->connected())
        if (bPubConnectWeb)
        {
            client->loop();            
        }

        currentMillis = millis();

        if (currentMillis < previousMillis) {
            previousMillis = 0;
        }

        if (currentMillis - previousMillis >= long(config["interval"]) && strPubModo == "a")
        {
            if (fltTemperatura < float(config["temp"])) {
              ligar_rele();
            } else {
              desligar_rele();
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

            if (estadoAtual == 1) {
                display_temp_hum(fltTemperatura, fltHumidade);
            }            

            previousMillis = currentMillis;
        } //verifica o intervalo e se o modo esta no automatico           

        //tratar bug estouro millis()
        if (currentMillis < lngdebounceBotao) {
            lngdebounceBotao = 0;
        }
        
        //botao para dar um reset na placa        
        estadoBotaoReset = digitalRead(pinBotaoReset);        
        if ( (currentMillis - lngdebounceBotao) > tempoDebounce) {

          bool bolTimeBotao = false;
          if (!estadoBotaoReset && estadoBotaoResetAnt) {

              #if debug == 1
                  Serial.println("botao reset precionado");
              #endif
                      
              int iTimeBotao = 0;
              while(!digitalRead(pinBotaoReset)) {
                  if ( (millis() - lngdebounceBotao) > 1000) {
                      iTimeBotao ++;
                      lngdebounceBotao = millis();
                      #if debug == 1
                          Serial.println("botao reset precionado loop: ");
                          Serial.println(iTimeBotao);                      
                      #endif                             
                  }

                  if (iTimeBotao == 5) {
                      bolTimeBotao = true;   
                      break;                   
                  }
              }
              if (bolTimeBotao) {
                  config["wifi_ssid"] = "";
                  updateConfig(config);  
              
                  ESP.restart();                   
              }
              lngdebounceBotao = millis();
          }          
        }    
        estadoBotaoResetAnt = estadoBotaoReset;    
           
        if (byte(config["usar_display"] == "1")) {
            estadoBotaoMenu = digitalRead(pinBotaoMenu);
            if ( (currentMillis - lngdebounceBotao) > tempoDebounce) {
                if (!estadoBotaoMenu && estadoBotaoMenuAnt) {
                    estadoAtual++;
                    if (estadoAtual > 6) {
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
                          if (strPubModo == "a") {
                              display_msg("Modo de operacao", "Automatico");
                          } else {
                              display_msg("Modo de operacao", "Manual");
                          }
                          break;
                       }
                       case 5 : {
                          String strMsg1 = "Time Sensor: " + String(long(config["interval"]) /1000);
                          String strMsg2 = "Time Mqtt:   " + String(long(config["interval_mqtt"]) /1000);
                          display_msg(strMsg1, strMsg2);
                          break;
                       }                            
                       case 6 : {
                          display_error(strPubMsgErro);
                          break;
                       }            
                    }
                    lngdebounceBotao = millis();
                } //botaoMenu pressionado
            } //controle umbouncing botaoMenu
            estadoBotaoMenuAnt = estadoBotaoMenu;
        } //usar display

        if (bPubConnectWeb) {
            if (currentMillis - previousMillisMqtt >= long(config["interval_mqtt"])) {
                publish_msg_mqtt();                             
                previousMillisMqtt = currentMillis;
            } 
        } //verifica se esta conectado e envia a mensagem para o MQTT
    } //verifica se o modo de operacao   
}
