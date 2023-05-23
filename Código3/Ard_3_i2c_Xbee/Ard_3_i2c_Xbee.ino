#include <Arduino.h>
#include <Wire.h>
#include <XBee.h>
#define LED_PORT LED_BUILTIN
#define VALVE_PORT 8              // port to activate a spring returned solenoid control valve
#define ARDUINO_A_SENSOR_I2C_ID 2 // address of arduino4 in I2C communication with arduino 5
#define XBEE_B_ADDRESS_SH 0x0013A200
#define XBEE_B_ADDRESS_SL 0x40C1B208

XBee xbee = XBee();
ZBRxResponse rx = ZBRxResponse();

// Specify the address of the remote XBee (this is the SH + SL)
XBeeAddress64 add64OfB = XBeeAddress64(XBEE_B_ADDRESS_SH,
                                       XBEE_B_ADDRESS_SL);

int aState = 0;
uint8_t step = 0;
uint8_t stepB = 0;
int ledPortState = 0;
int ledBuiltinState = 0;
int SEQ_A[5] = {1, -1, 0, 1, -1}; // SEQ = { A+, A-, B+, A+B- , A- }
int SEQ_A_SIZE = 5;

/**
 * Turns the rawState received by the Arduino in code
 * and turn it into piston commands (1, 0, -1)
 */
int stateToPositionMap(int rawState)
{
  if (rawState == 4)
    return 0;
  else if (rawState == 5)
    return 1;
  else if (rawState == 6)
    return -1;
}

/**
 * Because the system is automatic, the steps return to zero
 * after the steps hit max value
 */
void nextStep()
{
  delay(1000);
  if (step != SEQ_A_SIZE - 1)
  {
    step++;
  }
  else
  {
    step = 0;
  }
}

/**
 * Gets state of the B piston from Arduino 4
 */
int8_t getAState()
{
  int8_t temp;

  Wire.requestFrom(ARDUINO_A_SENSOR_I2C_ID, 8); // request 8 bytes from peripheral device ARDUINO_A_SENSOR_I2C_ID

  // peripheral may send less than requested
  temp = Wire.read(); // receive a byte as character

  return temp;
}

void sendStepToB()
{
  ZBTxRequest data = ZBTxRequest(add64OfB, &step, sizeof(step));
  xbee.send(data);
}

int8_t getStepFromB()
{
  xbee.readPacket();
  if (xbee.getResponse().isAvailable())
  {
    if (xbee.getResponse().getApiId() == ZB_RX_RESPONSE)
    {
      xbee.getResponse().getZBRxResponse(rx);
      stepB = rx.getData(0);
      Serial.println("stepB: " + stepB);
    }
  }

  return stepB;
}

void pistonForward()
{
  digitalWrite(LED_BUILTIN, 1);
  digitalWrite(VALVE_PORT, 0);
}

void pistonReturn()
{
  digitalWrite(LED_BUILTIN, 0);
  digitalWrite(VALVE_PORT, 1);
}

/**
 * Recieves a piston command (1, 0, -1) and
 * execute the correct move in the piston
 */
void movePiston(int command)
{
  // do nothing
  if (command == 0)
    return;

  // go forward
  if (command == 1)
  {
    pistonForward();
  }
  // return
  else if (command == -1)
  {
    pistonReturn();
  }
}

void setup()
{
  Serial.begin(9600);
  Wire.begin();
  // xbee.setSerial(Serial);

  pinMode(LED_PORT, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(VALVE_PORT, OUTPUT);
}

void loop()
{
  if (getStepFromB() == -1)
  {
    return;
  }

  int8_t aStateCode;

  aStateCode = getAState();

  aState = stateToPositionMap(aStateCode);

  Serial.print("aStateCode: ");
  Serial.print(aStateCode);
  Serial.print(", step: ");
  Serial.print(step);
  Serial.print(", SEQ_A[step]: ");
  Serial.print(SEQ_A[step]);
  Serial.print(", aState: ");
  Serial.println(aState);

  // if the state of the piston is not equal to the required state
  // send signal to go to that state
  if (SEQ_A[step] != 0 && SEQ_A[step] != aState)
  {
    movePiston(SEQ_A[step]);
    return; // the equivalent of continue for the arduino loop function
  }

  nextStep();

  while (step != stepB)
  {
    sendStepToB();
    getStepFromB();
  }
}