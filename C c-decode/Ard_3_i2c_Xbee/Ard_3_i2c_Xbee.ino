#include <Wire.h> // Inclusão da biblioteca Wire.h
#include <XBee.h> // Inclusão da biblioteca XBee.h
#define slave 2 // Definição uma constante do endereço do Arduino 2 (Escravo)
#define rele 8 // Definição uma constante do rele
#define avanco 1 // Definição uma constante do avanço
#define recuo 0 // Definição uma constante do avanço
#define A 0 // Definição uma constante do embolo A
#define B 1 // Definição uma constante do embolo B

char gr, a0, a1, b0, b1; // Definição de variáveis como caractere

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
}

void loop() {
  requestEvent(); // Requisição de dados do Arduino 2 (Escravo)
  reciveXbee(); // Recebimento de dados de comando via Xbee do Arduino 5

  // {IDENTIFICAÇÃO DO GRUPO}
  if (grupo(gr) != 0) {
    decode(grupo(gr)); // Decodificação dos comandos escritos em formato de string
    sendEvent("Fim"); // Envio de dados via i2c para o arduino 2
  }
  else {
    delay(500); // Atraso de 0,5 segundos
  }
}

/*
_______________________________________________Decodificação_______________________________________________
*/
void decode(String texto) {
  int count = 0; // Definição de um contador
  unsigned int tamanho = texto.length();

  status(texto, tamanho); // Identificação da posição de A e B

  // {LOOP DE DECODIFICAÇÃO DO TEXTO RECEBIDO}
  while (count <= tamanho-1) {
    if (String(texto[count]) == "A" || String(texto[count]) == "B") {
      if (leitura(count, texto) == 1) {
        comando(A, avanco, false); // Chamada da classe comando atribuindo o comando e identificando o tipo
      }
      else if (leitura(count, texto) == 2) {
        comando(A, recuo, false); // Chamada da classe comando atribuindo o comando e identificando o tipo
      }
      else if (leitura(count, texto) == 3) {
        comando(B, avanco, false); // Chamada da classe comando atribuindo o comando e identificando o tipo
      }
      else if (leitura(count, texto) == 4) {
        comando(B, recuo, false); // Chamada da classe comando atribuindo o comando e identificando o tipo
      }

      count += 2; // Incrementa na variável
    }
    else if (String(texto[count]) == "{") {
      count++; // Incrementa na variável
      if ((leitura(count, texto) == 1 && leitura(count + 4, texto) == 3) || (leitura(count, texto) == 3 && leitura(count + 4, texto) == 1)) {
        comando(avanco, avanco, true); // Chamada da classe comando atribuindo o comando e identificando o tipo
      }
      else if ((leitura(count, texto) == 1 && leitura(count + 4, texto) == 4) || (leitura(count, texto) == 4 && leitura(count + 4, texto) == 1)) {
        comando(avanco, recuo, true); // Chamada da classe comando atribuindo o comando e identificando o tipo
      }
      else if ((leitura(count, texto) == 2 && leitura(count + 4, texto) == 3) || (leitura(count, texto) == 3 && leitura(count + 4, texto) == 2)) {
        comando(recuo, avanco, true); // Chamada da classe comando atribuindo o comando e identificando o tipo
      }
      else if ((leitura(count, texto) == 2 && leitura(count + 4, texto) == 4) || (leitura(count, texto) == 4 && leitura(count + 4, texto) == 2)) {
        comando(recuo, recuo, true); // Chamada da classe comando atribuindo o comando e identificando o tipo
      }

      count += 7; // Incrementa na variável
    }
    else {
      count++; // Incrementa na variável
    }

    delay(500); // Atraso de 0,5 segundos
  }
}

int leitura(int num, String texto) {
  if (String(texto[num]) == "A" && String(texto[num + 1]) == "+") {
    return 1; // Retorna o valor para identificação do comando
  }
  if (String(texto[num]) == "A" && String(texto[num + 1]) == "-") {
    return 2; // Retorna o valor para identificação do comando
  }
  if (String(texto[num]) == "B" && String(texto[num + 1]) == "+") {
    return 3; // Retorna o valor para identificação do comando
  }
  if (String(texto[num]) == "B" && String(texto[num + 1]) == "-") {
    return 4; // Retorna o valor para identificação do comando
  }
  else {
    return 0;
  }
}

/*
_______________________________________________Identificação de Status_______________________________________________
*/

void status(String texto, int tamanho) {
  int count[3] = {0, 0, 0}; // Definição de um vetor de caracteres
  
  while (count[0] <= tamanho-1) {
    int val = leitura(count[0], texto);

    if (val == 1 && count[1] == 0 && a0 == 1) {
      count[1] = 1; // Identificador do primeiro A
      count[0] += 2; // Incremento para pular para a próxima leitura      
    }
    else if (val == 1 && count[1] == 0 && a1 == 1) {
      comando(A, recuo, false); // Chamada da classe comando atribuindo o comando e identificando o tipo
      count[1] = 1; // Identificador do primeiro A
      count[0] += 2; // Incremento para pular para a próxima leitura
    }
    else if (val == 2 && count[1] == 0 && a1 == 1) {
      count[1] = 1; // Identificador do primeiro A
      count[0] += 2; // Incremento para pular para a próxima leitura
    }
    else if (val == 2 && count[1] == 0 && a0 == 1) {
      comando(A, avanco, false); // Chamada da classe comando atribuindo o comando e identificando o tipo
      count[1] = 1; // Identificador do primeiro A
      count[0] += 2; // Incremento para pular para a próxima leitura
    }
    else if (val == 3 && count[2] == 0 && b0 == 1) {
      count[2] = 1; // Identificador do primeiro B
      count[0] += 2; // Incremento para pular para a próxima leitura
    }
    else if (val == 3 && count[2] == 0 && b1 == 1) {
      comando(B, recuo, false); // Chamada da classe comando atribuindo o comando e identificando o tipo
      count[2] = 1; // Identificador do primeiro B
      count[0] += 2; // Incremento para pular para a próxima leitura
    }
    else if (val == 4 && count[2] == 0 && b1 == 1) {
      count[2] = 1; // Identificador do primeiro B
      count[0] += 2; // Incremento para pular para a próxima leitura
    }
    else if (val == 4 && count[2] == 0 && b0 == 1) {
      comando(B, avanco, false); // Chamada da classe comando atribuindo o comando e identificando o tipo
      count[2] = 1; // Identificador do primeiro B
      count[0] += 2; // Incremento para pular para a próxima leitura
    }
    else {
      count[0]++; // Incremento para pular para a próxima leitura
    }

    if (count[1] == 1 && count[2] == 1) {
      count[0] = texto.length(); // Incremento para finalizar leitura
    }
  }

  delay(10000); // Atraso de 10 segundos
}

/*
_______________________________________________Comando_______________________________________________
*/

void comando(int num1, int num2, bool flag) {
  if (num1 == A && !flag) {
    digitalWrite(rele, num2); // Escrita no pino do rele para controle do solenoide A
    flag = true; // Bloqueio para finalização do deslocamento
  }
  else if (num1 == B && !flag) {
    payload[0] = num2; // Carrega os dados para envio
    xbee.send(data); // Envio do dado do pino do rele para controle do solenoide B via Xbee
    flag = true; // Bloqueio para finalização do deslocamento
  }
  else if (flag) {
    digitalWrite(rele, num1); // Escrita no pino do rele para controle do solenoide A
    payload[0] = num2; // Carrega os dados para envio
    xbee.send(data); // Envio do dado do pino do rele para controle do solenoide B via Xbee
  }

  delay(1000); // Atraso de execução de 1 segundo

  // Loop de leitura dos valores dos sensores A e B
  while (flag) {
    requestEvent(); // Requisição de dados do Arduino 2 (Escravo)
    reciveXbee(); // Recebimento de dados de comando via Xbee do Arduino 5

    // Verificador de posição dos valores dos sensores de A e B para Desbloqueio de looop
    if (flag && (a0 == 1 || a1 == 1) && (b0 == 1 || b1 == 1)) {
      flag = false;
    }

    delay(500); // Atraso de 0,5 segundos
  }
}

/*
_______________________________________________Comunicação_______________________________________________
*/

void sendEvent(char texto) {
  Wire.beginTransmission(slave); // Inicialização de conexão com o Arduino 2 (Escravo)

  Wire.write (texto); // Escrita de dados para o arduino 2 (Escravo)
}
  
void requestEvent() {
  Wire.requestFrom(slave,3); // Requisita 3 bytes do Arduino 2 (Escravo)
  
  // Loop de verificação de dados para recepção de dados
  while (Wire.available()) {
    gr = Wire.read(); // Atribuição do valor do grupo na leitura dos dados enviados pelo Arduino 2 (Escravo)
    a0 = Wire.read(); // Atribuição do valor do sensor A0 na leitura dos dados enviados pelo Arduino 2 (Escravo)
    a1 = Wire.read(); // Atribuição do valor do sensor A1 na leitura dos dados enviados pelo Arduino 2 (Escravo)
  }

  /*
  Serial.print(gr, DEC);
  Serial.print("\t");
  Serial.print(a0, DEC);
  Serial.print("\t");
  Serial.println(a1, DEC);
  */
}

void reciveXbee() {
  uint8_t resposta; // Definição de um vetor para armazenar os dados
  xbee.readPacket(); // Leitura do pacote

  // Checagem do recebimento do pacote
  if(xbee.getResponse().isAvailable()){
    if(xbee.getResponse().getApiId()==ZB_RX_RESPONSE){
      xbee.getResponse().getZBRxResponse(recive); // Atribui ao recebimento o pacote
      resposta = recive.getData(0); // Atribui a variável resposta a mensagem
    }
  }

  b0 = ((resposta>>2) & 3); // Atribuição do valor do sensor B0 na leitura dos dados enviados pelo Arduino 5
  b1 = (resposta & 3); // Atribuição do valor do sensor B1 na leitura dos dados enviados pelo Arduino 5
}

/*
_______________________________________________Inicialização_______________________________________________
*/

// Função de retorno da sequência de acionamento de cada grupo
String grupo(int num) {
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
  else {
    return ""; // Retorna vazio (Erro de recebimento)
  }
}