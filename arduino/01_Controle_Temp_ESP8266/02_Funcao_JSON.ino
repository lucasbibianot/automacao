
void write_default_param(File &paramFile) {
    paramFile = LittleFS.open("/param.json", "w");
    const char default_param[] = R"rawliteral(
    {
      "temp": "0",
      "qtd_boot": "0"
    })rawliteral";

    #if debug == 1
        Serial.println("Failed to open param file");
        Serial.println("Loading default param file");
    #endif
        
    paramFile.print(default_param);
    delay(5);
    paramFile.close();
}

void write_default_config(File &configFile) {
    //configFile = LittleFS.open("/config.json", "w");
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
      "soft-ap": "ESP-123456",
      "temp": "0",
      "qtd_boot": "0"      
    })rawliteral";

    #if debug == 1
      Serial.println("Failed to open config file");
      Serial.println("Loading default config file");
    #endif
      
    configFile.print(default_config);
    delay(5);
    configFile.close();
}

bool loadParam(DynamicJsonDocument &docParam) {
    File paramFile = LittleFS.open("/param.json", "r");
    if (!paramFile) {
        paramFile = LittleFS.open("/param.json", "w+");  
        //paramFile = LittleFS.open("/param.json", "w");
        paramFile.close();
        File paramFile = LittleFS.open("/param.json", "r");
    }
    size_t size = paramFile.size();
    if (size > 1024) {
        #if debug == 1
            Serial.println("Param file size is too large");
        #endif
            
        return false;
    }
  
    auto error = deserializeJson(docParam, paramFile);
    serializeJsonPretty(docParam, Serial);
    if (error || !docParam.containsKey("temp")) {
        paramFile.close();
        write_default_param(paramFile);
        paramFile = LittleFS.open("/param.json", "r");
        auto error = deserializeJson(docParam, paramFile);
    }
    paramFile.close();
    return true;  
}

bool loadConfig(DynamicJsonDocument &doc) {
    File configFile = LittleFS.open("/config.json", "r");
    if (!configFile) {
        configFile = LittleFS.open("/config.json", "w+");  
        //configFile = LittleFS.open("/config.json", "w");  
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
    if (error || !doc.containsKey("soft-ap")) {
        configFile.close();
        write_default_config(configFile);
        configFile = LittleFS.open("/config.json", "r");
        auto error = deserializeJson(doc, configFile);
    }
    configFile.close();
    return true;
}

bool saveParam(DynamicJsonDocument &docParam, String novo) {
    auto error = deserializeJson(docParam, novo);
    if (!error) {
        File paramFile = LittleFS.open("/param.json", "w"); 
        if (!paramFile) {
          
          #if debug == 1
              Serial.println("Failed to open param file for writing");
          #endif
              
          return false;
        }
        size_t val = serializeJson(docParam, paramFile);
        paramFile.close();
        return true;
    }
    return false;
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
