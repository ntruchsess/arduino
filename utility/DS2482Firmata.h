/*
  DS2482Firmata.h - Firmata library
  Copyright (C) 2012-2013 Norbert Truchsess. All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  See file LICENSE.txt for further informations on licensing terms.
*/

#ifndef DS2482Firmata_h
#define DS2482Firmata_h

#include "DS2482.h"
#include <Firmata.h>
#include <utility/FirmataFeature.h>

//default value for power:
#define ONEWIRE_POWER 1

#define DS2482_FIRMATA_CONFIG_ADDRESS 0x07 //bits 0-2
#define DS2482_FIRMATA_CONFIG_CHANNEL 0x38 //bits 3-5
#define DS2482_FIRMATA_CONFIG_MASK    0x0F //bit 0-3 config-bits of ds2482 (see DS2482.h constants DS2482_CFG_xxx)
#define DS2482_FIRMATA_CONFIG_FORCE   0x40 //bit 6 force ds2482 detect, channelselect and configure

class DS2482Firmata:public FirmataFeature
{
public:
  boolean handlePinMode(byte pin, int mode);
  void handleCapability(byte pin);
  boolean handleSysex(byte command, byte argc, byte* argv);
  void reset();

private:
  DS2482 ds2482;
  uint8_t config;  //bit 0-3: configiguration-bits of ds2482
  uint8_t address; //bit 0-2: i2c-subaddress, bit 3-5: channel (DS2482-800 only)
  bool configure(uint8_t conf, uint8_t addr);
};

#endif


