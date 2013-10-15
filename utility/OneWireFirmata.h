/*
  OneWireFirmata.h - Firmata library
  Copyright (C) 2012-2013 Norbert Truchsess. All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  See file LICENSE.txt for further informations on licensing terms.
*/

#ifndef OneWireFirmata_h
#define OneWireFirmata_h

#include "OneWire.h"
#include <Firmata.h>
#include <utility/FirmataFeature.h>

#define ONEWIRE_CRC 0 //for OneWire.h: crc-functions are not used by Firmata

//default value for power:
#define ONEWIRE_POWER 1

struct ow_device_info
{
  OneWire* device;
  byte addr[8];
  boolean power;
};

class OneWireFirmata:public FirmataFeature
{
public:
  boolean handlePinMode(byte pin, int mode);
  void handleCapability(byte pin);
  boolean handleSysex(byte command, byte argc, byte* argv);
  void reset();
  
private:
  ow_device_info pinOneWire[TOTAL_PINS];
  void oneWireConfig(byte pin, boolean power);
};

#endif


