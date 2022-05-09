#include <SPI.h>
#include <printf.h>
#include <RF24.h>

//*************** Controle do RF ***********************
#define radioID 1   //Informar "0" para um dispositivo e "1" para o outro dispositivo
#define tempoParado 1000

#if radioID == 0

  #include <Servo.h>
  #define pinBotao 6
  #define pinBotao2 5
  #define pinServo 9  
  //#define pinBotao 13
  
#endif

#if radioID == 1

  #define pinLed 6
  #define pinPot A0  
  #define pinBotao 5
  
#endif

#if radioID == 2 

  #define pinLed 6
  #define pinBotao2 5
  
#endif


struct estruturaDadosRF
{
   boolean ligando = false;   //Esta variavel será usada para solicitar os dados do outro aparelho. Será útil quando o aparelho solicitante esta sendo ligado, para manter os valores do aparelho que já esta ligado.
   boolean alterado = false;
   boolean botao1 = false;
   boolean botao2 = false;
   int potenciometro1 = 0;
};

typedef struct estruturaDadosRF tipoDadosRF;
tipoDadosRF dadosRF;
tipoDadosRF dadosRecebidos;

boolean transmitido = true;
boolean alterado = false;

unsigned long lngTempoCorrido;

#if radioID == 0
    //RF24 radio(0,2);
    RF24 radio(7,8);
#else 
    RF24 radio(7,8);
#endif

//byte enderecos[][6] = {"1Node","2Node", "3"};
 uint64_t enderecos[6] = {0x7878787878LL,
                        0xB3B4B5B6F1LL,
                        0xB3B4B5B6CDLL,
                        0xB3B4B5B6A3LL,
                        0xB3B4B5B60FLL,
                        0xB3B4B5B605LL
                       };


//*************** Controle do Projeto LOCAL ************
#if radioID == 0
    Servo servo1;
    int angulo1 = 0;        
    boolean botao1Ant = LOW;
    boolean botao1    = LOW;  
    boolean botao2Ant = LOW;
    boolean botao2    = LOW;      
#endif

#if radioID == 1
    int pot1Ant = 0;
    int pot1    = 0;
    boolean botao1Ant = HIGH;
    boolean botao1    = HIGH;      
#endif

#if radioID == 2 
    boolean botao2Ant = HIGH;
    boolean botao2    = HIGH;          
#endif 

void setup() {
  Serial.begin(115200);
  printf_begin();  
  Serial.println("inicio");
  
  //*************** Controle do RF ***********************
  if (!radio.begin()) {
    Serial.println(F("radio hardware is not responding!!"));
    while (1) {} // hold in infinite loop
  }
  radio.setChannel(100);
  radio.setPALevel(RF24_PA_LOW);

  radio.setDataRate(RF24_250KBPS);

  //radio.setPayloadSize(sizeof(tipoDadosRF));
  

  #if radioID == 0
      radio.openWritingPipe(enderecos[0]);
      radio.openReadingPipe(1, enderecos[1]);
      radio.openReadingPipe(2, enderecos[2]);
  #endif
  
  #if radioID == 1
      radio.openWritingPipe(enderecos[1]);
      radio.openReadingPipe(1, enderecos[0]);
  #endif

  #if radioID == 2        
      radio.openWritingPipe(enderecos[2]);
      radio.openReadingPipe(2, enderecos[0]);  
  # endif

  //Solicita os dados do outro aparelho, se ele estiver ligado. Tenta a comunicação por 2 segundos.
  dadosRF.ligando = true;
  radio.stopListening();                                  
  long tempoInicio = millis();
  while ( !radio.write( &dadosRF, sizeof(tipoDadosRF) ) ) {
     if ((millis() - tempoInicio) > 2000) {
        break;
     }  
  }
  dadosRF.ligando = false;
  radio.startListening();  

  //*************** Controle do Projeto LOCAL ************
  #if radioID == 0
      pinMode(pinBotao, INPUT);
      pinMode(pinBotao2, INPUT);
      servo1.attach(pinServo);      
  #endif
  
  #if radioID == 1     
      pinMode(pinLed, OUTPUT);
      pinMode(pinPot, INPUT);
      pinMode(pinBotao, INPUT_PULLUP);
  #endif

  #if radioID == 2
      pinMode(pinLed, OUTPUT);
      pinMode(pinBotao2, INPUT_PULLUP);  
  #endif  

  radio.printPrettyDetails();
}

void loop() {
  //*************** Controle do RF ***********************
  // se houve alteração dos dados, envia para o outro radio

  //Serial.print("alterado: ");
  //Serial.println(alterado);
  //Serial.print("transmitido: ");
  //Serial.println(transmitido);  

  if (alterado || !transmitido) {     
     if ( (millis() - lngTempoCorrido) > tempoParado) {
         radio.stopListening();  
         Serial.println("tentando transmitir");                                      
         transmitido = radio.write( &dadosRF, sizeof(tipoDadosRF) );
         radio.setRetries(3,5);
         if (transmitido) {
            Serial.println("Transmitido - houve alteracao dos dados");
         }
         radio.startListening();  
         alterado = false;

         lngTempoCorrido = millis();
     }
     //Serial.println("houve alteracao dos dados");
     //Serial.println(dadosRF);
  }

  //delay(1000);

  //verifica se esta recebendo mensagem    
  if (!alterado) {  
      if (radio.available()) {     
        Serial.println("radio.available()");                             
         radio.read( &dadosRecebidos, sizeof(tipoDadosRF) );
    
         //verifica se houve solicitação de envio dos dados
         if (dadosRecebidos.ligando) {
            alterado = true;
         } else {
            //if (dadosRecebidos.alterado) {
              dadosRF = dadosRecebidos;
              Serial.println("dados recebidos");
              Serial.print("RECEBIDO dadosRF.botao1: ");
              Serial.println(dadosRF.botao1);
              Serial.print("RECEBIDO dadosRF.botao2: ");
              Serial.println(dadosRF.botao2);              
              Serial.print("RECEBIDO dadosRF.potenciometro1: ");
              Serial.println(dadosRF.potenciometro1);
                         
              //Serial.println(dadosRF);
              //dadosRF.alterado = false;
            //}
         }
      }
  }

  //*************** Controle do Projeto LOCAL ************

  #if (radioID == 0 or radioID == 1)

      botao1 = digitalRead(pinBotao);
      //Serial.print("botao1: ");
      //Serial.println(botao1);
      if (botao1 && (botao1 != botao1Ant)) {
         Serial.print("ANTES dadosRF.botao1: ");
         Serial.println(dadosRF.botao1);    
         dadosRF.botao1 = !dadosRF.botao1;
         alterado = true;  //esta variavel controla o envio dos dados sempre que houver uma alteração
         dadosRF.alterado = true;
         Serial.println("botao pressionado");
         Serial.print("DEPOIS dadosRF.botao1: ");
         Serial.println(dadosRF.botao1);
      }
      botao1Ant = botao1;
      
  #endif

  #if radioID == 0 or  radioID == 2

      botao2 = digitalRead(pinBotao2);

         //Serial.print("botao2: ");
         //Serial.println(botao2); 
      
      if (botao2 && (botao2 != botao2Ant)) {
         Serial.print("ANTES dadosRF.botao2: ");
         Serial.println(dadosRF.botao2);    
         dadosRF.botao2 = !dadosRF.botao2;
         alterado = true;  //esta variavel controla o envio dos dados sempre que houver uma alteração
         dadosRF.alterado = true;
         Serial.println("botao pressionado");
         Serial.print("DEPOIS dadosRF.botao2: ");
         Serial.println(dadosRF.botao2);
      }
      botao2Ant = botao2;
      
  #endif  
  
  #if radioID == 1  
    //Serial.print("entrei aqui ");
    pot1 = analogRead(pinPot);

    if (pot1 >= 0 && pot1 <= 250) {
      pot1 = 0;
    } else if (pot1 >= 251 && pot1 <= 525) {
      pot1 = 45;
    } else if (pot1 >= 525 && pot1 <= 755) {
      pot1 = 135;      
    } else {
      pot1 = 180;      
    }
    
    
    //pot1 = 525;

    //pot1 = map(pot1, 0, 1023, 0, 180);  
    
    if (pot1 != pot1Ant) {
       dadosRF.potenciometro1 = pot1;
       alterado = true;  //esta variavel controla o envio dos dados sempre que houver uma alteração
       dadosRF.alterado = true;
       Serial.print("potenciometro alterado: ");
       Serial.println(dadosRF.potenciometro1);       
    }
    pot1Ant = pot1; 
    
    //Serial.print("dadosRF.botao1: ");
    //Serial.println(dadosRF.botao1);    
    digitalWrite(pinLed, dadosRF.botao1);

  #endif
  
  #if radioID == 0
    //angulo1 = map(dadosRF.potenciometro1, 0, 1023, 0, 180);  
    servo1.write(dadosRF.potenciometro1);  
  #endif

  #if radioID == 2
    digitalWrite(pinLed, dadosRF.botao2);  
  #endif
  

  //delay(30);
}
