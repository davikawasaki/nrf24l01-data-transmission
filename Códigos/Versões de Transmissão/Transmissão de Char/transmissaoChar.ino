/*
* Getting Started example sketch for nRF24L01+ radios
* This is an example of how to send data from one node to another using data structures
* Updated: Dec 2014 by TMRh20
*/

#include <SPI.h>
#include "nRF24.h"

byte addresses[][6] = {"1Node","2Node"};


/****************** User Config ***************************/
/***      Set this radio as radio number 0 or 1         ***/
bool radioNumber = 1;

/* Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 7 & 8 */
RF24 radio(7,8);
/**********************************************************/


// Used to control whether this node is sending or receiving
bool role = 0;

// Total error/sucess payloads
float totalPayloads = 0, errorPayloads = 0; 

/**
* Create a data structure for transmitting and receiving data
* This allows many variables to be easily sent and received in a single transmission
* See http://www.cplusplus.com/doc/tutorial/structures/
*/
struct dataStruct{
  unsigned long _micros;
  char value;
}myData;

void setup() {

  Serial.begin(115200);
  Serial.println(F("RF24/examples/GettingStarted_HandlingData"));
  Serial.println(F("*** Digite 'T' para comecar transmitindo para o outro no"));
  
  radio.begin();

  // Set the PA Level low to prevent power supply related issues since this is a
 // getting_started sketch, and the likelihood of close proximity of the devices. RF24_PA_MAX is default.
  radio.setPALevel(RF24_PA_LOW);
  
  // Open a writing and reading pipe on each radio, with opposite addresses
  if(radioNumber){
    radio.openWritingPipe(addresses[1]);
    radio.openReadingPipe(1,addresses[0]);
  }else{
    radio.openWritingPipe(addresses[0]);
    radio.openReadingPipe(1,addresses[1]);
  }
  
  myData.value = 'A';
  // Start the radio listening for data
  radio.startListening();
}




void loop() {
  
  
/****************** Ping Out Role ***************************/  
if (role == 1)  {
    
    radio.stopListening();                                    // First, stop listening so we can talk.
    
    
    Serial.print(F("Enviando..."));

    myData._micros = micros();
     if (!radio.write( &myData, sizeof(myData) )){
       Serial.println(" O envio falhou!");
       errorPayloads++;
       totalPayloads++;
     }
        
    radio.startListening();                                    // Now, continue listening
    
    unsigned long started_waiting_at = micros();               // Set up a timeout period, get the current microseconds
    boolean timeout = false;                                   // Set up a variable to indicate if a response was received or not
    
    while ( ! radio.available() ){                             // While nothing is received
      if (micros() - started_waiting_at > 200000 ){            // If waited longer than 200ms, indicate timeout and exit while loop
          timeout = true;
          break;
      }      
    }
        
    if ( timeout ){                                             // Describe the results
        Serial.println(F("Falha na transmissao, tempo de resposta esgotado!"));
        Serial.println(F("-----------------------------------------------------"));
        Serial.print(F("  Total de payloads: "));
        Serial.println(totalPayloads);
        Serial.print(F("  Total de payloads perdidos: "));
        Serial.println(errorPayloads);
        Serial.print(F("  Porcentagem de pacotes perdidos: "));
        Serial.println((errorPayloads/totalPayloads)*100);
        Serial.println(F("-----------------------------------------------------"));
    }else{
                                                                // Grab the response, compare, and send to debugging spew
        radio.read( &myData, sizeof(myData) );
        unsigned long time = micros();
        
        // Spew it
        Serial.print(F("Enviado "));
        Serial.print(time);
        Serial.print(F(", Recebeu resposta "));
        Serial.print(myData._micros);
        Serial.print(F(", Delay de retorno "));
        Serial.print(time-myData._micros);
        Serial.print(F(" Valor da letra "));
        Serial.println(myData.value);
        totalPayloads++;
        Serial.println(F("-----------------------------------------------------"));
        Serial.print(F(" Total de payloads: "));
        Serial.println(totalPayloads);
        Serial.print(F(" Total de payloads perdidos: "));
        Serial.println(errorPayloads);
        Serial.print(F(" Porcentagem de pacotes perdidos: "));
        Serial.println((errorPayloads/totalPayloads)*100);
        Serial.println(F("-----------------------------------------------------"));
    }

    // Try again 1s later
    delay(1000);
  }



/****************** Pong Back Role ***************************/

  if ( role == 0 )
  {
    
    if( radio.available()){
                                                           // Variable for the received timestamp
      while (radio.available()) {                          // While there is data ready
        radio.read( &myData, sizeof(myData) );             // Get the payload
      }
     
      radio.stopListening();                               // First, stop listening so we can talk  
      myData.value += 1;                                   // Increment the float value
      radio.write( &myData, sizeof(myData) );              // Send the final one back.      
      radio.startListening();                              // Now, resume listening so we catch the next packets.     
      Serial.print(F("Enviada a resposta "));
      Serial.print(myData._micros);  
      Serial.print(F(" : "));
      Serial.println(myData.value);
      totalPayloads++;
   }
 }




/****************** Change Roles via Serial Commands ***************************/

  if ( Serial.available() )
  {
    char c = toupper(Serial.read());
    if ( c == 'T' && role == 0 ){      
      Serial.print(F("*** MUDANDO PARA PAPEL DE TRANSMISSOR -- DIGITE 'R' PARA MUDAR"));
      role = 1;                  // Become the primary transmitter (ping out)
    
   }else
    if ( c == 'R' && role == 1 ){
      Serial.println(F("*** MUDANDO PARA PAPEL DE RECEPTOR -- DIGITE 'T' PARA MUDAR"));      
       role = 0;                // Become the primary receiver (pong back)
       radio.startListening();
       
    }
  }


} // Loop
