#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <time.h>
#include <TZ.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <CertStoreBearSSL.h>
#include <FS.h>
#include <ACS712.h>

#define debug 0
#define MSG_BUFFER_SIZE (500)

DynamicJsonDocument config(2048);

//Funcoes
void handleConfig(AsyncWebServerRequest *request);
void write_default_config(File &configFile);
bool loadConfig(DynamicJsonDocument &doc);
bool saveConfig(DynamicJsonDocument &doc, String novo);
void notFound(AsyncWebServerRequest *request);
void rotas_web_modo_1();
void rotas_web_default();
bool updateConfig(DynamicJsonDocument &doc);
String build_msg_json();

void setDateTime() {
  configTime(TZ_America_Sao_Paulo, "pool.ntp.org", "time.nist.gov");
#if debug == 1
  Serial.print("Waiting for NTP time sync: ");
#endif
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(500);
#if debug == 1
    Serial.println(".");
#endif
    now = time(nullptr);
  }
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
#if debug == 1
  Serial.print(".");
  Serial.print("Current time: ");
  Serial.print(asctime(&timeinfo));
#endif
}

unsigned int operation_mode = 0;  //0 - AP, 1 - NORMAL
BearSSL::CertStore certStore;

#define pinRele D2

#define tempoDebounce 300

int estadoAtual = 1;

AsyncWebServer server(80);
WiFiClient clienteWeb;
ACS712 ACS(A0, 3.3, 1023, 185);

unsigned long previousMillis = 0;
unsigned long previousMillisMqtt = 0;
unsigned long currentMillis = millis();
unsigned long previousMillisConnectedWeb = 0;

const char *PARAM_INPUT_CONFIGTEMP = "input1";
String LocalIpAdress;
String strPubMsgErro = "";
bool bPubConnectWeb = false;
String strPubModo = "a";
String strPubHashTopic = "";
int corrente = 0;

bool connectedWeb(String &strMsgErro) {

#if debug == 1
  Serial.println("inicio funcao connectedWeb");
#endif

  if (clienteWeb.connect("google.com", 80) == true) {
#if debug == 1
    Serial.println("fim funcao connectedWeb");
#endif

    strMsgErro = "";
    return true;
  } else {
#if debug == 1
    Serial.println("Falha na conexão com Servidor Google APIs");
    Serial.println("fim funcao connectedWeb");
#endif
    strMsgErro = "No Connect Web";
    return false;
  }
}

void ligar_rele() {
  digitalWrite(pinRele, LOW);
}

void desligar_rele() {
  digitalWrite(pinRele, HIGH);
}

void read_corrente() {
  corrente = ACS.mA_AC();
  if (corrente > int(config["temp"])) {
    ligar_rele();
  } else {
    desligar_rele();
  }
#if debug == 1
  Serial.println(String(config["temp"]));
  Serial.print("mA: ");
  Serial.print(corrente);
  Serial.print(". Form factor: ");
  Serial.println(ACS.getFormFactor());
#endif
}

const int sensorIn = A0;  //analog pin# A0;
int mVperAmp = 100;       // use 100 for 20A Module and 66 for 30A Module
double Voltage = 0;
double VRMS = 0;
double AmpsRMS = 0;
int decimalPrecision = 3;

float getVPP() {
  float result;
  int readValue;        //value read from the sensor
  int maxValue = 0;     // store max value here
  int minValue = 1024;  // store min value here
  uint32_t start_time = millis();
  while ((millis() - start_time) < 3000) {
    readValue = analogRead(sensorIn);
    // see if you have a new maxValue
    if (readValue > maxValue) {
      maxValue = readValue;
    }
    if (readValue < minValue) {
      minValue = readValue;
    }
  }
  result = ((maxValue - minValue) * 5.0) / 1024.0;
  return result;
}

void setup() {
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
    strPubModo = String(config["modo_operacao"]);
  }
#if debug == 1
  Serial.print("Usando o Display: ");
  Serial.println(int(config["usar_display"]));
#endif

  server.begin();
  rotas_web_default();

  pinMode(pinRele, OUTPUT);

  const String ssid = config["wifi_ssid"];
  const String wifi_key = config["wifi_key"];

  if (ssid == "") {
    const String soft_ap = config["soft_ap"];
    WiFi.softAP(soft_ap);
#if debug == 1
    Serial.println("Iniciar SoftAP");
    Serial.print("IP address: ");
    Serial.println(WiFi.softAPIP());
#endif
  } else {
    operation_mode = 1;
    WiFi.begin(ssid, wifi_key);
    WiFi.setAutoReconnect(true);

#if debug == 1
    Serial.println("Connecting to WiFi");
#endif

    int iCont = 1;
    while ((WiFi.status() != WL_CONNECTED) && (iCont < 16)) {
      delay(1000);
#if debug == 1
      Serial.print(iCont);
      Serial.println(" .");
#endif
      iCont++;
    }

    if (WiFi.status() != WL_CONNECTED) {
      strPubMsgErro = "WIFI no connected";
    } else {
#if debug == 1
      Serial.println(WiFi.localIP());
#endif
      LocalIpAdress = WiFi.localIP().toString();
      setDateTime();
    }
    rotas_web_modo_1();
#if debug == 1
    Serial.print("ACS712_LIB_VERSION: ");
    Serial.println(ACS712_LIB_VERSION);
#endif
    ACS.autoMidPoint();
#if debug == 1
    Serial.print("MidPoint: ");
    Serial.print(ACS.getMidPoint());
    Serial.print(". Noise mV: ");
    Serial.println(ACS.getNoisemV());
    read_corrente();
#endif
  }
}

void loop() {
  if (operation_mode == 1) {

    currentMillis = millis();

    if (currentMillis < previousMillisConnectedWeb) {
      previousMillisConnectedWeb = 0;
    }

    if (currentMillis - previousMillisConnectedWeb >= long(config["interval_mqtt"])) {
      if (LocalIpAdress != "") {

        if (connectedWeb(strPubMsgErro)) {
          bPubConnectWeb = true;
          strPubMsgErro = "";
        } else {
          bPubConnectWeb = false;
          strPubMsgErro = "No Connect Web";

          //TODO
        }
      } else {
        bPubConnectWeb = false;
        strPubMsgErro = "No Connect Web";
      }

      previousMillisConnectedWeb = currentMillis;
    }

    if ((LocalIpAdress == "") && (WiFi.status() == WL_CONNECTED)) {
#if debug == 1
      Serial.println("Entrou para conectar apos o WIFI retornar: ");
#endif
      LocalIpAdress = WiFi.localIP().toString();


      if (connectedWeb(strPubMsgErro)) {
        bPubConnectWeb = true;
      } else {
        bPubConnectWeb = false;
        strPubMsgErro = "No Connect Web";
      }
    }

    currentMillis = millis();

    if (currentMillis < previousMillis) {
      previousMillis = 0;
    }

    if (currentMillis - previousMillis >= long(config["interval"]) && strPubModo == "a") {
      previousMillis = currentMillis;
    }  //verifica o intervalo e se o modo esta no automatico

    if (bPubConnectWeb) {
      if (currentMillis - previousMillisMqtt >= long(config["interval_mqtt"])) {
        read_corrente();
        previousMillisMqtt = currentMillis;
      }
    }
  }
}
