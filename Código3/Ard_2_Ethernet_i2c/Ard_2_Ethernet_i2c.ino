#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include "Ethernet.h"
#include "EthernetUdp.h"
#define A0_PIN 3
#define A1_PIN 5
#define ARDUINO_A_SENSOR_I2C_ID 2

EthernetUDP Udp;
byte mac[] = {0x90, 0xA2, 0xDA, 0x00, 0x64, 0x50};
byte ip[] = {192, 168, 0, 44};
unsigned int port = 8888;

char packetBuffer[UDP_TX_PACKET_MAX_SIZE];
int led = LED_BUILTIN;

int8_t output = -1;
uint8_t btState = 0;

void requestEvent()
{
  int8_t a0 = digitalRead(A0_PIN);
  int8_t a1 = digitalRead(A1_PIN);

  output = btState * 4 + a0 * 2 + a1 * 1;

  Wire.write(output);

  Serial.println(output);
}

void setup()
{
  Ethernet.begin(mac, ip);
  Udp.begin(port);
  Serial.begin(115200);
  pinMode(led, OUTPUT);

  Wire.begin(ARDUINO_A_SENSOR_I2C_ID); // inicia i2c bus - mestre é opcional ter endereço
  Wire.onRequest(requestEvent);

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(A0_PIN, INPUT_PULLUP);
  pinMode(A1_PIN, INPUT_PULLUP);

  Serial.println("Escutando Por Requisições");
}

void loop()
{
  int packetSize = Udp.parsePacket();
  if (packetSize)
  {
    Udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
    Serial.println(packetBuffer[0]);
    if (packetBuffer[0] == '1')
    {
      digitalWrite(led, HIGH);
      btState = 1;
    }
    else
    {
      digitalWrite(led, LOW);
      btState = 0;
    }
  }
  delay(10);
}