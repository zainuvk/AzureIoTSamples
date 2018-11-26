#include <ModbusMaster.h>
#include <SoftwareSerial.h>

ModbusMaster node;

SoftwareSerial mySerial(D7, D8); // RX, TX

void setup()
{
  mySerial.begin(4800);
  Serial.begin(9600);
  // Modbus slave ID 1
  node.begin(1, mySerial);
}

void loop()
{
  uint8_t result;
  node.clearResponseBuffer();
   result = node.readHoldingRegisters(0x40000,2);
  if (result == node.ku8MBSuccess)
  {
    Serial.println("Reading humidity values");
    Serial.println(node.getResponseBuffer(0));//This shows humidity
    Serial.println("Reading temprature values");
    Serial.println(node.getResponseBuffer(1));//This shows temprature
     Serial.println(node.getResponseBuffer(2));  
  }

  if(result==node.ku8MBResponseTimedOut)
  {
    Serial.println("Inside Timeout block ");
  }

  if(result==node.ku8MBInvalidCRC)
  {
    Serial.println("Inside ku8MBInvalidCRC block ");
  }

  if(result==node.ku8MBInvalidFunction)
  {
    Serial.println("Inside ku8MBInvalidFunction block ");
  }

  if(result==node.ku8MBInvalidSlaveID)
  {
    Serial.println("Inside ku8MBInvalidFunction block ");
  }
  
  if(result==node.ku8MBSlaveDeviceFailure)
  {
    Serial.println("Inside ku8MBSlaveDeviceFailure block ");
  }

  if(result==node.ku8MBIllegalDataValue)
  {
    Serial.println("Inside ku8MBIllegalDataValue block ");
  }


  delay(4000);
}
