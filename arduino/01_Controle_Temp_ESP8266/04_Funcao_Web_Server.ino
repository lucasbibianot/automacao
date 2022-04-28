String processor(const String &var)
{
  if (var == "TEMPERATURE")
  {
    return String(fltTemperatura);
  }
  else if (var == "HUMIDITY")
  {
    return String(fltHumidade);
  }
  else if (var == "TEMP_CONFIG")
  {
    return String(config["temp"]);
  }
  else if (var == "QTD_BOOT")
  {
    return String(config["qtd_boot"]);
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

String read_config() {
    String json_string;
    serializeJsonPretty(config, json_string);
    return json_string;
}

void handleConfig(AsyncWebServerRequest * request) {

    config["wifi_ssid"] = request->getParam("input_wifi_ssid", true)->value();
    config["wifi_key"] = request->getParam("input_wifi_key", true)->value();
    config["topic"] = request->getParam("input_topic", true)->value();
    config["topic_subscribe"] = request->getParam("input_topic_subscribe", true)->value();
    config["mqtt_server"] = request->getParam("input_mqtt_server", true)->value();
    config["mqtt_server_port"] = request->getParam("input_mqtt_server_port", true)->value();
    config["mqtt_tag1"] = request->getParam("input_mqtt_tag1", true)->value();
    config["mqtt_tag2"] = request->getParam("input_mqtt_tag2", true)->value();
    config["mqtt_client_id"] = request->getParam("input_mqtt_client_id", true)->value();
    config["mqtt_user"] = request->getParam("input_mqtt_user", true)->value();
    config["mqtt_password"] = request->getParam("input_mqtt_password", true)->value();
    config["soft_ap"] = request->getParam("input_soft_ap", true)->value();
    config["temp"] = request->getParam("input_temp", true)->value();
    config["qtd_boot"] = request->getParam("input_qtd_boot", true)->value();
    config["interval"] = request->getParam("input_interval", true)->value();
    config["interval_mqtt"] = request->getParam("input_interval_mqtt", true)->value();
    config["usar_display"] = request->getParam("input_usar_display", true)->value();    
    config["modo_operacao"] = request->getParam("input_modo_operacao", true)->value();
    config["nome_dispositivo"] = request->getParam("input_nome_dispositivo", true)->value();

    updateConfig(config);

    //const bool retorno = saveConfig(config, p->value().c_str());

    ESP.restart();

    //request->send_P(200, "text/plain", "OK");
}

void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
}

void rotas_web_modo_1() {
    // Route for root / web page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest * request)
    {
        request->send_P(200, "text/html", index_html, processor);
    });

    server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest * request)
    {
        request->send_P(200, "text/plain", String(fltTemperatura).c_str());
    });

    server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest * request)
    {
        request->send_P(200, "text/plain", String(fltHumidade).c_str());
    });

    server.on("/mqtt", HTTP_GET, [](AsyncWebServerRequest * request)
    {
        request->send_P(200, "text/plain", String(msg).c_str());
    });

    server.on("/temp_config", HTTP_GET, [](AsyncWebServerRequest * request)
    {
        //request->send_P(200, "text/plain", String(param["temp"]).c_str());
        request->send_P(200, "text/plain", String(config["temp"]).c_str());
    });

    server.on("/qtd_boot", HTTP_GET, [](AsyncWebServerRequest * request)
    {
        request->send_P(200, "text/plain", String(config["qtd_boot"]).c_str());
        //request->send_P(200, "text/plain", String(param["qtd_boot"]).c_str());
    });

    server.on("/get", HTTP_GET, [] (AsyncWebServerRequest * request) {
        String inputMessage;
        String inputParam;
        // GET input1 value on <ESP_IP>/get?input1=<inputMessage>
        if (request->hasParam(PARAM_INPUT_CONFIGTEMP)) {
            inputMessage = request->getParam(PARAM_INPUT_CONFIGTEMP)->value();
            inputParam = PARAM_INPUT_CONFIGTEMP;
            config["temp"] = inputMessage;
            config["qtd_boot"] = "0";
            updateConfig(config);
        }
        else {
            inputMessage = "No message sent";
            inputParam = "none";
        }

        request->send(200, "text/html", "HTTP GET request sent to your ESP on input field ("
                                       + inputParam + ") with value: " + inputMessage +
                                       "<br><a href=\"/\">Return to Home Page</a>");

    });

 }

 void rotas_web_default(){
    server.on("/read_config", HTTP_GET, [](AsyncWebServerRequest * request)
    {
        request->send_P(200, "text/plain", String(read_config()).c_str());
    });

    server.on("/config", HTTP_GET, [](AsyncWebServerRequest * request) {
        request->send_P(200, "text/html", config_html, processor);
    });
    server.on("/config", HTTP_POST, handleConfig);
    server.onNotFound(notFound);
 }
