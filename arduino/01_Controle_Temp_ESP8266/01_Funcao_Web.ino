void setDateTime()
{
  // You can use your own timezone, but the exact time is not used at all.
  // Only the date is needed for validating the certificates.

  
  configTime(TZ_America_Sao_Paulo, "pool.ntp.org", "time.nist.gov");

  #if debug == 1
      Serial.print("Waiting for NTP time sync: ");
  #endif
      
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2)
  {
    delay(100);

    #if debug == 1
        Serial.print(".");
    #endif
        
    now = time(nullptr);
  }
  
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  //Serial.printf("%s %s", tzname[0], asctime(&timeinfo));
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
    //Serial.println();
  
    /*
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
    */
}

void connect_mqtt(String &strMsgErro) {
          setDateTime();

          
          
          //pinMode(LED_BUILTIN, OUTPUT); // Initialize the LED_BUILTIN pin as an output
          // you can use the insecure mode, when you want to avoid the certificates
          //espclient->setInsecure();
      
          int numCerts = certStore.initCertStore(LittleFS, PSTR("/certs.idx"), PSTR("/certs.ar"));
          //Serial.printf("Number of CA certs read: %d\n", numCerts);
          if (numCerts == 0)
          {
              #if debug == 1
                  Serial.print("No certs found. Did you run certs-from-mozilla.py and upload the LittleFS directory before running?\n");
              #endif
              strPubMsgErro = "No Certs";    
              return; // Can't connect to anything w/o certs!
          } else {
             strPubMsgErro = "";
          }
          
      
          BearSSL::WiFiClientSecure *bear = new BearSSL::WiFiClientSecure();
          // Integrate the cert store with this connection
          bear->setCertStore(&certStore);
      
          client = new PubSubClient(*bear);
          const char* mqq_server = config["mqtt_server"];
          //Serial.printf("MQTT: %s", mqq_server);
          client->setServer(mqq_server, (uint16_t) config["mqtt_server_port"]);
          client->setCallback(callback);

  
}

void reconnect_mqtt(String &strMsgErro)
{
    // Loop until we’re reconnected
    //while (!client->connected())
    //{
      #if debug == 1
          Serial.print("Attempting MQTT connection…");
      #endif
          
      // Attempt to connect
      // Insert your password
      if (client->connect(config["mqtt_client_id"], config["mqtt_user"], config["mqtt_password"]))
      
      {
          #if debug == 1
              Serial.println("connected");
          #endif
              
          client->subscribe(config["topic_subscribe"]);
          strMsgErro = "";
      }
      else
      {
          #if debug == 1
              Serial.print("failed, rc = ");
              Serial.print(client->state());
              Serial.println(" try again in 5 seconds");
          #endif
              
          // Wait 5 seconds before retrying
          //delay(5000);
          strMsgErro = "Client Mqtt Off";
          
      }
    //} fim loop
}

bool connectedWeb(String &strMsgErro) {
    #if debug == 1
        Serial.println("inicio funcao connectedWeb");
    #endif 
    
    if (clienteWeb.connect("google.com", 80) == true) {    
        //Tenta se conectar ao servidor do Google na porta 80 (HTTPS)
        #if debug == 1
            Serial.println("Conseguiu conexão com Servidor Google");
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
