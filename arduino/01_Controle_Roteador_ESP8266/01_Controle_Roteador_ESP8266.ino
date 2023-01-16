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

int estadoAtual = 1;

AsyncWebServer server(80);
WiFiClient clienteWeb;

unsigned long currentMillis = millis();
unsigned long previousMillisConnectedWeb = 0;
unsigned long shutdown_milli = 0;

const char *PARAM_INPUT_CONFIGTEMP = "input1";
const char *PARAM_INPUT_CONFIGSITE = "input2";
String LocalIpAdress;
String strPubMsgErro = "";
bool bPubConnectWeb = false;
String strPubModo = "a";
String strPubHashTopic = "";

bool connectedWeb(String &strMsgErro) {

#if debug == 1
  Serial.println("inicio funcao connectedWeb");
#endif

  if (clienteWeb.connect(String(config["site_check"]), 80) == true) {
#if debug == 1
    Serial.println("OK, conectado a web");
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
#if debug == 1
  Serial.println("Liguei relé");
#endif
  digitalWrite(pinRele, HIGH);
  estadoAtual = 1;
}

void desligar_rele() {
#if debug == 1
  Serial.println("Desliguei relé");
#endif
  digitalWrite(pinRele, LOW);
  estadoAtual = 0;
}

void setup() {
#if debug == 1
  Serial.begin(115200);
  Serial.println("Inicio");
#endif

  pinMode(pinRele, OUTPUT);
  digitalWrite(pinRele, HIGH);
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
  }
}

void watch_dog() {
  if (!bPubConnectWeb) {
    if (shutdown_milli == 0 && estadoAtual == 1) {
      Serial.print(currentMillis + " ");
      desligar_rele();
      shutdown_milli = currentMillis;
    }
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
        }
      } else {
        bPubConnectWeb = false;
        strPubMsgErro = "No Connect Web";
      }
      previousMillisConnectedWeb = currentMillis;
      watch_dog();
    }
    if (estadoAtual == 0 && currentMillis - shutdown_milli >= (long(config["interval"]))) {
      Serial.print(currentMillis + " ");
      ligar_rele();
    }
    if (currentMillis - shutdown_milli >= (long(config["temp"]) * 1000) && estadoAtual == 0) {
      Serial.println(currentMillis + " Restaurando após tempo de restart");
      shutdown_milli = 0;
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
  }
}
