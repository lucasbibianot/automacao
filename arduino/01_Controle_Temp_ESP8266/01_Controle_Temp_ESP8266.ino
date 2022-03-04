/*********
  
  
*********/

// Import required libraries
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

#define debug 1

DynamicJsonDocument config(2048);
//DynamicJsonDocument param(2048);

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

// A single, global CertStore which can be used by all connections.
// Needs to stay live the entire time any of the WiFiClientBearSSLs
// are present.
BearSSL::CertStore certStore;

//WiFiClientSecure espClient;
PubSubClient *client;
#define MSG_BUFFER_SIZE (500)
char msg[MSG_BUFFER_SIZE];

//Controle do Display
#define enderecoLcd  0x27
#define colunasLcd   16
#define linhasLcd    2

#define pinDHT  0         // Digital pin connected to the DHT sensor
#define typeDHT DHT22     // DHT 22 (AM2302)
#define pinRele 14        // Digital pin rele
#define pinBotao 15       // Digital pin button

#define tempoDebounce 200

//Instanciando objetos
DHT dht(pinDHT, typeDHT);
LiquidCrystal_I2C lcd(enderecoLcd, colunasLcd, linhasLcd);

unsigned long lngdebounceBotao;
bool estadoBotao;
bool estadoBotaoAnt = LOW;

bool estadoRele = false;

int estadoAtual = 1;

// current temperature & humidity, updated in loop()
float fltTemperatura = 0.0;
float fltHumidade = 0.0;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

//WebClient Secure clienteWeb; //Cria um cliente seguro (para ter acesso ao HTTPS)
WiFiClient clienteWeb; //Cria um cliente seguro (para ter acesso ao HTTPS)

// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
unsigned long previousMillis = 0; // will store last time DHT was updated

unsigned long currentMillis = millis();

const char* PARAM_INPUT_CONFIGTEMP = "input1";

String LocalIpAdress;

String strPubMsgErro = "";
bool bPubConnectWeb = false;

void setup()
{
    // Serial port for debugging purposes
    #if debug == 1
      Serial.begin(115200);
      Serial.println("Inicio");
    #endif
    
    LittleFS.begin();

    dht.begin();
    pinMode(pinRele, OUTPUT);
    pinMode(pinBotao, INPUT);

    //Display
    lcd.init(); //inicia comunicacao com o display
    lcd.backlight(); //liga iluminacao do display
    lcd.clear(); //limpa o display
    lcd.setCursor(0,0); //posiciona o cursor na primeira coluna da linha 1
    lcd.print("Inicio");

    if (strPubMsgErro != "") {
        display_error(strPubMsgErro);
    } 
    
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
    // Connect to Wi-Fi
    const String ssid = config["wifi-ssid"];
    const String wifi_key = config["wifi-key"];
    //if (ssid == 0) {
    if (ssid == "") {
        const String soft_ap = config["soft-ap"];
        // Iniciar o SoftAP
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
        //Serial.printf("%s", ssid);
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
            // Print ESP8266 Local IP Address
            #if debug == 1
                Serial.println(WiFi.localIP());
            #endif
                
            LocalIpAdress = WiFi.localIP().toString();
        
            //verifica se esta conectado na internet e connecta no servidor mqtt
            if (connectedWeb(strPubMsgErro)) { 
                connect_mqtt(strPubMsgErro);
                     
                bPubConnectWeb = true;
            }                
        
        } // verifica se esta conectado no WIFI
  
        rotas_web_modo_1(); 
      
    } // verifica em qual modo esta 0: modo AP ou 1: modo WIFI 
  
    rotas_web_default(); 
    
    // Start server    
    server.begin();

    if (strPubMsgErro != "") {
        display_error(strPubMsgErro);
    }    
}

void loop()
{
    if (operation_mode == 1) {

      if ((LocalIpAdress == "") && (WiFi.status() == WL_CONNECTED)){
          #if debug == 1  
            Serial.println("Entrou para conectar apos o WIFI retornar: ");
          #endif            
          LocalIpAdress = WiFi.localIP().toString();
          
          if (connectedWeb(strPubMsgErro)) { 
              connect_mqtt(strPubMsgErro);          
              bPubConnectWeb = true;
          }
      }

      if (!bPubConnectWeb && LocalIpAdress != "") {
          if (connectedWeb(strPubMsgErro)) {
              connect_mqtt(strPubMsgErro);          
              bPubConnectWeb = true;        
          }
      }

      currentMillis = millis();
      
      //trata o bug do millis() zerar depois de 49 dias
      if (currentMillis < previousMillis) {
          previousMillis = 0;
      }          
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
          } // se o estado do botao foi alterado
      } // controle deboucing botao        

      if (fltTemperatura < float(config["temp"])) {
          digitalWrite(pinRele, HIGH);
          estadoRele = true;
      } else {
          digitalWrite(pinRele, LOW);
          estadoRele = false;
      }             
      
      if (currentMillis - previousMillis >= long(config["interval"]))
      {
        
          // Read temperature as Celsius (the default)
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
          // Read Humidity
          float newH = dht.readHumidity();
          // if humidity read failed, don't change h value
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
          
          //snprintf(msg, MSG_BUFFER_SIZE, "{\"temp\": %.2f, \"hum\": %.2f, \"temp_config\": %.2f}", fltTemperatura, fltHumidade, float(param["temp"]));
          snprintf(msg, MSG_BUFFER_SIZE, "{\"temp\": %.2f, \"hum\": %.2f, \"temp_config\": %.2f, \"qtd_boot\": %s, \"topic_subscribe\": \"%s\", \"estadoRele\": %s }", fltTemperatura, fltHumidade, float(config["temp"]), String(config["qtd_boot"]).c_str(), String(config["topic_subscribe"]).c_str(), String(estadoRele));

          //if (currentMillis - previousMillis >= interval)
          //{

          #if debug == 1    
              Serial.print("client: ");
              Serial.println(client != 0);
          #endif          
          if (client != 0) {
              #if debug == 1    
                  Serial.print("client->connected(): ");
                  Serial.println(client->connected());
              #endif   
            
              if (!client->connected())
              {
                  reconnect_mqtt(strPubMsgErro);
              }
              client->loop();          
    
              client->publish(config["topic"], msg);

              #if debug == 1    
                  Serial.println(msg);
              #endif
              
          } // se o cliente do mqtt esta conectado              
          
        //}

          // save the last time you updated the DHT values
          previousMillis = currentMillis;            
      } //se entreo no intervalo de tempo programado (interval)
    
      estadoBotaoAnt = estadoBotao;      
    } //se o modo de operacao = 1 modo wifi normal
}
