
void write_default_param(File &paramFile) {
    paramFile = LittleFS.open("/param.json", "w");
    const char default_param[] = R"rawliteral(
    {
      "temp": ""
    })rawliteral";
    
    Serial.println("Failed to open param file");
    Serial.println("Loading default param file");
    paramFile.print(default_param);
    delay(5);
    paramFile.close();
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

boolean loadParam(DynamicJsonDocument &doc, size_t &size) {
    File paramFile = LittleFS.open("/param.json", "r");
    if (!paramFile) {
        paramFile = LittleFS.open("/param.json", "w+");  
        paramFile.close();
        File paramFile = LittleFS.open("/param.json", "r");
    }
    size = paramFile.size();
    if (size > 2048) {
        Serial.println("Param file size is too large");
        return false;
    }
  
    auto error = deserializeJson(doc, paramFile);
    serializeJsonPretty(doc, Serial);
    if (error || !doc.containsKey("temp")) {
        paramFile.close();
        write_default_param(paramFile);
        paramFile = LittleFS.open("/param.json", "r");
        auto error = deserializeJson(doc, paramFile);
    }
    paramFile.close();
    return true;  
}

boolean loadConfig(DynamicJsonDocument &doc, size_t &size) {
    File configFile = LittleFS.open("/config.json", "r");
    if (!configFile) {
        configFile = LittleFS.open("/config.json", "w+");  
        configFile.close();
        File configFile = LittleFS.open("/config.json", "r");
    }
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

bool saveParam(DynamicJsonDocument &doc, String novo) {
    auto error = deserializeJson(doc, novo);
    if (!error) {
        File paramFile = LittleFS.open("/param.json", "w");
        if (!paramFile) {
          Serial.println("Failed to open param file for writing");
          return false;
        }
        size_t val = serializeJson(doc, paramFile);
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
