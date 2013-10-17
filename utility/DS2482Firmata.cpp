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
#include <Wire.h>

boolean DS2482Firmata::handlePinMode(byte pin, int mode)
{
  if (IS_PIN_I2C(pin) && mode == ONEWIRE)
    {
      uint8_t pinmode = Firmata.getPinMode(pin);
      if (pinmode == IGNORE)
        return false;
      if (pinmode != I2C && pinmode != ONEWIRE)
        {
          for (byte i=0; i < TOTAL_PINS; i++)
            {
              if (IS_PIN_I2C(i))
                Firmata.setPinMode(i,ONEWIRE);
            }
        }
      configure(ONEWIRE_CONFIG_FORCE | ONEWIRE_CONFIG_ACTIVE_PULLUP,0);
      return true;
    }
  return false;
}

void DS2482Firmata::handleCapability(byte pin)
{
  if (IS_PIN_I2C(pin))
    {
      Firmata.write(ONEWIRE);
      Firmata.write(2); // To differentiate from Standard (Arduino-only) OneWireFirmata which sends 1
    }
}

boolean DS2482Firmata::handleSysex(byte command, byte argc, byte* argv)
{
  if (command == ONEWIRE_DATA)
    {
      if (argc>1)
        {
          uint8_t subcommand = argv[0];
          uint8_t pin = argv[1];
          uint8_t pinmode = Firmata.getPinMode(pin);
          if (pinmode != I2C && pinmode != ONEWIRE)
            return false;
          switch(subcommand) {
          case ONEWIRE_SEARCH_REQUEST:
          case ONEWIRE_SEARCH_ALARMS_REQUEST:
            {
              Firmata.write(START_SYSEX);
              Firmata.write(ONEWIRE_DATA);
              bool isAlarmSearch = (subcommand == ONEWIRE_SEARCH_ALARMS_REQUEST);
              Firmata.write(isAlarmSearch ? (byte)ONEWIRE_SEARCH_ALARMS_REPLY : (byte)ONEWIRE_SEARCH_REPLY);
              Firmata.write(pin);
              Encoder7Bit.startBinaryWrite();
              uint8_t addrArray[8];
              ds2482.wireResetSearch();
              while (isAlarmSearch ? ds2482.wireSearchAlarms(addrArray) : ds2482.wireSearch(addrArray))
                {
                  for (uint8_t i=0; i < 8 ; i++)
                    Encoder7Bit.writeBinary(addrArray[i]);
                }
              Encoder7Bit.endBinaryWrite();
              Firmata.write(END_SYSEX);
              break;
            }
          case ONEWIRE_CONFIG_REQUEST:
            {
              if (argc>2)
                {
                  uint8_t config = argv[2];
                  uint8_t address = argc > 3 ? argv[3] : 0;
                  return configure(config, address);
                }
              else
                return false;
            }
          default:
            {
              bool power = config & DS2482_CFG_SPU;
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
                          //if configured turn strong pullup on before write of last byte (if no bytes are read after write)
                          if (power && !(subcommand & ONEWIRE_READ_REQUEST_BIT) && i==numBytes-1 && !ds2482.configure(config))
                            break;
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
                          //if configured turn strong pullup on before read of last byte
                          if (power && i==numReadBytes-1 && !ds2482.configure(config))
                            break;
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
            {
              ds2482.reset();
              return false;
            }
        }
      return true;
    }
  return false;
}

void DS2482Firmata::reset()
{
  Wire.begin();
  ds2482.reset();
  config = 0;
  address = 0;
}

bool DS2482Firmata::configure(uint8_t conf, uint8_t addr)
{
  bool changeAddr = (conf & ONEWIRE_CONFIG_FORCE) || (addr & DS2482_FIRMATA_CONFIG_ADDRESS != address & DS2482_FIRMATA_CONFIG_ADDRESS);
  // DS2482_FIRMATA_CONFIG_ADDRESS 0x07 //bits 0-2
  if (changeAddr && !ds2482.detect(addr & DS2482_FIRMATA_CONFIG_ADDRESS))
    return false;
  // DS2482_FIRMATA_CONFIG_CHANNEL 0x38 //bits 3-5
  if ((changeAddr || (addr & DS2482_FIRMATA_CONFIG_CHANNEL != address & DS2482_FIRMATA_CONFIG_CHANNEL))
      && !ds2482.selectChannel((addr & DS2482_FIRMATA_CONFIG_CHANNEL)>>3))
    return false;
  address = addr;
  // DS2482_FIRMATA_CONFIG_MASK    0x0F //bit 0-3 config-bits of ds2482 (see DS2482.h constants DS2482_CFG_xxx)
  uint8_t newconfig = conf & ONEWIRE_CONFIG_STRONG_PULLUP ? DS2482_CFG_SPU : 0;
  if (conf & ONEWIRE_CONFIG_ACTIVE_PULLUP)
    newconfig |= DS2482_CFG_APU;
  if (conf & ONEWIRE_CONFIG_HIGHSPEED)
    newconfig |= DS2482_CFG_1WS;
  if ((changeAddr || (newconfig != config))
      && !ds2482.configure(newconfig))
    return false;
  config = newconfig;
  return true;
}
