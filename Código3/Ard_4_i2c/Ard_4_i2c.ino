#include <Arduino.h>
// Wire Master Writer by Nicholas Zambetti http://www.zambetti.com
#include <Wire.h>
#define B0_PIN 5
#define B1_PIN 8
#define ARDUINO_B_SENSOR_I2C_ID 4

int ledState = 0;
int isActive = 1;
int8_t output = -1;

void requestEvent()
{
  int8_t b0 = digitalRead(B0_PIN);
  int8_t b1 = digitalRead(B1_PIN);

  output = b0 * 2 + b1 * 1;

  Wire.write(output);

  Serial.println(output);
}

void setup()
{
  Wire.begin(ARDUINO_B_SENSOR_I2C_ID); // inicia i2c bus - mestre é opcional ter endereço
  Wire.onRequest(requestEvent);
  Serial.begin(115200); // inicia canal serial para saída

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(B0_PIN, INPUT_PULLUP);
  pinMode(B1_PIN, INPUT_PULLUP);
}

void loop() {}