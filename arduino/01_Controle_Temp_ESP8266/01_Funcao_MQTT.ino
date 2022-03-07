void setDateTime()
{
  configTime(TZ_America_Sao_Paulo, "pool.ntp.org", "time.nist.gov");
  #if debug == 1
      Serial.print("Waiting for NTP time sync: ");
  #endif
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2)
  {
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
  // Switch on the LED if the first character is present
  if ((char)payload[0] != NULL)
  {
      digitalWrite(LED_BUILTIN, LOW);
      delay(500);
      digitalWrite(LED_BUILTIN, HIGH);
  }
  else
  {
      digitalWrite(LED_BUILTIN, HIGH);
  }
}

void connect_mqtt(String &strMsgErro) {
  setDateTime();
  int numCerts = certStore.initCertStore(LittleFS, PSTR("/certs.idx"), PSTR("/certs.ar"));
  if (numCerts == 0)
  {
      #if debug == 1
          Serial.print("No certs found. Did you run certs-from-mozilla.py and upload the LittleFS directory before running?\n");
      #endif
      strPubMsgErro = "No Certs";
      return;
  } else {
     strPubMsgErro = "";
  }
  BearSSL::WiFiClientSecure *bear = new BearSSL::WiFiClientSecure();
  bear->setCertStore(&certStore);
  client = new PubSubClient(*bear);
  const char* mqq_server = config["mqtt_server"];
  client->setServer(mqq_server, (uint16_t) config["mqtt_server_port"]);
  client->setCallback(callback);
  client->connect(config["mqtt_client_id"], config["mqtt_user"], config["mqtt_password"]);
}

void reconnect_mqtt(String &strMsgErro)
{
  #if debug == 1
      Serial.print("Attempting MQTT connection…");
  #endif
  if (client->connect(config["mqtt_client_id"], config["mqtt_user"], config["mqtt_password"]))
  {
    #if debug == 1
        Serial.println("connected");
    #endif
    client->subscribe(config["topic_subscribe"]);
    #if debug == 1
      Serial.println("Subscribed: ");
    #endif
    strMsgErro = "";
  }
  else
  {
    #if debug == 1
        Serial.print("failed, rc = ");
        Serial.print(client->state());
        Serial.println(" try again in 5 seconds");
    #endif
    strMsgErro = "Client Mqtt Off";
  }
}

bool connectedWeb(String &strMsgErro) {
  if (clienteWeb.connect("google.com", 80) == true) {
    strMsgErro = "";
    return true;
  }
  else {
    #if debug == 1
        Serial.println("Falha na conexão com Servidor Google APIs");
        Serial.println("fim funcao connectedWeb");
    #endif
    strMsgErro = "No Connect Web";
    return false;
  }
}
