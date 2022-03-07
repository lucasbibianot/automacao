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

void handleConfig(AsyncWebServerRequest * request) {
  int params = request->params();
  String message;
  for (int i = 0; i < params; i++)
  {
      AsyncWebParameter *p = request->getParam(i);
      if (p->isPost())
      {
        const bool retorno = saveConfig(config, p->value().c_str());
        if (retorno) {
          ESP.restart();
        }
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
    server.on("/config", HTTP_GET, [](AsyncWebServerRequest * request) {
        request->send_P(200, "text/html", "<form method='POST' action='/config' enctype='x-www-form-urlencoded'><textarea name='textarea' rows='10' cols='50'>%JSON_STRING%</textarea><input type='submit' value='Salvar'></form>", processor);
    });
    server.on("/config", HTTP_POST, handleConfig);
    server.onNotFound(notFound);
 }
