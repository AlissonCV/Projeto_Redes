#include <Wire.h> //Inclusão da biblioteca Wire.h
#define slave 4 //Definição uma constante do endereço do arduino 4 (Escravo)
#define pinb0 5  //Definição uma constante do sensor B0
#define pinb1 8  //Definição uma constante do sensor B1

char b0, b1, buf[2] = {0, 0};  //Definição de um vetor de 2 caracteres

void setup() {
  Serial.begin(9600); //Inicio da comunicação Serial e define a velocidade de comunicação
  Wire.begin(slave);  //Inicio da comunicação via bus entre arduino 5/arduino 4 (Mestre/Escravo)
  Wire.onRequest(requestEvent); //Definição de quando solicitado chamamos a função requestEvent

  Serial.println("Adaptor I2C e Ethernet Inicializado");

  pinMode(pinb0, INPUT_PULLUP);  //Definição da pinagem do sensor B0 como entrada
  pinMode(pinb1, INPUT_PULLUP);  //Definição da pinagem do sensor B1 como entrada
  pinMode(10,OUTPUT);
  pinMode(12,OUTPUT);
}

void loop() {
  buf[0] = digitalRead(pinb0); //Atribuição do valor de leitura do sensor B0
  buf[1] = digitalRead(pinb1); //Atribuição do valor de leitura do sensor B1

  digitalWrite(10,buf[0]);
  digitalWrite(12,buf[1]);

  delay(500); //Atraso de 0,5 segundos
}

//Função de envio de dados para o arduino 5 (Mestre)
void requestEvent() {
  Serial.print("Conexão Estabelecida: Mestre \t");
  
  Wire.write (buf, 2);  //Escrita de dados em formato de bytes com 2 buffer a ser enviado para o arduino 5 (Mestre)

  Serial.println("Conexão Encerrada");  //
}