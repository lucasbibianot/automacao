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
bool saveConfig2(DynamicJsonDocument &doc);


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

#define debug 0

//Instanciando objetos
DHT dht(pinDHT, typeDHT);
LiquidCrystal_I2C lcd(enderecoLcd, colunasLcd, linhasLcd);

unsigned long lngdebounceBotao;
bool estadoBotao;
bool estadoBotaoAnt = LOW;

int estadoAtual = 1;

// current temperature & humidity, updated in loop()
float fltTemperatura = 0.0;
float fltHumidade = 0.0;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
unsigned long previousMillis = 0; // will store last time DHT was updated

// Updates DHT readings every 5 seconds
const long interval = 5000;

const char* PARAM_INPUT_CONFIGTEMP = "input1";

String LocalIpAdress;

void setDateTime()
{
  // You can use your own timezone, but the exact time is not used at all.
  // Only the date is needed for validating the certificates.
  configTime(TZ_America_Sao_Paulo, "pool.ntp.org", "time.nist.gov");

  #if debug == 1
      Serial.print("Waiting for NTP time sync: ");
  #endif
      
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2)
  {
    delay(100);

    #if debug == 1
        Serial.print(".");
    #endif
        
    now = time(nullptr);
  }
  

  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  //Serial.printf("%s %s", tzname[0], asctime(&timeinfo));
}

void callback(char *topic, byte *payload, unsigned int length)
{
    #if debug == 1
        Serial.print("Message arrived [");
        Serial.print(topic);
        Serial.print("] ");
    #endif
        
    for (int i = 0; i < length; i++)
    {
        #if debug == 1
            Serial.print((char)payload[i]);
        #endif
    }
    //Serial.println();
  
    /*
    // Switch on the LED if the first character is present
    if ((char)payload[0] != NULL)
    {
        digitalWrite(LED_BUILTIN, LOW); // Turn the LED on (Note that LOW is the voltage level
        // but actually the LED is on; this is because
        // it is active low on the ESP-01)
        delay(500);
        digitalWrite(LED_BUILTIN, HIGH); // Turn the LED off by making the voltage HIGH
    }
    else
    {
        digitalWrite(LED_BUILTIN, HIGH); // Turn the LED off by making the voltage HIGH
    }
    */
}

void reconnect()
{
    // Loop until we’re reconnected
    while (!client->connected())
    {
        #if debug == 1
            Serial.print("Attempting MQTT connection…");
        #endif
            
        // Attempt to connect
        // Insert your password
        if (client->connect(config["mqtt_client_id"], config["mqtt_user"], config["mqtt_password"]))
        {
            #if debug == 1
                Serial.println("connected");
            #endif
                
            client->subscribe(config["topic_subscribe"]);
        }
        else
        {
            #if debug == 1
                Serial.print("failed, rc = ");
                Serial.print(client->state());
                Serial.println(" try again in 5 seconds");
            #endif
                
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }
}

void setup()
{
    // Serial port for debugging purposes

    #if debug == 1
      Serial.begin(115200);
      Serial.println("Inicio");
    #endif
    
    LittleFS.begin();

    //SPIFFS.begin();
    
    dht.begin();
    pinMode(pinRele, OUTPUT);
    pinMode(pinBotao, INPUT);

    //Display
    lcd.init(); //inicia comunicacao com o display
    lcd.backlight(); //liga iluminacao do display
    lcd.clear(); //limpa o display
    lcd.setCursor(0,0); //posiciona o cursor na primeira coluna da linha 1
    lcd.print("Inicio");
    //delay(3000);
    //lcd.clear(); //limpa o display
     
    /*
    if (loadParam(param)) {
        //Serial.println("Carreguei os parametros");

        
        //delay(5000);
        //Serial.println("debug param");

        int intQtdBoot = 0;
        intQtdBoot = int(param["qtd_boot"]) + 1;

        String strQtdBoot = String(intQtdBoot);
        
        //String strParam = "{'temp': '" + String(param["temp"]).c_str() + "','qtd_boot':" + strQtdBoot + "}";
        snprintf(msg, MSG_BUFFER_SIZE, "{\"temp\": %s, \"qtd_boot\": %s}", String(param["temp"]).c_str(), strQtdBoot);
        saveParam(param, msg);    
        
        Serial.print("param.temp: ");
        Serial.println( String(param["temp"]).c_str());
        Serial.print("param.qtd_boot: ");
        Serial.println( String(param["qtd_boot"]).c_str());  
        Serial.print("intQtdBoot: ");
        Serial.println(intQtdBoot);  

              
        
        Serial.print("SETUP msg file json: ");
        Serial.print(msg);  
          
    }
    */
    
    if (loadConfig(config)) {
        #if debug == 1
            Serial.println("Carreguei as configurações");
        #endif

        int intQtdBoot = 0;
        intQtdBoot = int(config["qtd_boot"]) + 1;

        String strQtdBoot = String(intQtdBoot);
        
        /*
        snprintf(msg, MSG_BUFFER_SIZE, "{\"temp\": %s, \"qtd_boot\": %s}", String(param["temp"]).c_str(), strQtdBoot);
        saveParam(param, msg);         
        */

        config["qtd_boot"] = strQtdBoot;
        saveConfig2(config);
    } 
    // Connect to Wi-Fi
    const String ssid = config["wifi-ssid"];
    const String wifi_key = config["wifi-key"];
    //if (ssid == 0) {
    if (ssid == "") {
        const String soft_ap = config["soft-ap"];
        // Iniciar o SoftAP
        WiFi.softAP(soft_ap);
        #if debug == 1
          Serial.println("Iniciar SoftAP");
          Serial.print("IP address: ");
          Serial.println(WiFi.softAPIP());
        #endif
        
    } else {
      operation_mode = 1;
      //Serial.printf("%s", ssid);
      WiFi.begin(ssid, wifi_key);

      #if debug == 1
          Serial.println("Connecting to WiFi");
      #endif
          
      while (WiFi.status() != WL_CONNECTED)
      {
        delay(1000);
        //Serial.println(".");
      }
  
      // Print ESP8266 Local IP Address
      #if debug == 1
          Serial.println(WiFi.localIP());
      #endif
          
      LocalIpAdress = WiFi.localIP().toString();
  
      setDateTime();
      //pinMode(LED_BUILTIN, OUTPUT); // Initialize the LED_BUILTIN pin as an output
      // you can use the insecure mode, when you want to avoid the certificates
      //espclient->setInsecure();
  
      int numCerts = certStore.initCertStore(LittleFS, PSTR("/certs.idx"), PSTR("/certs.ar"));
      //Serial.printf("Number of CA certs read: %d\n", numCerts);
      if (numCerts == 0)
      {
        #if debug == 1
            Serial.print("No certs found. Did you run certs-from-mozilla.py and upload the LittleFS directory before running?\n");
        #endif
            
        return; // Can't connect to anything w/o certs!
      }
  
      BearSSL::WiFiClientSecure *bear = new BearSSL::WiFiClientSecure();
      // Integrate the cert store with this connection
      bear->setCertStore(&certStore);
  
      client = new PubSubClient(*bear);
      const char* mqq_server = config["mqtt_server"];
      //Serial.printf("MQTT: %s", mqq_server);
      client->setServer(mqq_server, (uint16_t) config["mqtt_server_port"]);
      client->setCallback(callback);
  
      rotas_web_modo_1();     
    }
  
    rotas_web_default(); 
    
    // Start server    
    server.begin();
}

void loop()
{
    if (operation_mode == 1) {

      if (client != 0) {
          if (!client->connected())
          {
              reconnect();
          }
          client->loop();

          unsigned long currentMillis = millis();
          
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
                  //contador ++;
                  
                  estadoAtual++;
                  
                  if (estadoAtual > 3) {
                     estadoAtual = 1;
                  }

                  #if debug == 1  
                    Serial.print("estadoAtual: ");
                    Serial.println(estadoAtual);
                  #endif          

                  switch (estadoAtual) {    
                        case 1: {
                            #if debug == 1  
                              Serial.println("case 1 ");
                            #endif
                            display_temp_hum(fltTemperatura, fltHumidade);
                            break;
                        }
                        case 2: {
                              #if debug == 1      
                                  Serial.println("case 2 ");
                              #endif
                              display_ip(LocalIpAdress);
                              break;
                         }  
                         case 3 : {
                              display_temp_config(config["temp"], config["qtd_boot"]);  
                              //display_temp_config(param["temp"], param["qtd_boot"]);  
                              break;
                         }
                  }                    
                  
                  lngdebounceBotao = millis();
              }
          }         

          //if (fltTemperatura < float(param["temp"])) {
          if (fltTemperatura < float(config["temp"])) {
              digitalWrite(pinRele, HIGH);
          } else {
              digitalWrite(pinRele, LOW);
          }             
          
    

          if (currentMillis - previousMillis >= interval)
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

              if (estadoAtual == 1) {
                  display_temp_hum(fltTemperatura, fltHumidade);
              }
              
              //snprintf(msg, MSG_BUFFER_SIZE, "{\"temp\": %.2f, \"hum\": %.2f, \"temp_config\": %.2f}", fltTemperatura, fltHumidade, float(param["temp"]));
              snprintf(msg, MSG_BUFFER_SIZE, "{\"temp\": %.2f, \"hum\": %.2f, \"temp_config\": %.2f}", fltTemperatura, fltHumidade, float(config["temp"]));
      
              //if (currentMillis - previousMillis >= interval)
              //{
              #if debug == 1    
                  Serial.println(msg);
              #endif
                  
              client->publish(config["topic"], msg);
            //}

              // save the last time you updated the DHT values
              previousMillis = currentMillis;            
          }
        
          estadoBotaoAnt = estadoBotao;
      }
    }
}
