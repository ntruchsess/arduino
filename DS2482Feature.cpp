/*
  DS2482 library for Arduino
  Copyright (C) 2009-2010 Paeae Technologies

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.

	crc code is from OneWire library
	
	-Updates:
		* fixed wireReadByte busyWait (thanks Mike Jackson)
		* Modified search function (thanks Gary Fariss)
		
*/
#if ARDUINO>=100
#include <Arduino.h> // Arduino 1.0
#else
#include <WProgram.h> // Arduino 0022
#endif

#include "PortsDS2482.h"
#include "Wire.h"

DS2482::DS2482()
: mAddress(DS2482_I2C_ADDR)
{
}

bool DS2482::detect(uint8_t addr)
{
  mAddress = DS2482_I2C_ADDR | addr;
  if (!reset())
    return false;
  return configure(DS2482_CFG_APU);
}

void DS2482::setReadPtr(uint8_t readPtr)
{
  Wire.beginTransmission(mAddress);
  Wire.write(DS2482_CMD_SRP);  // changed from 'send' to 'write' according http://blog.makezine.com/2011/12/01/arduino-1-0-is-out-heres-what-you-need-to-know/'
  Wire.write(readPtr);
  Wire.endTransmission();
}

uint8_t DS2482::readByte()
{
  Wire.requestFrom(mAddress,(uint8_t)1);
  return Wire.read();
}

uint8_t DS2482::wireReadStatus(bool setPtr)
{
  if (setPtr)
    setReadPtr(DS2482_READPTR_SR);

  return readByte();
}

uint8_t DS2482::busyWait(bool setReadPtr)
{
	uint8_t status;
	int loopCount = 1000;
	while((status = wireReadStatus(setReadPtr)) & DS2482_STATUS_1WB)
	{
		if (--loopCount <= 0)
		{
			mTimeout = 1;
			break;
		}
		delayMicroseconds(20);
	}
	return status;
}

//----------interface
bool DS2482::reset()
{
  mTimeout = 0;
  Wire.beginTransmission(mAddress);
  Wire.write(DS2482_CMD_DRST);
  Wire.endTransmission();
  uint8_t result = readByte();

  // check for failure due to incorrect read back of status
  return ((result & 0xf7) == 0x10);
}

bool DS2482::configure(uint8_t config)
{
  busyWait(true);
  Wire.beginTransmission(mAddress);
  Wire.write(DS2482_CMD_WCFG);
  Wire.write(config | (~config << 4));

  if (readByte() != config)
    {
      this->config = config;
      return true;
    }
  reset();
  return false;
}

bool DS2482::selectChannel(uint8_t channel)
{	
  uint8_t ch, ch_read;
  switch (channel) {
  default:
  case 0:
    ch = DS2482_CH_IO0;
    ch_read = DS2482_RCH_IO0;
    break;
  case 1:
    ch = DS2482_CH_IO1;
    ch_read = DS2482_RCH_IO1;
    break;
  case 2:
    ch = DS2482_CH_IO2;
    ch_read = DS2482_RCH_IO2;
    break;
  case 3:
    ch = DS2482_CH_IO3;
    ch_read = DS2482_RCH_IO3;
    break;
  case 4:
    ch = DS2482_CH_IO4;
    ch_read = DS2482_RCH_IO4;
    break;
  case 5:
    ch = DS2482_CH_IO5;
    ch_read = DS2482_RCH_IO5;
    break;
  case 6:
    ch = DS2482_CH_IO6;
    ch_read = DS2482_RCH_IO6;
    break;
  case 7:
    ch = DS2482_CH_IO7;
    ch_read = DS2482_RCH_IO7;
    break;
  }

  Wire.beginTransmission(mAddress);
  Wire.write(DS2482_CMD_CHSL);
  Wire.write(ch);
  Wire.endTransmission();
  return (readByte() == ch_read);
}



bool DS2482::wireReset()
{
  busyWait(true);
  Wire.beginTransmission(mAddress);
  Wire.write(DS2482_CMD_1WRS);
  Wire.endTransmission();
  uint8_t status = busyWait();

  // check for short condition
  shorted = status & DS2482_STATUS_SD;
  // check for presence detect
  return status & DS2482_STATUS_PPD;
}


bool DS2482::wireWriteByte(uint8_t b)
{
  Wire.beginTransmission(mAddress);
  Wire.write(DS2482_CMD_1WWB);
  Wire.write(b);
  Wire.endTransmission();
  if (busyWait() & DS2482_STATUS_1WB)
    {
      reset();
      return false;
    }
  return true;
}

uint8_t DS2482::wireReadByte()
{
  Wire.beginTransmission(mAddress);
  Wire.write(DS2482_CMD_1WRB);
  Wire.endTransmission();
  if (busyWait() & DS2482_STATUS_1WB)
    {
      reset();
      return 0;
    }
  setReadPtr(DS2482_READPTR_RDR);
  return readByte();
}

uint8_t DS2482::wireWriteBit(uint8_t bit)
{
  Wire.beginTransmission(mAddress);
  Wire.write(DS2482_CMD_1WSB);
  Wire.write(bit ? 0x80 : 0);
  Wire.endTransmission();
  if (busyWait() & DS2482_STATUS_1WB)
    {
      reset();
      return 0;
    }
  uint8_t status = busyWait(true);
  return status & DS2482_STATUS_SBR ? 1 : 0;
}

uint8_t DS2482::wireReadBit()
{
  return wireWriteBit(1);
}

void DS2482::wireSkip()
{
  wireWriteByte(SKIP_ROM);
}

void DS2482::wireSelect(uint8_t rom[8])
{
  wireWriteByte(0x55);
  for (int i=0;i<8;i++)
    wireWriteByte(rom[i]);
}


#if ONEWIRE_SEARCH
void DS2482::wireResetSearch()
{
  searchExhausted = 0;
  searchLastDisrepancy = 0;

  for(uint8_t i = 0; i<8; i++)
    searchAddress[i] = 0;
}

uint8_t DS2482::wireSearch(uint8_t *newAddr)
{
  uint8_t i;
  uint8_t direction;
  uint8_t last_zero=0;

  if (searchExhausted)
    return 0;

  if (!wireReset())
    return 0;

  busyWait(true);
  wireWriteByte(DS2482_CMD_DRST);

  for(i=1;i<65;i++)
  {
    int romByte = (i-1)>>3;
    int romBit = 1<<((i-1)&7);

    if (i < searchLastDisrepancy)
      direction = searchAddress[romByte] & romBit;
    else
      direction = i == searchLastDisrepancy;

    busyWait();
    Wire.beginTransmission(mAddress);
    Wire.write(DS2482_CMD_1WT);
    Wire.write(direction ? 0x80 : 0);
    Wire.endTransmission();
    uint8_t status = busyWait();

    uint8_t id = status & DS2482_STATUS_SBR;
    uint8_t comp_id = status & DS2482_STATUS_TSB;
    direction = status & DS2482_STATUS_DIR;

    if (id && comp_id)
      return 0;
    else
      {
        if (!id && !comp_id && !direction)
          last_zero = i;
      }

    if (direction)
      searchAddress[romByte] |= romBit;
    else
      searchAddress[romByte] &= (uint8_t)~romBit;
  }

  searchLastDisrepancy = last_zero;

  if (last_zero == 0)
    searchExhausted = 1;

  for (i=0;i<8;i++)
    newAddr[i] = searchAddress[i];

  return 1;
}
#endif

#if ONEWIRE_CRC
// The 1-Wire CRC scheme is described in Maxim Application Note 27:
// "Understanding and Using Cyclic Redundancy Checks with Maxim iButton Products"
//

uint8_t DS2482::crc8( uint8_t *addr, uint8_t len)
{
  uint8_t crc=0;

  for (uint8_t i=0; i<len;i++)
    {
      uint8_t inbyte = addr[i];
      for (uint8_t j=0;j<8;j++)
        {
          uint8_t mix = (crc ^ inbyte) & 0x01;
          crc >>= 1;
          if (mix)
            crc ^= 0x8C;

          inbyte >>= 1;
        }
    }
  return crc;
}

#endif
