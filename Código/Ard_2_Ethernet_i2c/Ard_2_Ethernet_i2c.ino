#include <Wire.h> // Inclusão da biblioteca Wire.h
#define slave 2 // Definição uma constante do endereço do arduino 2 (Escravo)
#include <SPI.h> // Inclusão da biblioteca SPI.h
#include <Ethernet.h> // Inclusão da biblioteca Ethernet.h
#include <EthernetUdp.h> // Inclusão da biblioteca EthernetUdp.h
#define port 8888 // Definição da porta de comunicação
#define a0 3 // Definição de uma constante do sensor A0
#define a1 5 // Definição de uma constante do sensor A1

EthernetUDP Udp; // Atribuição a variável Udp a classe EthernetUDP
byte mac[] = {0x90, 0xA2, 0xDA, 0x00, 0x64, 0x50}; // Definição do MAC Address do arduino 2 (Escravo)
byte remoteip[] = {192,168,1,131}; // Definição do IP Address do arduino 1 (Acionamento)
byte ip[] = {192,168,1,151}; // Definição do IP Address do arduino 2 (Escravo)

char packetBuffer[UDP_TX_PACKET_MAX_SIZE]; // Definição do packetBuffer com tamanho 24 byte
volatile bool flag = false, flag1 = false; // Definição de um flag
int alok[3] = {0,0,0}; // Definição do vetor de alocagem
char akg = 1; // Definição de um aknologe

void setup() {
  Serial.begin(9600); // Inicio da comunicação Serial e define a velocidade de comunicação
  ethini();
  slvini();

  pinMode(a0, INPUT_PULLUP); // Definição da pinagem do sensor A0 como entrada
  pinMode(a1, INPUT_PULLUP); // Definição da pinagem do sensor A1 como entrada
}

void loop() {
  alok[0] = ethrec(); //
  alok[1] = digitalRead(a0); // Atribuição do valor de leitura do sensor A0
  alok[2] = digitalRead(a1); // Atribuição do valor de leitura do sensor A1

  delay(500); // Atraso de 0,5 segundos
}

/*
_______________________________________________Comunicação_______________________________________________
*/

// Função de envio de dados para o arduino 3 (Mestre)
void requestEvent() {
  /*Serial.print("Conexão Estabelecida: slave ");
  Serial.print(slave);
  Serial.print("\t")  
  */
  /*
  {ESCRITA DE BYTE NA PORTA SERIAL}

  write(buf,len)
  buf - vetor dados para envio no formato de bytes
  len - tamanho do buffer a ser enviado
  */

  char buf[3] = {alok[0], alok[1], alok[2]};

  Wire.write(buf, 3); // Escrita de dados em formato de bytes com 3 buffer a ser enviado para o arduino 3 (Mestre)

  //Serial.println("Dado Enviado");
}

// Função de recebimento de dados do arduino 3 (Mestre)
void receiveEvent() {
  /*
  Serial.print("Conexão Estabelecida: slave ");
  Serial.print(slave);
  Serial.print("\t")  
  */

  char texto = 0;
  // Loop de verificação de disponibilidade para recebimento de dados
  if (Wire.available()) {
    texto = Wire.read();
  }
  Serial.println(texto);

  if (texto == akg){
    Serial.println("Dado Recebido");

    ethenv(akg);
  }
}

// Função para envio de dados
void ethenv(char texto[UDP_TX_PACKET_MAX_SIZE]) {
  Udp.beginPacket(remoteip, port); // Inicialização de pacote para envio da menssagem
  /*
  Serial.print("Conexão Estabelecida: ")
  for (i = 1;i < 4; i++){
    Serial.print(remoteip[i], DEC);
    if (i < 3) {
      Serial.print(".")
    }
  }
  Serial.print("\t");
  */

  Udp.write(texto); // Ecrita no pacote do texto

  Udp.endPacket(); // Finalização de pacote e envio da menssagem

  // Serial.println("Dado Enviado");
}

// Função para recebimento de dados
int ethrec() {
  int packetSize = Udp.parsePacket(); // Definição do tamanho do pacote

  // Verifica se o pacote tem dados
  if (packetSize) {
    /*
    Serial.print("Conexão Estabelecida: ")
    for (i = 1;i < 4; i++){
      Serial.print(remoteip[i], DEC);
      if (i < 3) {
        Serial.print(".")
      }
    }
    Serial.print("\t");
    */
    /*
    {LEITURA DE PACOTES}

    Udp.read(packetBuffer, MaxSize)
    packetBuffer - Buffer para capturar entradas de pacote
    MaxSize - Tamanho máximo do buffer
    */

    Udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);

    // Serial.println("Dado Recebido");
    //Serial.println(packetBuffer[0], DEC);

    return packetBuffer[0];
  }
  else {
    return 0;
  }
}

/*
_______________________________________________Inicialização_______________________________________________
*/

void ethini() {
  while (!Serial) { } // Espera pela conexão com a porta serial

  /*
  {INICIALIZAÇÃO DA CONEXÃO VIA ETHERNET}

  Ethernet.begin(mac, ip, gateway, subnet)
  mac - midia de controle de acesso de endereco da maquina (arduino/array 6 bytes)
  ip - endereco de IP da maquina (arduino/array 4 bytes)
  gateway - endereco de IP da porta network (array 4 bytes)
  subnet - mascara da network (array 4 bytes)
  */

  Ethernet.begin(mac, ip);

  // Verificação de que o Controlador de Ethernet está conectado
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Controlador de Ethernet não encontrado.");
    while (Ethernet.hardwareStatus() == EthernetNoHardware) {
      delay(1000); // Atraso de 1 segundo
    }
  }
  if (Ethernet.hardwareStatus() == EthernetW5100) {
    Serial.println("Controlador de Ethernet W5100 detectado.");
  }
  else if (Ethernet.hardwareStatus() == EthernetW5200) {
    Serial.println("Controlador de Ethernet W5200 detectado.");
    flag = true;
  }
  else if (Ethernet.hardwareStatus() == EthernetW5500) {
    Serial.println("Controlador de Ethernet W5500 detectado.");
    flag = true;
  }

  // Teste do Status do Link (Ligado/Desligado)
  if (Ethernet.linkStatus() == LinkOFF && flag) {
    Serial.println("Status do Link = DESLIGADO");
    while (Ethernet.linkStatus() == LinkOFF) {
      delay(1000); // Atraso de 1 segundo
    }
  }

  Udp.begin(port); // Inicializa o protocolo UDP na Ethernet
  flag = true;
}

void slvini() {
  Wire.begin(slave); // Inicio da comunicação via bus entre arduino 3/arduino 2 (Mestre/Escravo)
  Wire.onReceive(receiveEvent); // Definição de quando solicitado chamamos a função receiveEvent
  Wire.onRequest(requestEvent); // Definição de quando solicitado chamamos a função requestEvent
}