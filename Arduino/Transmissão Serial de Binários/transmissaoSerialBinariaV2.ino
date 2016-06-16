/*
* Código para envio de 8 dígitos binários entre dois nRF24L01+ rádios
* Envio de informações por struct
* Última atualização: 07/06/2016
*/

#include <SPI.h>
#include "nRF24.h"

// Determina dois endereços de Pipe para conversa entre os nós (endereço de 5 bytes)
byte addresses[][6] = {"1Node","2Node"};

// Configuração da identificação do rádio, sendo 1 ou 0
bool radioNumber = 0;

// Configuração do Hardware: configura o nRF24L01 nos pinos 7 e 8 do protocolo SPI
RF24 radio(7,8);

// Variável de controle para setagem de nó de transmissão ou recepção
bool role = 0;

// Flags de leitura serial
bool leituraSerial = 0;
// char leituraOpcaoSerial;

// Struct com tempo, opção do binário e valor do binário a ser transmitido
struct dataStruct{
  unsigned long _micros;
  char opcaoEnviada;
  boolean value[16];
  // Utilizado para armazenar o endereço do vetor de char
  // char* pont = value;
}myData;

// Struct com tipo de envio e vetor que contabiliza a quantidade de bits errados por envio
struct errorStruct{
  char tipoEnvio;
  int quantBitErrados[100];
}myError;

// Struct com informações de payloads
struct payloadStruct {
  unsigned int totalPayloads = 0;
  unsigned int errorPayloads = 0;  
  unsigned int porcentagemErros = 0;
}myPayloads;

void setup() {

  // Determinar o 'baud rate' a realizar a comunicação com o hardware
  Serial.begin(115200);
  Serial.println(F("*** Digite 'T' para comecar transmitindo para o outro no e para determinar qual binario enviado ***"));
  Serial.println("Opcoes de transmissao binarias");
  Serial.println("-----------------------------------");
  Serial.println("a) 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1");
  Serial.println("b) 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0");
  Serial.println("c) 0 0 0 0 0 0 0 0 1 1 1 1 1 1 1 1");
  Serial.println("d) 1 1 1 1 1 1 1 1 0 0 0 0 0 0 0 0");
  Serial.println("-----------------------------------");
  Serial.println("*** Digite a opcao do digito a enviar ***");
  
  // Inicializa o rádio para começar a transmissão
  radio.begin();

  // Seta o nível da potência. RF24_PA_MAX é o valor padrão, porém nesse caso começa-se com valor baixo.
  radio.setPALevel(RF24_PA_LOW);

  // Seta a velocidade da transmissão.
  radio.setDataRate(RF24_2MBPS);
  
  // Abre um pipe de escrita e leitura para cada rádio, com um endereço oposto
  if(radioNumber){
    radio.openWritingPipe(addresses[1]);
    radio.openReadingPipe(1,addresses[0]);
  }else{
    radio.openWritingPipe(addresses[0]);
    radio.openReadingPipe(1,addresses[1]);
  }
  
  // Inicia o rádio para leitura de dados
  radio.startListening();
}

void loop() { 
  
  // Papel de transmissor, chamando a função pingOut
  if (role == 1)  {
    pingOut();        
  }

  // Papel de receptor, chamando a função pongBack
  if ( role == 0 ) {
    pongBack();
  }

  // Mudança de papeis por comandos seriais
  // Verifica se a entrada serial está disponível
  if ( Serial.available() ) {
    char c = toupper(Serial.read());
    if ( c == 'T' && role == 0 && leituraSerial == 1) {      
      Serial.print(F("*** MUDANDO PARA PAPEL DE TRANSMISSOR -- DIGITE 'R' PARA MUDAR"));
      role = 1;                  // Torna-se o transmissor primário
    } else if ( c == 'R' && role == 1 && leituraSerial == 1 ) {
      Serial.println(F("*** MUDANDO PARA PAPEL DE RECEPTOR -- DIGITE A OPCAO A/B/C/D DE BINARIO E DEPOIS 'T' PARA MUDAR"));      
      role = 0;                // Torna-se o receptor primário
      leituraSerial = 0;       // Zera a flag para receber outro número binário
      radio.startListening();   
    } else if ( c == 'A' ) {
      //myData.value = 1111111111111111;
      for (int i=0;i<16;i++) {
        myData.value[i] = true;
      }
      leituraSerial = 1;
    } else if ( c == 'B' ) {
      //myData.value = 0000000000000000;
      for (int i=0;i<16;i++) {
        myData.value[i] = false;
      }
      leituraSerial = 1;
    } else if ( c == 'C' ) {
      //myData.value = 0000000011111111;
      for (int i=0;i<16;i++) {
        if(i < 8) {
          myData.value[i] = false;
        }
        else {
          myData.value[i] = true;  
        }  
      }
      leituraSerial = 1;
    } else if ( c == 'D' ) {
      //myData.value = 1111111100000000;
      for (int i=0;i<16;i++) {
        if(i < 8) {
          myData.value[i] = true;
        }
        else {
          myData.value[i] = false;  
        }
      }
      leituraSerial = 1;
    }
  }
}

// Função respectiva ao nó receptor
void pongBack() {
  if( radio.available()){
      // Enquanto existe informação a ser recebida
      while (radio.available()) {
        // Obtém o pacote de dados
        radio.read( &myData, sizeof(myData) );
      }
      // Para a recepção para permitir a escrita
      radio.stopListening();

      // AQUI ENTRA A CORREÇÃO DE ERROS
      // for(int i=0, i<16, i++) {
        
      // }

      radio.write( &myData, sizeof(myData) );

      // Resume a recepção para obter os próximos pacotes
      radio.startListening();
      Serial.print(F("Recebido o pacote "));
      Serial.print(myData._micros);  
      Serial.print(F(" : "));
      for (int i=0;i<16;i++) {
        if(myData.value[i] = true) {
          Serial.print('1');
        }
        else {
          Serial.print('0');
        }
      }
      Serial.println("");
      // Serial.println(myData.value);
      myPayloads.totalPayloads++;
  }
}

// Função respectiva ao nó transmissor
void pingOut() {
    // Cancela a recepção para começar a transmissão
    radio.stopListening();
    // Leitura serial para determinar qual número binário a ser enviado
    Serial.print(F("Comecando a enviar..."));

    myData._micros = micros();
     if (!radio.write( &myData, sizeof(myData) )){
       Serial.println(" O envio falhou!");
       myPayloads.errorPayloads++;
       myPayloads.totalPayloads++;
     }
        
    // Volta o rádio para estado de receptor
    radio.startListening();
    
    // Determina o tempo de espera, obtém o tempo atual em ms
    unsigned long started_waiting_at = micros();
    // Instancia uma variável para indicar se a resposta foi recebida ou não
    boolean timeout = false;
    
    while ( ! radio.available() ){
      // Se a espera superou 200ms, indica um timeout e sai do laço
      if (micros() - started_waiting_at > 200000 ){
          timeout = true;
          break;
      }      
    }
        
    if ( timeout ){
        Serial.println(F("Falha na transmissao, tempo de resposta esgotado!"));
        Serial.println(F("-----------------------------------------------------"));
        Serial.print(F("  Total de payloads: "));
        Serial.println(myPayloads.totalPayloads);
        Serial.print(F("  Total de payloads perdidos: "));
        Serial.println(myPayloads.errorPayloads);
        Serial.print(F("  Porcentagem de pacotes perdidos: "));
        myPayloads.porcentagemErros = (myPayloads.errorPayloads/myPayloads.totalPayloads)*100;
        Serial.println(myPayloads.porcentagemErros);
        // Serial.println("%%");
        // Correção de possíveis perdas de pacote por distância ou obstáculos na transmissão entre os nós
        if(micros() == 60000 && myPayloads.porcentagemErros >= 70) {
          radio.setPALevel(RF24_PA_MAX); // Aumenta a potência do rádio para máximo para contornar perda de pacotes
          radio.setDataRate(RF24_250KBPS); // Diminui a velocidade da transmissão do rádio
          Serial.print(F(" Potencia setada para maxima e velocidade de transmissao reduzida devido a perda de pacotes! "));
        }
        Serial.println(F("-----------------------------------------------------"));
    } else { // Se a mensagem não teve timeout, pega a mesma, compara e envia para o serial
        radio.read( &myData, sizeof(myData) );
        unsigned long time = micros();
        
        // Imprime o resultado
        Serial.print(F("Enviado "));
        Serial.println(time);
        Serial.println(F("-----------------------------------------------------"));
        Serial.print(F(", Recebeu resposta "));
        Serial.println(myData._micros);
        Serial.print(F(", Delay de retorno "));
        Serial.println(time-myData._micros);
        Serial.print(F(" Valor da letra "));
        for (int i=0;i<16;i++) {
          if(myData.value[i] = true) {
            Serial.print('1');
          }
          else {
            Serial.print('0');
          }
        }
        Serial.println("");
        // Serial.println(myData.value);
        myPayloads.totalPayloads++;
        Serial.println(F("-----------------------------------------------------"));
        Serial.print(F(" Total de payloads: "));
        Serial.println(myPayloads.totalPayloads);
        Serial.print(F(" Total de payloads perdidos: "));
        Serial.println(myPayloads.errorPayloads);
        Serial.print(F(" Porcentagem de pacotes perdidos: "));
        myPayloads.porcentagemErros = (myPayloads.errorPayloads/myPayloads.totalPayloads)*100;
        Serial.println(myPayloads.porcentagemErros);
        // Serial.println("%%");
        // Correção de possíveis perdas de pacote por distância ou obstáculos na transmissão entre os nós
        if(micros() == 60000 && myPayloads.porcentagemErros >= 70) {
          radio.setPALevel(RF24_PA_MAX); // Aumenta a potência do rádio para máximo para contornar perda de pacotes
          radio.setDataRate(RF24_250KBPS); // Diminui a velocidade da transmissão do rádio
          Serial.print(F(" Potencia setada para maxima e velocidade de transmissao reduzida devido a perda de pacotes! "));
        }
        Serial.println(F("-----------------------------------------------------"));
    }

    // Tenta após um segundo novamente
    delay(1000);
}