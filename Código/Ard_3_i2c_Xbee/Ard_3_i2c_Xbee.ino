#include <Wire.h> // Inclusão da biblioteca Wire.h
#include <XBee.h> // Inclusão da biblioteca XBee.h
#define slave 2 // Definição uma constante do endereço do Arduino 2 (Escravo)
#define rele 8 // Definição uma constante do rele
#define avanco 1 // Definição uma constante do avanço
#define recuo 0 // Definição uma constante do avanço
#define A 0 // Definição uma constante do embolo A
#define B 1 // Definição uma constante do embolo B

int gr, a0, a1, b0, b1, aux; // Definição de variáveis como caractere
volatile bool flag = false, flagg = false;

XBee xbee = XBee(); // Atribui uma variável de chamada da classe Xbee

uint8_t payload[] = {0}; // Atribui uma variável de entrada de dados no pacote
XBeeAddress64 add64 = XBeeAddress64(0x0013A200,0x40C1B1F0); // Atribui o valor do MAC Address do Xbee
ZBTxRequest data = ZBTxRequest(add64, payload, sizeof(payload)); // Atribui uma variável de envio de dados
ZBTxStatusResponse txtStatus = ZBTxStatusResponse(); // Atribui o status do Xbee para envio

ZBRxResponse recive = ZBRxResponse(); // Atribui uma variável de recebimento de dados

void setup() {
  Serial.begin(9600); // Inicialização da porta Serial

  xbee.setSerial(Serial); // Inicialização da placa Xbee (Arduino 3/Arduino 5)
  Wire.begin(); // Inicialização do barramento i2c (Arduino 3 - Mestre/Arduino 2 - Escravo)

  pinMode(rele, OUTPUT); // Definição da pinagem do rele como saida
  pinMode(9, OUTPUT); // Definição da pinagem do rele como saida
  pinMode(10, OUTPUT); // Definição da pinagem do rele como saida
  pinMode(11, OUTPUT); // Definição da pinagem do rele como saida
  pinMode(12, OUTPUT); // Definição da pinagem do rele como saida
}

void loop() {
  requestEvent(); // Requisição de dados do Arduino 2 (Escravo)

  // {IDENTIFICAÇÃO DO GRUPO}
  if (flag) {
    // Recebimento de dados de comando via Xbee do Arduino 5
    comando(A, avanco, false); // Chamada da classe comando atribuindo o comando e identificando o tipo
    comando(recuo, avanco, true); // Chamada da classe comando atribuindo o comando e identificando o tipo
    comando(A, avanco, false); // Chamada da classe comando atribuindo o comando e identificando o tipo
    comando(recuo, recuo, true); // Chamada da classe comando atribuindo o comando e identificando o tipo

    flag = false;
    //sendEvent(1); // Envio de dados via i2c para o arduino 2
  }
  else {
    delay(500); // Atraso de 0,5 segundos
  }
}

/*
_______________________________________________Comando_______________________________________________
*/

void comando(int num1, int num2, bool flag1) {
  if (num1 == A && !flag1) {
    digitalWrite(rele, num2); // Escrita no pino do rele para controle do solenoide A
    flag = true; // Bloqueio para finalização do deslocamento
  }
  else if (num1 == B && !flag1) {
    flag = true; // Bloqueio para finalização do deslocamento
    while (flag) {
      requestData();
      if (flag) {
        payload[0] = (4 | (num2&3)); // Carrega os dados para envio
        xbee.send(data); // Envio do dado do pino do rele para controle do solenoide B via Xbee
      }
    }
    flag = true; // Bloqueio para finalização do deslocamento */
  }
  else if (flag1) {
    digitalWrite(rele, num1); // Escrita no pino do rele para controle do solenoide A
    flag = true; // Bloqueio para finalização do deslocamento
    while (flag) {
      requestData();
      if (flag) {
        payload[0] = (4 | (num2&3)); // Carrega os dados para envio
        xbee.send(data); // Envio do dado do pino do rele para controle do solenoide B via Xbee
      }
    }
    flag = true; // Bloqueio para finalização do deslocamento
  }

  // Loop de leitura dos valores dos sensores A e B
  while (flag) {
    delay(500);

    requestEvent(); // Requisição de dados do Arduino 2 (Escravo)
    while (flag) {
      requestData(); // Requisição de dados do Arduino 5 (Xbee)
    }

    flag = true;
    // Verificador de posição dos valores dos sensores de A e B para Desbloqueio de looop
    if (flag && (a0 == 1 || a1 == 1) && (b0 == 1 || b1 == 1)) {
      digitalWrite(9,1);
      flag = false;
    }
  }
}

/*
_______________________________________________Comunicação_______________________________________________
*/

void sendEvent(char texto) {
  Wire.beginTransmission(slave); // Inicialização de conexão com o Arduino 2 (Escravo)

  Wire.write(texto); // Escrita de dados para o arduino 2 (Escravo)
}
  
void requestEvent() {
  Wire.requestFrom(slave,3); // Requisita 3 bytes do Arduino 2 (Escravo)
  
  // Loop de verificação de dados para recepção de dados
  while (Wire.available()) {
    gr = Wire.read(); // Atribuição do valor do grupo na leitura dos dados enviados pelo Arduino 2 (Escravo)
    a0 = Wire.read(); // Atribuição do valor do sensor A0 na leitura dos dados enviados pelo Arduino 2 (Escravo)
    a1 = Wire.read(); // Atribuição do valor do sensor A1 na leitura dos dados enviados pelo Arduino 2 (Escravo)
  }

  if (gr == 1) {
    flag = true;
  }
}

void requestData() {
  payload[0] = 255; // Bit de permissão para envio de dados
  xbee.send(data); // Envio de dado para o Arduino 5 (Xbee)

  uint8_t resposta; // Definição de um vetor para armazenar os dados
  xbee.readPacket(); // Leitura do pacote

  // Checagem do recebimento do pacote
  if(xbee.getResponse().isAvailable()){
    if(xbee.getResponse().getApiId()==ZB_RX_RESPONSE){
      xbee.getResponse().getZBRxResponse(recive); // Atribui ao recebimento o pacote
      resposta = recive.getData(0); // Atribui a variável resposta a mensagem
    }
  }

  if (resposta == 255) {
    flag = false;
    //aux = 0;
  }
  else if (((resposta>>4) & 3) == 1) {
    digitalWrite(9, 1);
    b0 = ((resposta>>2) & 3); // Atribuição do valor do sensor B0 na leitura dos dados enviados pelo Arduino 5
    b1 = (resposta & 3); // Atribuição do valor do sensor B1 na leitura dos dados enviados pelo Arduino 5
    flag = false;
    //aux = 0;
  }
}