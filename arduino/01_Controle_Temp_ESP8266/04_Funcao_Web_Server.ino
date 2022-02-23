

// Replaces placeholder with DHT values
String processor(const String &var)
{
  //Serial.println(var);
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
    return String(param["temp"]);
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
        request->send_P(200, "text/plain", String(param["temp"]).c_str());
    });    

    server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
        String inputMessage;
        String inputParam;
        // GET input1 value on <ESP_IP>/get?input1=<inputMessage>
        if (request->hasParam(PARAM_INPUT_CONFIGTEMP)) {
            inputMessage = request->getParam(PARAM_INPUT_CONFIGTEMP)->value();
            inputParam = PARAM_INPUT_CONFIGTEMP;
    
            String strParam = "{'temp': '" + inputMessage + "'}";
            //snprintf(msg, MSG_BUFFER_SIZE, "{\"temp\":%.2f}", inputMessage);
            saveParam(param, strParam);
        }
        else {
            inputMessage = "No message sent";
            inputParam = "none";
        }
        Serial.println(inputMessage);
        
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