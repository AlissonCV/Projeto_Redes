#include <Wire.h> //Inclusão da biblioteca Wire.h
#include <XBee.h> // Inclusão da biblioteca XBee.h
#define slave 4 //Definição uma constante do endereço do arduino 4 (Escravo)
#define rele 9  //Definição uma constante do rele

int cmd = 0, aux;  //Definição de variáveis carecteres
uint8_t b0, b1, resposta;
volatile bool flag = false, flagg = false;

XBee xbee = XBee(); // Atribui uma variável de chamada da classe Xbee

uint8_t payload[] = {0}; // Atribui uma variável de entrada de dados no pacote
XBeeAddress64 add64 = XBeeAddress64(0x0013A200,0x40E8B8A3); // Atribui o valor do MAC Address do Xbee
ZBTxRequest data = ZBTxRequest(add64, payload, sizeof(payload)); // Atribui uma variável de envio de dados
ZBTxStatusResponse txtStatus = ZBTxStatusResponse(); // Atribui o status do Xbee para envio

ZBRxResponse recive = ZBRxResponse(); // Atribui uma variável de recebimento de dados

void setup() {
  Serial.begin(9600); //Inicio da comunicação Serial e define a velocidade de comunicação
  
  xbee.setSerial(Serial); // Inicialização da placa Xbee (Arduino 3/Arduino 5)
  Wire.begin(); //Inicio da comunicação via bus entre arduino 5/arduino 4 (Mestre/Escravo)

  pinMode(rele, OUTPUT);  //Definição da pinagem do rele como saida
  pinMode(10, OUTPUT);  //Definição da pinagem do rele como saida
  pinMode(11, OUTPUT);  //Definição da pinagem do rele como saida
}

void loop() {
  requestEvent(); //Requisição de dados do arduino 4 (Escravo)
  onrequestData(); // Quando requisitado dados do arduino 3 (Xbee)

  if (flag) {
    digitalWrite(rele, cmd); //Escrita na porta do rele para controle do solenoide
    cmd = 0;
  }
  
  delay(500); //Atraso de 0,1 segundos
}

/*
_______________________________________________Comunicação_______________________________________________
*/

void requestEvent() {
  Wire.requestFrom(slave, 2);  //Requisita 2 bytes do arduino 4 (Escravo)

  //Loop de verificação de disponibilidade para envio de dados
  while (Wire.available()) {
    b0 = Wire.read(); //Atribuição do valor do sensor B0 na leitura dos dados enviados pelo Arduino 4 (Escravo)
    b1 = Wire.read(); //Atribuição do valor do sensor B1 na leitura dos dados enviados pelo Arduino 4 (Escravo)
  }

  digitalWrite(10, b0); //Escrita na porta do rele para controle do solenoide
  digitalWrite(11, b1); //Escrita na porta do rele para controle do solenoide
}

void onrequestData() {
  xbee.readPacket(); // Leitura do pacote

  // Checagem do recebimento do pacote
  if(xbee.getResponse().isAvailable()){
    if(xbee.getResponse().getApiId()==ZB_RX_RESPONSE){
      xbee.getResponse().getZBRxResponse(recive); // Atribui ao recebimento o pacote
      resposta = (int)recive.getData(0); // Atribui a variável resposta a mensagem
    }
  }

  if (resposta == 255) {
    payload[0] = (16 | ((b0&3)<<2) | (b1&3)); // Carrega os dados para envio
    xbee.send(data); // Envio do dado do sensor B0 e B1 via Wirelees
  }
  else if (((resposta>>2)&3) == 1) {
    cmd = (resposta & 3); // Atribuição do valor do sensor B1 na leitura dos dados enviados pelo Arduino 5
    flag = true;

    //payload[0] = 255; // Carrega os dados para envio
    //xbee.send(data); // Envio do dado do sensor B0 e B1 via Wirelees
  }
}