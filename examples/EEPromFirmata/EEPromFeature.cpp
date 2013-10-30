/*
 * FILE: EEPromFirmata.h
 * VERSION: 0.1
 * PURPOSE: Read/Write Arduino EEProm feature library for Firmata
 * LICENSE: GPL v3 (http://www.gnu.org/licenses/gpl.html)
 *
 * to be used with Firmata
 * URL: http://www.firmata.org
 * uses EEPROMM
 * URL: http://arduino.cc/de/Reference/EEPROM
 *
 * HISTORY:
 * Norbert Truchsess - Original version
 */

#include "EEPromFeature.h"
#include <EEPROM.h>
#include <utility/Encoder7Bit.h>

void EEPromFeature::handleCapability(byte pin)
{
}

boolean EEPromFeature::handlePinMode(byte pin, int mode)
{
  return false;
}

/*
 * byte 0 subcommand
 * byte 1,2 address
 * byte 3 len or data (read-request: len, write-request data)
 * byte 4-end data (write-request)
 * .. alles 7-bitStream encoded
 */
boolean EEPromFeature::handleSysex(byte command, byte argc, byte* argv)
{
  if ( command == RESERVED_COMMAND )
    {
      if ( argc > 4 )
        {
          uint8_t len = num7BitOutbytes(argc-1);
          Encoder7Bit.readBinary(len,argv+1, argv+1); //decode inplace
          int address = argv[1]+argv[2]<<8;
          switch(argv[0])
          {
          case FIRMATA_READ_EEPROM:
            {
              Firmata.write(START_SYSEX);
              Firmata.write(RESERVED_COMMAND);
              Firmata.write(FIRMATA_READ_EEPROM_RESPONSE);
              Encoder7Bit.startBinaryWrite();
              Encoder7Bit.writeBinary(argv[1]);
              Encoder7Bit.writeBinary(argv[2]);
              for (len = argv[3]; len > 0; len--)
                {
                  Encoder7Bit.writeBinary(EEPROM.read(address++));
                }
              Encoder7Bit.endBinaryWrite();
              Firmata.write(END_SYSEX);
              break;
            }
          case FIRMATA_WRITE_EEPROM:
            {
              argv+=3;
              for (len -= 2; len > 0; len--)
                {
                  EEPROM.write(address++,*(argv++));
                }
              break;
            }
          }
        }
      return true;
    }
  return false;
}

void EEPromFeature::reset()
{

}


