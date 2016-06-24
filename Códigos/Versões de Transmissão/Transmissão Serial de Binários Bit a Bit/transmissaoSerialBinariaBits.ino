/*
* Código para envio e recepção de N dígitos binários bit a bit entre dois nRF24L01+ rádios
* Envio de informações por struct
* Última atualização: 24/06/2016
*/

// Bibliotecas de protocolo serial e do módulo nRF24L01
#include <SPI.h>
#include "nRF24.h"

// Determina o máximo de pacotes a serem transmitidos por envio
#define MAX_NUM_PAYLOADS 20

// Determina dois endereços de Pipe para conversa entre os nós (endereço de 5 bytes)
byte addresses[][6] = {"1Node","2Node"};

// Configuração da identificação do rádio, sendo 1 ou 0
bool radioNumber = 0;

// Configuração do Hardware: configura o nRF24L01 nos pinos 7 e 8 do protocolo SPI
RF24 radio(7,8);

// Variável de controle para setagem de nó de transmissão ou recepção
bool role = 0;

// Iteração para andar no vetor conforme os valores são lidos
unsigned int finalValueIndex = 0;

// Struct com tempo, valor do binário a ser transmitido por pacote, quantidades possíveis de bits perdidos, vetor final (esperado e recebido)
struct dataStruct{
  unsigned long _micros;
  char sentValue = '0';
  unsigned int lostBits;
  char sentFinalValue[MAX_NUM_PAYLOADS];
  char receivedFinalValue[MAX_NUM_PAYLOADS];
}myData;

// Struct com informações de payloads
struct payloadStruct {
  unsigned int totalPayloads = 0;
  unsigned int errorPayloads = 0;  
  unsigned int totalIterations = MAX_NUM_PAYLOADS; // Estabelece uma comunicação que trafegará MAX_NUM_PAYLOADS bits, decrementando de acordo com a quantidade recebida
}myPayloads;

// Porcentagem instantânea de erro obtida pelos pacotes perdidos/pacotes recebidos
float errorPercentage = 0;

// Flag de segurança para incrementar e decrementar contadores
boolean erroGeral = false;

void setup() {

  // Determinar o 'baud rate' a realizar a comunicação com o hardware
  Serial.begin(115200);
  Serial.println(F("*** Digite 'T' para comecar transmitindo para o outro no e para determinar qual binario enviado ***"));
  
  // Inicializa o rádio para começar a transmissão
  radio.begin();

  // Seta o nível da potência. RF24_PA_MAX é o valor padrão, porém nesse caso começa-se com valor baixo.
  radio.setPALevel(RF24_PA_MAX);

  // Seta a velocidade da transmissão.
  radio.setDataRate(RF24_2MBPS);

  // Seta o canal da transmissão (4GHz + 0-125).
  radio.setChannel(6);
  
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
    if ( c == 'T' && role == 0 ) {      
      Serial.print(F("*** MUDANDO PARA PAPEL DE TRANSMISSOR -- DIGITE 'R' PARA MUDAR"));
      role = 1;                  // Torna-se o transmissor primário
    } else if ( c == 'R' && role == 1 ) {
      Serial.println(F("*** MUDANDO PARA PAPEL DE RECEPTOR -- DIGITE 'T' PARA MUDAR"));      
      role = 0;                // Torna-se o receptor primário
      radio.startListening();   
    } 
  }
}

// Função respectiva ao nó receptor
void pongBack() {
  if( radio.available() && myPayloads.totalIterations > 0 ){
      // Enquanto existe informação a ser recebida
      while (radio.available()) {
        // Obtém o pacote de dados
        radio.read( &myData, sizeof(myData) );
        // Verifica se foram perdidos pacotes de transmissão anteriores ao pacote recebido
        if (myData.lostBits > 0) {
          for (int i=0;i<myData.lostBits;i++) {
            // Para a quantidade de pacotes perdidos atribui-se N don't cares X
            myData.receivedFinalValue[finalValueIndex] = 'X';
            myData.sentFinalValue[finalValueIndex] = 'X';
            // Incrementa o índice do valor final e decrementa o total de interações previamente estabelecido
            finalValueIndex++;
            myPayloads.totalIterations--;
          }
          // Zera a quantidade de pacotes perdidos para enviar ao transmissor
          myData.lostBits = 0;
        }
        // Atribui o valor recebido no pacote da transmissão, incrementando o índice do valor final
        myData.receivedFinalValue[finalValueIndex] = myData.sentValue;
        finalValueIndex++;
      }
      // Para a recepção para permitir a escrita
      radio.stopListening();
      radio.write( &myData, sizeof(myData) );

      // Resume a recepção para obter os próximos pacotes
      radio.startListening();
      Serial.println(F("-----------------------------------------------------"));
      Serial.print(F("Recebido o pacote "));
      Serial.print(myData._micros);  
      Serial.print(F(" : "));
      // Imprime bit a bit na linha
      for (int i=0;i<finalValueIndex;i++) {
        Serial.print(char(myData.receivedFinalValue[i]));
      }
      Serial.println(" ");
      // Serial.println(myData.value);
      // Decrementando e incrementando contadores de iteração e de total de payloads
      myPayloads.totalIterations--;
      myPayloads.totalPayloads++;
  }
  // Após o fim da quantidade de interações previamente estabelecidas
  else if (myPayloads.totalIterations == 0) {
    radio.read( &myData, sizeof(myData) );
    // Correção do pacote final
    for (int i=0;i<finalValueIndex;i++) {
      if (myData.receivedFinalValue[i] == 'X') {
        myData.receivedFinalValue[i] = myData.sentFinalValue[i];
      }
    }
    // Serial.print(F("Palavra final corrigida: ")); 
    // // Imprime bit a bit na linha
    // for (int i=0;i<finalValueIndex;i++) {
    //   Serial.print(char(myData.receivedFinalValue[i]));
    // }
    // Serial.println(" ");
    Serial.println("Fim de transmissao!");
    Serial.end();
  }
}

// Função respectiva ao nó transmissor
void pingOut() {
    erroGeral = false;
    if (myPayloads.totalIterations < 1) {
      // Garante que caso o último pacote enviado não seja entregue, ele dispare 10 pacotes de segurança
      for(int i=0;i<10;i++){
        radio.write( &myData, sizeof(myData));
        // Serial.println("Fim de transmissao!");
      }
      Serial.println("Fim de transmissao!");
      Serial.end();
    }
    // Cancela a recepção para começar a transmissão
    radio.stopListening();
    // Leitura serial para determinar qual número binário a ser enviado
    Serial.print(F("Comecando a enviar..."));

    myData._micros = micros();
    // if(micros() == 50000) {
    //   myData.sentValue = '1'; // Determina mudança do valor de bit após metade do tempo
    // }
     if (!radio.write( &myData, sizeof(myData) )){
       // Caso o envio não seja feito pelo transmissor, incrementa-se o número de pacotes perdidos, de pacotes enviados, seta a flag de segurança, incrementa a quantidade de bits perdidos e decrementa a de interações
       Serial.println(" O envio falhou!");
       myPayloads.errorPayloads++;
       myPayloads.totalPayloads++;
       erroGeral = true;
       myData.lostBits++;
       myPayloads.totalIterations--;
     }
        
    // Volta o rádio para estado de receptor
    radio.startListening();
    
    // Determina o tempo de espera, obtém o tempo atual em ms
    unsigned long started_waiting_at = micros();
    // Instancia uma variável para indicar se a resposta foi recebida ou não
    boolean timeout = false;
    
    // Verifica o valor retornado de ACK
    while ( ! radio.available() ){
      // Se a espera superou 200ms, indica um timeout e sai do laço
      if (micros() - started_waiting_at > 200000 ){
          timeout = true;
          break;
      }      
    }
    // Se o ACK não foi recebido, entra na condicional
    if ( timeout ){
        Serial.println(F("Falha na transmissao, tempo de resposta esgotado!"));
        Serial.println("");
        
        // for (int i=0;i<10;i++) {
        //   myData.receivedFinalValue[i] = '1';
        // }
        // for (int i=0;i<10;i++) {
        //   Serial.print(char(myData.receivedFinalValue[finalValueIndex]));
        // }
        // Verifica se os contadores já foram incrementados e decrementados por meio da flag
        if(!erroGeral) {
          myPayloads.errorPayloads++;
          myPayloads.totalPayloads++;
          myPayloads.totalIterations--;
        }
        Serial.println(F("-----------------------------------------------------"));
        Serial.print(F("  Total de payloads: "));
        Serial.println(myPayloads.totalPayloads);
        Serial.print(F("  Total de payloads perdidos: "));
        Serial.println(myPayloads.errorPayloads);
        Serial.print(F("  Porcentagem de pacotes perdidos: "));
        errorPercentage = ((float(myPayloads.errorPayloads))/(float(myPayloads.totalPayloads)))*100; // Casting necessário para cálculo de float
        Serial.println(errorPercentage);
        // Serial.println("%%");
        // Correção de possíveis perdas de pacote por distância ou obstáculos na transmissão entre os nós
        if(micros() == 60000 && errorPercentage >= 70) {
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
        Serial.print(F(" Valor da palavra "));
        // Atribui o valor recebido no pacote ao último valor da array da palavra enviada
        myData.sentFinalValue[finalValueIndex] = myData.sentValue;
        finalValueIndex++;
        // Imprime na linha o valor da palavra bit a bit até a última posição recebida (impressão instantânea)
        for (int i=0;i<finalValueIndex;i++) {
          Serial.print(char(myData.sentFinalValue[i]));
        }
        Serial.println(" ");
        // Serial.println(myData.value);
        // Verifica se os contadores já foram incrementados e decrementados por meio da flag
        if(!erroGeral) {
          myPayloads.totalIterations--;
          myPayloads.totalPayloads++;
        }
        Serial.println(F("-----------------------------------------------------"));
        Serial.print(F("  Total de payloads: "));
        Serial.println(myPayloads.totalPayloads);
        Serial.print(F("  Total de payloads perdidos: "));
        Serial.println(myPayloads.errorPayloads);
        Serial.print(F("  Porcentagem de pacotes perdidos: "));
        errorPercentage = ((float(myPayloads.errorPayloads))/(float(myPayloads.totalPayloads)))*100;
        Serial.println(errorPercentage);
        // Serial.println("%%");
        // Correção de possíveis perdas de pacote por distância ou obstáculos na transmissão entre os nós
        if(micros() == 60000 && errorPercentage >= 70) {
          radio.setPALevel(RF24_PA_MAX); // Aumenta a potência do rádio para máximo para contornar perda de pacotes
          radio.setDataRate(RF24_250KBPS); // Diminui a velocidade da transmissão do rádio
          Serial.print(F(" Potencia setada para maxima e velocidade de transmissao reduzida devido a perda de pacotes! "));
        }
        Serial.println(F("-----------------------------------------------------"));
    }
    // Serial.println(myPayloads.totalIterations);
    // Tenta após um segundo novamente
    delay(1000);
}