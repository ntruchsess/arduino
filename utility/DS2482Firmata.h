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

class DS2482Firmata:public FirmataFeature
{
public:
  boolean handlePinMode(byte pin, int mode);
  void handleCapability(byte pin);
  boolean handleSysex(byte command, byte argc, byte* argv);
  void reset();

private:
  DS2482 ds2482;
};

#endif


