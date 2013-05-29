#include <panstamp.h>
#include <EEPROM.h>
#include <Firmata.h>
#include <utility/PanStream.h>
//#include "HardwareSerial.h"
#include <Wire.h>  // remove this after first compile
#include <Servo.h> // remove this after first compile

/**
 * LED pin
 */
#define LEDPIN 4

void setup()
{
//  Serial.begin(9600);
//  Serial.println("started");
  // Init panStamp
  panstamp.init();

  // Transmit product code
  getRegister(REGI_PRODUCTCODE)->getData();

  // Enter SYNC state
  panstamp.enterSystemState(SYSTATE_SYNC);

  // During 3 seconds, listen the network for possible commands whilst the LED blinks
  for(uint8_t i=0 ; i<6 ; i++)
  {
    digitalWrite(LEDPIN, HIGH);
    delay(100);
    digitalWrite(LEDPIN, LOW);
    delay(400);
  }
  // Transmit periodic Tx interval
  getRegister(REGI_TXINTERVAL)->getData();
  
  // Switch to Rx OFF state
  panstamp.enterSystemState(SYSTATE_RXOFF);
}

void loop()
{
  // Sleep for panstamp.txInterval seconds (register 10)
  panstamp.goToSleep();

  PanStream.println("Hello World");
//  while (PanStream.available()>0) {
//    Serial.print(PanStream.read());
//  }
}
