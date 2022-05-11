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
#include <LiquidCrystal_I2C.h>

#include <SPI.h>
#include <printf.h>
#include <RF24.h>

#define debug 1
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
void display_listenning(String &msg);
void display_ip(String &strIP);
void display_vazio();
bool updateConfig(DynamicJsonDocument &doc);
void display_error(String &strMsgErro);
void display_msg(String strMsg1, String strMsg2 = " ");
bool connectedWeb(String &strMsgErro);
void reconnect_mqtt(String &strMsgErro);
void connect_mqtt(String &strMsgErro);
void publish_msg_mqtt();
String Hash256(String InputString);
String build_msg_mqtt();

BearSSL::CertStore certStore;
PubSubClient *client;

#define enderecoLcd  0x27
#define colunasLcd   16
#define linhasLcd    2

#define pinRele D3
#define pinBotaoMenu A0
#define pinBotaoReset 2 //pinBotaoReset 13

#define tempoDebounce 300

LiquidCrystal_I2C lcd(enderecoLcd, colunasLcd, linhasLcd);

unsigned int operation_mode = 0; //0 - AP, 1 - NORMAL

bool estadoBotaoMenu;
bool estadoBotaoReset;
bool estadoBotaoMenuAnt = LOW;
bool estadoBotaoResetAnt = HIGH;

int estadoAtual = 1;
int inputVal = 0;
AsyncWebServer server(80);
WiFiClient clienteWeb;

unsigned long previousMillis = 0;
unsigned long previousMillisMqtt = 0;
unsigned long currentMillisMqtt = millis();
unsigned long currentMillis = millis();
unsigned long previousMillisBtn = 0;
unsigned long previousMillisRadio = 0;
unsigned long lngdebounceBotao;
unsigned long previousMillisConnectedWeb = 0;

const char* PARAM_INPUT_CONFIGTEMP = "input1";
String LocalIpAdress;
String strPubMsgErro = "";
bool bPubConnectWeb = false;
String strPubModo = "a";
String strPubHashTopic = "";


struct estruturaDadosRF
{
   boolean ligando = false;   //Esta variavel será usada para solicitar os dados do outro aparelho. Será útil quando o aparelho solicitante esta sendo ligado, para manter os valores do aparelho que já esta ligado.
   boolean alterado = false;
   boolean botao1 = false;
   boolean botao2 = false;
   int potenciometro1 = 0;
   boolean sensor1 = false;
   boolean sensor2 = false;
   boolean sensor3 = false;
   boolean sensor4 = false;
   boolean sensor5 = false;
   boolean sensor6 = false;
};

typedef struct estruturaDadosRF tipoDadosRF;
tipoDadosRF dadosRF;
tipoDadosRF dadosRecebidos;
boolean transmitido = true;
boolean alterado = false;
RF24 radio(2,15);
boolean botao2Ant = LOW;
boolean botao2    = LOW;
uint64_t enderecos[6] = {0x7878787878LL,
                      0xB3B4B5B6F1LL,
                      0xB3B4B5B6CDLL,
                      0xB3B4B5B6A3LL,
                      0xB3B4B5B60FLL,
                      0xB3B4B5B605LL
                      };
#define tempoParado 1000


void setup_radio() {
  #if debug == 1
    Serial.println(F("Iniciando rádio"));
    display_msg("Radio setup");
  #endif
  if (!radio.begin()) {
    #if debug == 1
      Serial.println(F("radio hardware is not responding!!"));
    #endif
    while (1) {} // hold in infinite loop
  }
  radio.setChannel(100);
  radio.setPALevel(RF24_PA_LOW);
  radio.setDataRate(RF24_250KBPS);
  radio.openWritingPipe(enderecos[4]);
  radio.openReadingPipe(1, enderecos[1]);
  radio.openReadingPipe(2, enderecos[2]);
  // dadosRF.ligando = true;
  // radio.stopListening();
  // long tempoInicio = millis();
  // while ( !radio.write( &dadosRF, sizeof(tipoDadosRF) ) ) {
  //    if ((millis() - tempoInicio) > 2000) {
  //       break;
  //    }
  // }
  // dadosRF.ligando = false;
  radio.startListening();
  #if debug == 1
    radio.printPrettyDetails();
  #endif
}


void loop_button(){
    inputVal = analogRead(pinBotaoMenu);
    currentMillis = millis();

    if (currentMillis < previousMillis) {
        previousMillis = 0;
    }

    if (currentMillis - previousMillis >= long(config["interval"]))
    {
        if (estadoAtual == 1) {
            String msg = "aguardando";
            display_listenning(msg);
        }

        previousMillis = currentMillis;
    } //verifica o intervalo e se o modo esta no automatico

    //tratar bug estouro millis()
    if (currentMillis < lngdebounceBotao) {
        lngdebounceBotao = 0;
    }
    if (byte(config["usar_display"] == "1")) {
        estadoBotaoMenu = inputVal > 1000;
        if ( (currentMillis - lngdebounceBotao) > tempoDebounce) {
            if (!estadoBotaoMenu && estadoBotaoMenuAnt) {
                estadoAtual++;
                if (estadoAtual > 5) {
                    estadoAtual = 1;
                }
                #if debug == 1
                  Serial.print("estadoAtual: ");
                  Serial.println(estadoAtual);
                #endif
                switch (estadoAtual) {
                  case 1: {
                    String msg = "teste";
                    display_listenning(msg);
                    break;
                  }
                  case 2: {
                      display_ip(LocalIpAdress);
                      break;
                    }
                    case 3 : {
                      if (strPubModo == "a") {
                          display_msg("Modo de operacao", "Automatico");
                      } else {
                          display_msg("Modo de operacao", "Manual");
                      }
                      break;
                    }
                    case 4 : {
                      String strMsg1 = "Time Sensor: " + String(long(config["interval"]) /1000);
                      String strMsg2 = "Time Mqtt:   " + String(long(config["interval_mqtt"]) /1000);
                      display_msg(strMsg1, strMsg2);
                      break;
                    }
                    case 5 : {
                      display_error(strPubMsgErro);
                      break;
                    }
                }
                lngdebounceBotao = millis();
            } //botaoMenu pressionado
        } //controle umbouncing botaoMenu
        estadoBotaoMenuAnt = estadoBotaoMenu;
    } //usar display
    //botao para dar um reset na placa
    estadoBotaoReset = inputVal >= 700 && inputVal < 1000;
    if ( (currentMillis - lngdebounceBotao) > tempoDebounce) {
      bool bolTimeBotao = false;
      if (estadoBotaoReset && estadoBotaoResetAnt) {

          #if debug == 1
              Serial.println("botao reset precionado");
          #endif

          int iTimeBotao = 0;
          while(analogRead(pinBotaoMenu) >= 700 && analogRead(pinBotaoMenu) < 1000) {
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
    yield();
}


void loop_radio() {
    if (!alterado) {
      if (radio.available()) {
          Serial.println("radio.available()");
          radio.read( &dadosRecebidos, sizeof(tipoDadosRF) );
         //verifica se houve solicitação de envio dos dados
         if (dadosRecebidos.ligando) {
            alterado = true;
         } else {
            #if debug == 1
              dadosRF = dadosRecebidos;
              Serial.println("dados recebidos");
              Serial.print("RECEBIDO dadosRF.botao1: ");
              Serial.println(dadosRF.botao1);
              Serial.print("RECEBIDO dadosRF.botao2: ");
              Serial.println(dadosRF.botao2);
              Serial.print("RECEBIDO dadosRF.potenciometro1: ");
              Serial.println(dadosRF.potenciometro1);
              if (dadosRF.potenciometro1 > 45) {
                  ligar_rele();
              } else {
                desligar_rele();
              }
              radio.stopListening();
              Serial.println("tentando transmitir");
              transmitido = radio.write( &dadosRF, sizeof(tipoDadosRF) );
              radio.setRetries(3,5);
              if (transmitido) {
                  Serial.println("Transmitido - houve alteracao dos dados");
              }
              radio.startListening();

            #endif
         }
      } else {
        #if debug == 1
            Serial.println("Rádio Indisponível");
        #endif
      }
  }
}

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

  server.begin();
  rotas_web_default();

  pinMode(pinRele, OUTPUT);

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
        if (client->connected()) {
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

  setup_radio();

  #if debug == 1
    //Serial.println("Hash256");
    //Serial.println(Hash256("device/sensor/ronan/granja/1"));
    Serial.println("strPubMsgErro");
    Serial.print(strPubMsgErro);
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
        currentMillisMqtt = millis();
        if (millis() - previousMillisBtn > 300)
        {
            loop_button();
            previousMillisBtn = millis();

        }
        if (millis() - previousMillisRadio > tempoParado)
        {
            loop_radio();
            previousMillisRadio = millis();

        }
        if (currentMillisMqtt < previousMillisConnectedWeb) {
            previousMillisConnectedWeb = 0;
        }

        if (currentMillisMqtt - previousMillisConnectedWeb >= long(config["interval_mqtt"])) {
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

            previousMillisConnectedWeb = currentMillisMqtt;
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

        currentMillisMqtt = millis();

        if (currentMillisMqtt < previousMillisMqtt) {
            previousMillisMqtt = 0;
        }

        if (bPubConnectWeb) {
            if (currentMillisMqtt - previousMillisMqtt >= long(config["interval_mqtt"])) {
                publish_msg_mqtt();
                previousMillisMqtt = currentMillis;
            }
        } //verifica se esta conectado e envia a mensagem para o MQTT
    } //verifica se o modo de operacao
}

