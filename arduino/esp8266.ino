/*********
  Rui Santos
  Complete project details at https://randomnerdtutorials.com/esp8266-dht11dht22-temperature-and-humidity-web-server-with-arduino-ide/
*********/

// Import required libraries
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <PubSubClient.h>
#include <time.h>
#include <TZ.h>
#include <FS.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <CertStoreBearSSL.h>

size_t capacity;
DynamicJsonDocument config(2048);
unsigned int operation_mode = 0; //0 - AP, 1 - NORMAL
// A single, global CertStore which can be used by all connections.
// Needs to stay live the entire time any of the WiFiClientBearSSLs
// are present.
BearSSL::CertStore certStore;

//WiFiClientSecure espClient;
PubSubClient *client;
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (500)
char msg[MSG_BUFFER_SIZE];
#define DHTPIN 4 // Digital pin connected to the DHT sensor
// Uncomment the type of sensor in use:
#define DHTTYPE DHT11 // DHT 11
//#define DHTTYPE    DHT22     // DHT 22 (AM2302)
//#define DHTTYPE    DHT21     // DHT 21 (AM2301)

DHT dht(DHTPIN, DHTTYPE);

// current temperature & humidity, updated in loop()
float t = 0.0;
float h = 0.0;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
unsigned long previousMillis = 0; // will store last time DHT was updated

// Updates DHT readings every 10 seconds
const long interval = 10000;
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <style>
    html {
     font-family: Arial;
     display: inline-block;
     margin: 0px auto;
     text-align: center;
    }
    h2 { font-size: 3.0rem; }
    p { font-size: 3.0rem; }
    .units { font-size: 1.2rem; }
    .dht-labels{
      font-size: 1.5rem;
      vertical-align:middle;
      padding-bottom: 15px;
    }
  </style>
</head>
<body>
  <h2>ESP8266 DHT Server</h2>
  <p>
    <i class="fas fa-thermometer-half" style="color:#059e8a;"></i> 
    <span class="dht-labels">Temperature</span> 
    <span id="temperature">%TEMPERATURE%</span>
    <sup class="units">&deg;C</sup>
  </p>
  <p>
    <i class="fas fa-tint" style="color:#00add6;"></i> 
    <span class="dht-labels">Humidity</span>
    <span id="humidity">%HUMIDITY%</span>
    <sup class="units">%</sup>
  </p>
  <p>
    <i class="fas fa-wifi" style="color:#00add6;"></i> 
    <span class="dht-labels">Broker:</span>
    <span id="mqtt">%MQTT%</span>
    <sup class="units"></sup>
  </p>
</body>
<script>
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("temperature").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/temperature", true);
  xhttp.send();
}, 10000 ) ;
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      console.log(this.responseText);
      document.getElementById("mqtt").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/mqtt", true);
  xhttp.send();
}, 10000 ) ;
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("humidity").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/humidity", true);
  xhttp.send();
}, 10000 ) ;
</script>
</html>)rawliteral";

// Replaces placeholder with DHT values
String processor(const String &var)
{
  //Serial.println(var);
  if (var == "TEMPERATURE")
  {
    return String(t);
  }
  else if (var == "HUMIDITY")
  {
    return String(h);
  }
  else if (var == "MQTT")
  {
    return String(msg);
  }
  else if (var == "JSON_STRING") {
    String json_string;
    serializeJsonPretty(config, json_string);
    return json_string;
  }
  return String();
}

void write_default_config(File &configFile) {
  configFile = LittleFS.open("/config.json", "w");
  const char default_config[] = R"rawliteral(
  {
    "wifi-ssid": "",
    "wifi-key": "",
    "topic": "device/sensor/",
    "topic_subscribe": "device/sensor//cmd",
    "mqtt_server": "",
    "mqtt_server_port": "",
    "mqtt_tag1": "device",
    "mqtt_tag2": "local",
    "mqtt_client_id": "",
    "mqtt_user": "",
    "mqtt_password": "",
    "soft-ap": "ESP-123456"
  })rawliteral";
  Serial.println("Failed to open config file");
  Serial.println("Loading default config file");
  configFile.print(default_config);
  delay(5);
  configFile.close();
}

boolean loadConfig(DynamicJsonDocument &doc, size_t &size) {
  File configFile = LittleFS.open("/config.json", "r");
  size = configFile.size();
  if (size > 2048) {
    Serial.println("Config file size is too large");
    return false;
  }
  auto error = deserializeJson(doc, configFile);
  serializeJsonPretty(doc, Serial);
  if (error || !doc.containsKey("soft-ap")) {
    configFile.close();
    write_default_config(configFile);
    configFile = LittleFS.open("/config.json", "r");
    auto error = deserializeJson(doc, configFile);
  }
  configFile.close();
  return true;
}

bool saveConfig(DynamicJsonDocument &doc, String novo) {
  auto error = deserializeJson(doc, novo);
  if (!error) {
    File configFile = LittleFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("Failed to open config file for writing");
      return false;
    }
    size_t val = serializeJson(doc, configFile);
    configFile.close();
    Serial.println("Reniciando");
    ESP.restart();
    return true;
  }
  return false;

}

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

void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
  if (!index) {
    Serial.printf("Update Start: %s\n", filename.c_str());
    Update.runAsync(true);
    if (!Update.begin((ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000)) {
      Update.printError(Serial);
    }
  }
  if (!Update.hasError()) {
    if (Update.write(data, len) != len) {
      Update.printError(Serial);
    }
  }
  if (final) {
    if (Update.end(true)) {
      Serial.printf("Update Success: %uB\n", index + len);
    } else {
      Update.printError(Serial);
    }
  }
}

void handleBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
  if (!index) {
    Serial.printf("BodyStart: %u B\n", total);
  }
  for (size_t i = 0; i < len; i++) {
    Serial.write(data[i]);
  }
  if (index + len == total) {
    Serial.printf("BodyEnd: %u B\n", total);
  }
}

void handleConfig(AsyncWebServerRequest * request) {
  int params = request->params();
  String message;
  Serial.printf("%d params sent in\n", params);
  for (int i = 0; i < params; i++)
  {
    AsyncWebParameter *p = request->getParam(i);
    if (p->isFile())
    {
      Serial.printf("_FILE[%s]: %s, size: %u", p->name().c_str(), p->value().c_str(), p->size());
    }
    else if (p->isPost())
    {
      Serial.printf("_POST[%s]: %s", p->name().c_str(), p->value().c_str());
      saveConfig(config, p->value().c_str());

    }
    else
    {
      Serial.printf("_GET[%s]: %s", p->name().c_str(), p->value().c_str());
    }
  }
  if (request->hasParam("textarea", true))
  {
    message = request->getParam("textarea", true)->value();
  }
  else
  {
    message = "not specified";
  }
  request->send_P(200, "text/plain", "OK");
}

void setup()
{
  // Serial port for debugging purposes
  Serial.begin(115200);
  Serial.println("Inicio");
  dht.begin();
  LittleFS.begin();
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

    // Route for root / web page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest * request)
    {
      request->send_P(200, "text/html", index_html, processor);
    });
    server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest * request)
    {
      request->send_P(200, "text/plain", String(t).c_str());
    });
    server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest * request)
    {
      request->send_P(200, "text/plain", String(h).c_str());
    });
    server.on("/mqtt", HTTP_GET, [](AsyncWebServerRequest * request)
    {
      request->send_P(200, "text/plain", String(msg).c_str());
    });

  }
  server.on("/config", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/html", "<form method='POST' action='/config' enctype='x-www-form-urlencoded'><textarea name='textarea' rows='10' cols='50'>%JSON_STRING%</textarea><input type='submit' value='Salvar'></form>", processor);
  });
  server.on("/config", HTTP_POST, handleConfig);
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
      unsigned long now = millis();
      unsigned long currentMillis = millis();
      if (currentMillis - previousMillis >= interval)
      {
        // save the last time you updated the DHT values
        previousMillis = currentMillis;
        // Read temperature as Celsius (the default)
        float newT = dht.readTemperature();

        if (isnan(newT))
        {
          Serial.println("Failed to read from DHT sensor!");
        }
        else
        {
          t = newT;
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
          h = newH;
        }
        snprintf(msg, MSG_BUFFER_SIZE, "{\"temp\":%.2f, \"hum\": %.f}", t, h);
        if (now - lastMsg > 2000)
        {
          lastMsg = now;
          Serial.println(msg);
          client->publish(config["topic"], msg);
        }
      }
    }
  }
}