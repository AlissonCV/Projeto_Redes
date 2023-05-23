#include <Wire.h> // Inclusão da biblioteca Wire.h
#include <XBee.h> // Inclusão da biblioteca XBee.h
#define slave 2 // Definição uma constante do endereço do Arduino 2 (Escravo)
#define rele 8 // Definição uma constante do rele
#define avanco 1 // Definição uma constante do avanço
#define recuo 0 // Definição uma constante do avanço
#define A 0 // Definição uma constante do embolo A
#define B 1 // Definição uma constante do embolo B

int gr, a0, a1, b0, b1, aux = 0; // Definição de variáveis como caractere
uint8_t resposta; // Definição de um vetor para armazenar os dados
volatile bool flag = false;

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
  if (gr == 1 && a0 == 1) {
      comando(A, avanco, false); // Chamada da classe comando atribuindo o comando e identificando o tipo
  }
  else if (a1 == 1 && b0 == 1) {
    comando(recuo, avanco, true); // Chamada da classe comando atribuindo o comando e identificando o tipo
  }
  else if (a0 == 1 && b1 == 1) {
    comando(A, avanco, false); // Chamada da classe comando atribuindo o comando e identificando o tipo
  }
  else if (a1 == 1 && b1 == 1) {
    comando(recuo, recuo, true); // Chamada da classe comando atribuindo o comando e identificando o tipo
    flag = false;
  }
  else {
    delay(500); // Atraso de 0,5 segundos
  }
}

/*
_______________________________________________Comando_______________________________________________
*/

void comando(int num1, int num2, bool flagc) {
  if (num1 == A && !flagc) {
    digitalWrite(rele, num2); // Escrita no pino do rele para controle do solenoide A
  }
  else if (num1 == B && !flagc) {
    payload[0] = (176 | (num2&3)); // Carrega os dados para envio
    xbee.send(data); // Envio do dado do pino do rele para controle do solenoide B via Xbee
  }
  else if (flagc) {
    digitalWrite(rele, num1); // Escrita no pino do rele para controle do solenoide A
    payload[0] = 255; // Carrega os dados para envio
    xbee.send(data);
    /*payload[0] = (176 | (num2&3)); // Carrega os dados para envio
    xbee.send(data); // Envio do dado do pino do rele para controle do solenoide B via Xbee*/
  }
}

/*
_______________________________________________Comunicação_______________________________________________
*/
  
void requestEvent() {
  Wire.requestFrom(slave,3); // Requisita 3 bytes do Arduino 2 (Escravo)
  
  // Loop de verificação de dados para recepção de dados
  while (Wire.available()) {
    if (!flag) {
      gr = Wire.read(); // Atribuição do valor do grupo na leitura dos dados enviados pelo Arduino 2 (Escravo)
      if (gr != 0) {
        flag = true;
      }
    }
    else {
      aux = Wire.read(); // Atribuição do valor do grupo na leitura dos dados enviados pelo Arduino 2 (Escravo)
    }
    a0 = Wire.read(); // Atribuição do valor do sensor A0 na leitura dos dados enviados pelo Arduino 2 (Escravo)
    a1 = Wire.read(); // Atribuição do valor do sensor A1 na leitura dos dados enviados pelo Arduino 2 (Escravo)
  }

  digitalWrite(9, a0);
  digitalWrite(10, a1);

  payload[0] = 255; // Bit de permissão para envio de dados
  xbee.send(data); // Envio de dado para o Arduino 5 (Xbee)

  xbee.readPacket(); // Leitura do pacote

  // Checagem do recebimento do pacote
  if(xbee.getResponse().isAvailable()){
    if(xbee.getResponse().getApiId()==ZB_RX_RESPONSE){
      xbee.getResponse().getZBRxResponse(recive); // Atribui ao recebimento o pacote
      resposta = (int)recive.getData(0); // Atribui a variável resposta a mensagem
    }
  }

  if (((resposta>>4) & 15) == 11) {
    b0 = (resposta & 3); // Atribuição do valor do sensor B0 na leitura dos dados enviados pelo Arduino 5
    b1 = ((resposta>>2) & 3); // Atribuição do valor do sensor B1 na leitura dos dados enviados pelo Arduino 5
  }

  digitalWrite(11, b0);
  digitalWrite(12, b1);
}