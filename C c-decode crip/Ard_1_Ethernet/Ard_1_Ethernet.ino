#include <SPI.h> // Inclusão da biblioteca SPI.h
#include <Ethernet.h> // Inclusão da biblioteca Ethernet.h
#include <EthernetUdp.h> // Inclusão da biblioteca EthernetUdp.h
#define port 8888 // Definição da porta de comunicação
#define bt 2 // Definição uma constante do botão
#define gr 3 // Definição uma constante do botão de grupo

EthernetUDP Udp; // Atribuição a variável Udp a classe EthernetUDP
byte mac[] = {0x90, 0xA2, 0xDA, 0x00, 0x64, 0x44}; // Definição do MAC Address do arduino 2
byte ip[] = {192,168,1,131}; // Definição do IP Address do arduino 1
byte remoteip[] = {192,168,1,151}; // Definição do IP Address do arduino 2

char packetBuffer[UDP_TX_PACKET_MAX_SIZE]; // Definição do packetBuffer com tamanho 24 byte
volatile bool flag1 = false, flag2 = false, grflag = false; // Definição de flags
char val[2] = {0,0}; // Definição do vetor val
int aux = 0;  // Definição da variável de contagem

void setup() {
  Serial.begin(9600); //Inicio da comunicação Serial e define a velocidade de comunicação
  flag1 = ethini(flag1); // 

  pinMode(bt, INPUT_PULLUP);  //Definição da pinagem do botao como saida
  pinMode(gr, INPUT_PULLUP);  //Definição da pinagem do botao grupo como saida
}

void loop() {
  val[0] = digitalRead(gr); //Atribuição do valor de leitura do botao grupo
  val[1] = digitalRead(bt); //Atribuição do valor de leitura do botao
/*
  //Identificação do numero do grupo
  if (val[0] == 1 && !grflag) {
    grflag = true;
  }
  else if (val[0] == 1 && grflag){
    aux = (aux + 1) % 7;
    grflag = false;
  }
*/
  //Inicialização do processo
  if (val[1] == 1 && flag1) {
    flag1 = false; //Bloqueio de acionamento do botão
  }
  else if (val[1] == 0 && !flag1 && !flag2) {
    ethenv(grupo(aux + 1));
    flag2 = true;  //Bloqueio de envio de dados
  }

  if (ethrec() == "Fim") {
    flag1 = true; //Desbloqueio do botão de acionamento
    flag2 = false; //Desbloqueio de envio de dados
  }

  delay(500); //Atraso de 0,5 segundos
}

/*
_______________________________________________Comunicação_______________________________________________
*/

//Função para envio de dados
void ethenv(char texto) {
  Udp.beginPacket(remoteip, port);  //Inicialização de pacote para envio da menssagem
  //Serial.println("Conexão Estabelecida: \t IP: %s", remoteip);

  Udp.write(texto);  //Ecrita no pacote do texto

  Udp.endPacket();  //Finalização de pacote e envio da menssagem

  //Serial.println("Dado Enviado");
}

//Função para recebimento de dados
String ethrec() {
  int packetSize = Udp.parsePacket(); //Definição do tamanho do pacote
  //Verifica se o pacote tem dados
  if (packetSize) {
    /*Serial.println("Conexão Estabelecida: \t IP: %s", remoteip);
    Serial
*/
/*
    {LEITURA DE PACOTES}

    Udp.read(packetBuffer, MaxSize)
    packetBuffer - Buffer para capturar entradas de pacote
    MaxSize      - Tamanho máximo do buffer
*/
    Udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
    
    //Serial.println("Dado Recebido.");
    return String(packetBuffer);
  }
  else {
    return "";
  }
}

/*
_______________________________________________Inicialização_______________________________________________
*/
bool ethini(bool flag) {
  while (!Serial) { } // Espera pela conexão com a porta serial

  /*
  {INICIALIZAÇÃO DA CONEXÃO VIA ETHERNET}
  
  Ethernet.begin(mac, ip, gateway, subnet)
  mac     - midia de controle de acesso de endereco da maquina (arduino/array 6 bytes)
  ip      - endereco de IP da maquina (arduino/array 4 bytes)
  gateway - endereco de IP da porta network (array 4 bytes)
  subnet  - mascara da network (array 4 bytes)
  */

  Ethernet.begin(mac, ip);

  //Verificação de que o Controlador de Ethernet está conectado
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Controlador de Ethernet não encontrado.");
    while (Ethernet.hardwareStatus() == EthernetNoHardware) {
      delay(1000); //Atraso de 1 segundo
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

  //Teste do Status do Link (Ligado/Desligado)
  if (Ethernet.linkStatus() == LinkOFF && flag) {
    Serial.println("Status do Link = DESLIGADO");
    while (Ethernet.linkStatus() == LinkOFF) {
      delay(1000); //Atraso de 1 segundo
    }
  }

  Udp.begin(port);  //Inicializa o protocolo UDP na Ethernet
  flag = true;

  return flag;
}

// Função de retorno da sequência de acionamento de cada grupo
char grupo(int num) {
  if (num == 1) {
    return "A+{A-//B+}A+{B-//A-}"; // Retorno da sequência de acionamento
  }
  else if (num == 2) {
    return "A-{A+//B+}A-{B-//A+}"; // Retorno da sequência de acionamento
  }
  else if (num == 3) {
    return "A+{A-//B-}A+{B+//A-}"; // Retorno da sequência de acionamento
  }
  else if (num == 4) {
    return "A-{A+//B-}A-{B+//A+}"; // Retorno da sequência de acionamento
  }
  else if (num == 5) {
    return "A+{A-//B+}{A+//B-}A-"; // Retorno da sequência de acionamento
  }
  else if (num == 6) {
    return "A-{A+//B+}{A-//B-}A+"; // Retorno da sequência de acionamento
  }
  else if (num == 7) {
    return "A+{A-//B-}{A+//B+}A-"; // Retorno da sequência de acionamento
  }
  else if (num == 8) {
    return "A-{A+//B-}{A-//B+}A+"; // Retorno da sequência de acionamento
  }
}