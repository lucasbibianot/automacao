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

// Import required libraries Components
#include <DHT.h>
#include <LiquidCrystal_I2C.h>

size_t capacity;
DynamicJsonDocument config(2048);
DynamicJsonDocument param(512);

//Funcoes
boolean loadParam(DynamicJsonDocument &doc, size_t &size);
void handleConfig(AsyncWebServerRequest * request);
void write_default_param(File &paramFile);
void write_default_config(File &configFile);
boolean loadConfig(DynamicJsonDocument &doc, size_t &size);
bool saveParam(DynamicJsonDocument &doc, String novo);
bool saveConfig(DynamicJsonDocument &doc, String novo);
void notFound(AsyncWebServerRequest *request);
void rotas_web_modo_1();
void rotas_web_default();


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

//Instanciando objetos
DHT dht(pinDHT, typeDHT);
LiquidCrystal_I2C lcd(enderecoLcd, colunasLcd, linhasLcd);

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

void setDateTime()
{
  // You can use your own timezone, but the exact time is not used at all.
  // Only the date is needed for validating the certificates.
  configTime(TZ_America_Sao_Paulo, "pool.ntp.org", "time.nist.gov");

  Serial.print("Waiting for NTP time sync: ");
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2)
  {
    delay(100);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println();

  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.printf("%s %s", tzname[0], asctime(&timeinfo));
}

void callback(char *topic, byte *payload, unsigned int length)
{
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (int i = 0; i < length; i++)
    {
        Serial.print((char)payload[i]);
    }
    Serial.println();
  
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
}

void reconnect()
{
    // Loop until we’re reconnected
    while (!client->connected())
    {
        Serial.print("Attempting MQTT connection…");
        // Attempt to connect
        // Insert your password
        if (client->connect(config["mqtt_client_id"], config["mqtt_user"], config["mqtt_password"]))
        {
            Serial.println("connected");
            client->subscribe(config["topic_subscribe"]);
        }
        else
        {
            Serial.print("failed, rc = ");
            Serial.print(client->state());
            Serial.println(" try again in 5 seconds");
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }
}

void setup()
{
    // Serial port for debugging purposes
    Serial.begin(115200);
    Serial.println("Inicio");
    dht.begin();
    LittleFS.begin();
  
    if (loadParam(param, capacity)) {
        Serial.println("Carreguei os parametros");
        Serial.print("param.temp: ");
        Serial.println( String(param["temp"]).c_str());
    }
    
    if (loadConfig(config, capacity)) {
        Serial.println("Carreguei as configurações");
    }
    // Connect to Wi-Fi
    const char *ssid = config["wifi-ssid"];
    const char *wifi_key = config["wifi-key"];
    if (*ssid == 0) {
        const char *soft_ap = config["soft-ap"];
        // Iniciar o SoftAP
        WiFi.softAP(soft_ap);
        Serial.println("Iniciar SoftAP");
        Serial.print("IP address: ");
        Serial.println(WiFi.softAPIP());
    } else {
      operation_mode = 1;
      Serial.printf("%s", ssid);
      WiFi.begin(ssid, wifi_key);
      Serial.println("Connecting to WiFi");
      while (WiFi.status() != WL_CONNECTED)
      {
        delay(1000);
        Serial.println(".");
      }
  
      // Print ESP8266 Local IP Address
      Serial.println(WiFi.localIP());
  
      setDateTime();
      pinMode(LED_BUILTIN, OUTPUT); // Initialize the LED_BUILTIN pin as an output
      // you can use the insecure mode, when you want to avoid the certificates
      //espclient->setInsecure();
  
      int numCerts = certStore.initCertStore(LittleFS, PSTR("/certs.idx"), PSTR("/certs.ar"));
      Serial.printf("Number of CA certs read: %d\n", numCerts);
      if (numCerts == 0)
      {
        Serial.printf("No certs found. Did you run certs-from-mozilla.py and upload the LittleFS directory before running?\n");
        return; // Can't connect to anything w/o certs!
      }
  
      BearSSL::WiFiClientSecure *bear = new BearSSL::WiFiClientSecure();
      // Integrate the cert store with this connection
      bear->setCertStore(&certStore);
  
      client = new PubSubClient(*bear);
      const char* mqq_server = config["mqtt_server"];
      Serial.printf("MQTT: %s", mqq_server);
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
        
          if (currentMillis - previousMillis >= interval)
          {
              // Read temperature as Celsius (the default)
              float newT = dht.readTemperature();
      
              if (isnan(newT))
              {
                  Serial.println("Failed to read from DHT sensor!");
              }
              else
              {
                  fltTemperatura = newT;
                  Serial.print("fltTemperatura: ");
                  Serial.println(fltTemperatura);                  
              }
              // Read Humidity
              float newH = dht.readHumidity();
              // if humidity read failed, don't change h value
              if (isnan(newH))
              {
                  Serial.println("Failed to read from DHT sensor!");
              }
              else
              {
                  fltHumidade = newH;
                  Serial.print("fltHumidade: ");
                  Serial.println(fltHumidade);
              }
              snprintf(msg, MSG_BUFFER_SIZE, "{\"temp\": %.2f, \"hum\": %.2f, \"temp_config\": %.2f}", fltTemperatura, fltHumidade, float(param["temp"]));
      
              //if (currentMillis - previousMillis >= interval)
              //{    
              Serial.println(msg);
              client->publish(config["topic"], msg);
            //}

              // save the last time you updated the DHT values
              previousMillis = currentMillis;            
          }
      }
    }
}
