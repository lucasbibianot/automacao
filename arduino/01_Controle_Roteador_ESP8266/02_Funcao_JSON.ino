void write_default_config(File &configFile) {
  configFile = LittleFS.open("/config.json", "w");
  const char default_config[] = R"rawliteral(
  {
    "wifi_ssid": "",
    "wifi_key": "",
    "topic": "device/sensor/",
    "topic_subscribe": "cmd/sensor/",
    "mqtt_server": "",
    "mqtt_server_port": "",
    "mqtt_tag1": "device",
    "mqtt_tag2": "local",
    "mqtt_client_id": "",
    "mqtt_user": "",
    "mqtt_password": "",
    "soft_ap": "ESP-123456",
    "temp": "0",
    "qtd_boot": "0",
    "interval" : "5000",
    "interval_mqtt": "30000",
    "usar_display": "1",
    "modo_operacao": "a",
    "nome_dispositivo": "",
    "site_check": "google.com"
  })rawliteral";
  #if debug == 1
    Serial.println("Failed to open config file");
    Serial.println("Loading default config file");
  #endif
  configFile.print(default_config);
  delay(5);
  configFile.close();
}

bool loadConfig(DynamicJsonDocument &doc) {
  File configFile = LittleFS.open("/config.json", "r");
  if (!configFile) {
      configFile = LittleFS.open("/config.json", "w+");
      configFile.close();
      File configFile = LittleFS.open("/config.json", "r");
  }
  size_t size = configFile.size();
  if (size > 2048) {
      #if debug == 1
          Serial.println("Config file size is too large");
      #endif
      strPubMsgErro = "File is large";
      return false;
  }
  strPubMsgErro = "";
  auto error = deserializeJson(doc, configFile);
  serializeJsonPretty(doc, Serial);
  if (error || !doc.containsKey("soft_ap") || !doc.containsKey("nome_dispositivo") || !doc.containsKey("site_check")) {
  //if (error || !doc.containsKey("soft_ap")) {
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
        #if debug == 1
            Serial.println("Failed to open config file for writing");
            strPubMsgErro = "Failed file w";
        #endif
        return false;
      }
      size_t val = serializeJson(doc, configFile);
      configFile.close();
      strPubMsgErro = "";
      #if debug == 1
          Serial.println("Reniciando");
      #endif
      ESP.restart();
      return true;
  }
  return false;
}

bool updateConfig(DynamicJsonDocument &doc) {
  File configFile = LittleFS.open("/config.json", "w");
  if (!configFile) {
    #if debug == 1
        Serial.println("Failed to open config file for writing");
    #endif
    strPubMsgErro = "Failed file w";
    return false;
  }
  size_t val = serializeJson(doc, configFile);
  configFile.close();
  strPubMsgErro = "";
  return true;
}

String build_msg_json() {
    DynamicJsonDocument msgJson(1024);
    char msg[MSG_BUFFER_SIZE];

    JsonObject temp  = msgJson.createNestedObject("conectado");
    temp["valor"] = String(bPubConnectWeb ? "Sim" : "N&atilde;o");
    temp["tipo"] = "M";
    JsonObject temp_config  = msgJson.createNestedObject("tempo_threshold");
    temp_config["valor"] = float(config["temp"]);
    temp_config["tipo"] = "M";    
    JsonObject qtd_boot  = msgJson.createNestedObject("qtd_boot");
    qtd_boot["valor"] = int(config["qtd_boot"]);
    qtd_boot["tipo"] = "M";   
    JsonObject topic_subscribe  = msgJson.createNestedObject("topic_subscribe");
    topic_subscribe["valor"] = String(config["topic_subscribe"]);
    topic_subscribe["tipo"] = "C";  
    JsonObject estadoRele  = msgJson.createNestedObject("estadoRele");
    estadoRele["valor"] = int(digitalRead(pinRele)) == 1 ? "NF" : "NA";
    estadoRele["tipo"] = "M";  
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
    JsonObject site_check  = msgJson.createNestedObject("site_check");
    site_check["valor"] = String(config["site_check"]);
    site_check["tipo"] = "C";       
    //msgJson["hash"] = strPubHashTopic;
    serializeJson(msgJson, msg);  
    return msg;
  
}
