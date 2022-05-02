void setDateTime() {
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

void execute_operacao(String chave, int valor) {
  if (String("estadoRele") == chave) {
    #if debug == 1
      Serial.println("Alterando relé para: ");
      Serial.print(valor);
      Serial.println("");
    #endif
    switch (valor) {
      case 1: {
        ligar_rele();
        break;
      }
      case 0: {
        desligar_rele();
        break;
      }
      default:
        break;
    }
  } else if (String("temp") == chave) {
      #if debug == 1
        Serial.println("Alterando temperatura: ");
        Serial.print(valor);
        Serial.println("");
      #endif    
      config["temp"] = valor;
      updateConfig(config);
  } else if (String("interval_mqtt") == chave) {
      #if debug == 1
        Serial.println("Alterando intervalo: ");
        Serial.print(valor);
        Serial.println("");
      #endif        
      config["interval_mqtt"] = valor;
      updateConfig(config);
  } else if (String("reboot") == chave && valor == 1) {
      #if debug == 1
        Serial.println("Reinstart..... ");
        Serial.println("");
      #endif        
      ESP.restart();
  }
}

void callback(char *topic, byte *payload, unsigned int length)
{
  #if debug == 1
      Serial.print("Message arrived [");
      Serial.print(topic);
      Serial.print("] ");
  #endif
  StaticJsonDocument<256> doc;
  deserializeJson(doc, (const byte*)payload, length);
  
  strPubModo = String(doc["modo"]);
  String device = doc["device"];

  config["modo_operacao"] = strPubModo;
  updateConfig(config);
  
  serializeJsonPretty(doc, Serial);
  execute_operacao(device, int(doc["value"]));
  publish_msg_mqtt();
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
  client->setBufferSize(512);
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

  #if debug == 1
      Serial.println("inicio funcao connectedWeb");
  #endif

  if (clienteWeb.connect("google.com", 80) == true) {
    #if debug == 1
        Serial.println("fim funcao connectedWeb");
    #endif

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

String build_msg_mqtt() {
    DynamicJsonDocument msgJson(1024);
    char msg[MSG_BUFFER_SIZE];

    JsonObject temp  = msgJson.createNestedObject("temp");
    temp["valor"] = fltTemperatura;
    temp["tipo"] = "M";
    JsonObject hum  = msgJson.createNestedObject("hum");
    hum["valor"] = fltHumidade;
    hum["tipo"] = "M";    
    JsonObject temp_config  = msgJson.createNestedObject("temp_config");
    temp_config["valor"] = float(config["temp"]);
    temp_config["tipo"] = "C";    
    JsonObject qtd_boot  = msgJson.createNestedObject("qtd_boot");
    qtd_boot["valor"] = String(config["qtd_boot"]);
    qtd_boot["tipo"] = "C";   
    JsonObject topic_subscribe  = msgJson.createNestedObject("topic_subscribe");
    topic_subscribe["valor"] = String(config["topic_subscribe"]);
    topic_subscribe["tipo"] = "C";  
    JsonObject estadoRele  = msgJson.createNestedObject("estadoRele");
    estadoRele["valor"] = String(digitalRead(pinRele));
    estadoRele["tipo"] = "C";  
    //msgJson["estadoRele"] = String(digitalRead(pinRele));
    JsonObject modo_operacao  = msgJson.createNestedObject("modo_operacao");
    modo_operacao["valor"] = String(config["modo_operacao"]);
    modo_operacao["tipo"] = "C";  
    //msgJson["modo_operacao"] = String(config["modo_operacao"]);
    JsonObject nome_dispositivo  = msgJson.createNestedObject("nome_dispositivo");
    nome_dispositivo["valor"] = String(config["nome_dispositivo"]);
    nome_dispositivo["tipo"] = "C";      
    //msgJson["nome_dispositivo"] = String(config["nome_dispositivo"]);
    JsonObject hash_  = msgJson.createNestedObject("hash");
    hash_["valor"] = strPubHashTopic;
    hash_["tipo"] = "C";  
    //msgJson["hash"] = strPubHashTopic;
    serializeJson(msgJson, msg);  
    return msg;
  
}


void publish_msg_mqtt() {
    char msg[MSG_BUFFER_SIZE];
    build_msg_mqtt().toCharArray(msg, MSG_BUFFER_SIZE);    
    if (client != 0) {
      client->publish(config["topic"], msg);
      #if debug == 1
          Serial.println(msg);
      #endif
    } 
}
   
               
