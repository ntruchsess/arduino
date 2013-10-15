/*
  DS2482Firmata.cpp - Firmata library
  Copyright (C) 2012-2013 Norbert Truchsess. All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  See file LICENSE.txt for further informations on licensing terms.
*/

#include <Firmata.h>
#include <DS2482Firmata.h>
#include <OneWireFirmataCommands.h>
#include <Encoder7Bit.h>

boolean DS2482Firmata::handlePinMode(byte pin, int mode)
{
  if (IS_PIN_I2C(pin) && mode == ONEWIRE)
    {
      //oneWireConfig(pin,ONEWIRE_POWER);
      return true;
    }
  return false;
}

void DS2482Firmata::handleCapability(byte pin)
{
  if (IS_PIN_I2C(pin))
    {
      Firmata.write(ONEWIRE);
      Firmata.write(1);
    }
}

boolean DS2482Firmata::handleSysex(byte command, byte argc, byte* argv)
{
  if (command == ONEWIRE_DATA)
    {
      if (argc>1)
        {
          byte subcommand = argv[0];
          byte pin = argv[1];
          switch(subcommand) {
          case ONEWIRE_SEARCH_REQUEST:
          case ONEWIRE_SEARCH_ALARMS_REQUEST:
            {
              Firmata.write(START_SYSEX);
              Firmata.write(ONEWIRE_DATA);
              boolean isAlarmSearch = (subcommand == ONEWIRE_SEARCH_ALARMS_REQUEST);
              Firmata.write(isAlarmSearch ? (byte)ONEWIRE_SEARCH_ALARMS_REPLY : (byte)ONEWIRE_SEARCH_REPLY);
              Firmata.write(pin);
              Encoder7Bit.startBinaryWrite();
              uint8_t addrArray[8];
              ds2482.wireResetSearch();
              while (isAlarmSearch ? ds2482.wireSearchAlarms(addrArray) : ds2482.wireSearch(addrArray))
                {
                  for (int i=0;i<8;i++)
                    Encoder7Bit.writeBinary(addrArray[i]);
                }
              Encoder7Bit.endBinaryWrite();
              Firmata.write(END_SYSEX);
              break;
            }
          case ONEWIRE_CONFIG_REQUEST:
            {
              if (argc==3 && Firmata.getPinMode(pin)!=IGNORE)
                {
                  //TODO check whether we should call Firmata.setPinMode(pin,I2C) here
                  //TODO implement suitable oneWireConfig
                  //oneWireConfig(pin, argv[2]); // this calls oneWireConfig again, this time setting the correct config (which doesn't cause harm though)
                } else {
                  return false;
                }
              break;
            }
          default:
            {
              if (subcommand & ONEWIRE_RESET_REQUEST_BIT)
                {
                  if (!ds2482.wireReset())
                    break;
                }
              if (subcommand & ONEWIRE_SKIP_REQUEST_BIT)
                {
                  ds2482.wireSkip();
                  if (ds2482.getState())
                    break;
                }
              if (subcommand & ONEWIRE_WITHDATA_REQUEST_BITS)
                {
                  int numBytes=num7BitOutbytes(argc-2);
                  int numReadBytes=0;
                  int correlationId;
                  argv+=2;
                  Encoder7Bit.readBinary(numBytes,argv,argv); //decode inplace

                  if (subcommand & ONEWIRE_SELECT_REQUEST_BIT)
                    {
                      if (numBytes<8) break;
                      ds2482.wireSelect(argv);
                      if (ds2482.getState())
                        break;
                      argv+=8;
                      numBytes-=8;
                    }

                  if (subcommand & ONEWIRE_READ_REQUEST_BIT)
                    {
                      if (numBytes<4) break;
                      numReadBytes = *((int*)argv);
                      argv+=2;
                      correlationId = *((int*)argv);
                      argv+=2;
                      numBytes-=4;
                    }

                  if (subcommand & ONEWIRE_DELAY_REQUEST_BIT)
                    {
                      if (numBytes<4) break;
                      Firmata.delayTask(*((long*)argv));
                      argv+=4;
                      numBytes-=4;
                    }

                  if (subcommand & ONEWIRE_WRITE_REQUEST_BIT)
                    {
                      for (int i=0;i<numBytes;i++)
                        {
                          ds2482.wireWriteByte(argv[i]);
                          if (ds2482.getState())
                            break;
                        }
                    }

                  if (numReadBytes>0)
                    {
                      Firmata.write(START_SYSEX);
                      Firmata.write(ONEWIRE_DATA);
                      Firmata.write(ONEWIRE_READ_REPLY);
                      Firmata.write(pin);
                      Encoder7Bit.startBinaryWrite();
                      Encoder7Bit.writeBinary(correlationId&0xFF);
                      Encoder7Bit.writeBinary((correlationId>>8)&0xFF);
                      for (int i=0; i<numReadBytes; i++)
                        {
                          uint8_t val = ds2482.wireReadByte();
                          if (ds2482.getState())
                            break;
                          Encoder7Bit.writeBinary(val);
                        }
                      Encoder7Bit.endBinaryWrite();
                      Firmata.write(END_SYSEX);
                    }
                }
            }
          }
          if (ds2482.getState())
            ds2482.reset();
        }
      return true;
    }
  return false;
}

void DS2482Firmata::reset()
{
  ds2482.reset();
}
