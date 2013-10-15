/*
  DS2482 library for Arduino
  Copyright (C) 2009-2010 Paeae Technologies
  Copyright (C) 2012 martmaiste
  Copyright (C) 2013 Norbert Truchsess

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
*/

#ifndef __DS2482_H__
#define __DS2482_H__

#include <inttypes.h>

// you can exclude onewire_search by defining that to 0
#ifndef ONEWIRE_SEARCH
#define ONEWIRE_SEARCH 1
#endif

// You can exclude CRC checks altogether by defining this to 0
#ifndef ONEWIRE_CRC
#define ONEWIRE_CRC 0
#endif

// constants/macros/typdefs
#define DS2482_I2C_ADDR         0x18    //< Base I2C address of DS2482 devices
#define POLL_LIMIT              0x30    // 0x30 is the minimum poll limit

//1-wire eeprom and silicon serial number commands
#define READ_DEVICE_ROM         0x33
#define SKIP_ROM                0xCC
#define WRITE_SCRATCHPAD        0x0F
#define READ_MEMORY             0xF0
#define COPY_SCRATCHPAD         0x55
#define SEARCH                  0xF0
#define SEARCH_ALARMS           0xEC

// DS2482 command defines
#define DS2482_CMD_DRST         0xF0    //< DS2482 Device Reset
#define DS2482_CMD_SRP          0xE1    //< DS2482 Set Read Pointer
#define DS2482_CMD_WCFG         0xD2    //< DS2482 Write Configuration
#define DS2482_CMD_CHSL         0xC3    //< DS2482 Channel Select
#define DS2482_CMD_1WRS         0xB4    //< DS2482 1-Wire Reset
#define DS2482_CMD_1WWB         0xA5    //< DS2482 1-Wire Write Byte
#define DS2482_CMD_1WRB         0x96    //< DS2482 1-Wire Read Byte
#define DS2482_CMD_1WSB         0x87    //< DS2482 1-Wire Single Bit
#define DS2482_CMD_1WT          0x78    //< DS2482 1-Wire Triplet

// DS2482 status register bit defines
#define DS2482_STATUS_1WB       0x01    //< DS2482 Status 1-Wire Busy
#define DS2482_STATUS_PPD       0x02    //< DS2482 Status Presence Pulse Detect
#define DS2482_STATUS_SD        0x04    //< DS2482 Status Short Detected
#define DS2482_STATUS_LL        0x08    //< DS2482 Status 1-Wire Logic Level
#define DS2482_STATUS_RST       0x10    //< DS2482 Status Device Reset
#define DS2482_STATUS_SBR       0x20    //< DS2482 Status Single Bit Result
#define DS2482_STATUS_TSB       0x40    //< DS2482 Status Triplet Second Bit
#define DS2482_STATUS_DIR       0x80    //< DS2482 Status Branch Direction Taken

// DS2482 configuration register bit defines
#define DS2482_CFG_APU          0x01    //< DS2482 Config Active Pull-Up
#define DS2482_CFG_PPM          0x02    //< DS2482 Config Presence Pulse Masking
#define DS2482_CFG_SPU          0x04    //< DS2482 Config Strong Pull-Up
#define DS2482_CFG_1WS          0x08    //< DS2482 Config 1-Wire Speed

// DS2482 channel selection code for defines
#define DS2482_CH_IO0           0xF0    //< DS2482 Select Channel IO0
#define DS2482_CH_IO1           0xE1    //< DS2482 Select Channel IO1
#define DS2482_CH_IO2           0xD2    //< DS2482 Select Channel IO2
#define DS2482_CH_IO3           0xC3    //< DS2482 Select Channel IO3
#define DS2482_CH_IO4           0xB4    //< DS2482 Select Channel IO4
#define DS2482_CH_IO5           0xA5    //< DS2482 Select Channel IO5
#define DS2482_CH_IO6           0x96    //< DS2482 Select Channel IO6
#define DS2482_CH_IO7           0x87    //< DS2482 Select Channel IO7

// DS2482 channel selection read back code for defines
#define DS2482_RCH_IO0          0xB8    //< DS2482 Select Channel IO0
#define DS2482_RCH_IO1          0xB1    //< DS2482 Select Channel IO1
#define DS2482_RCH_IO2          0xAA    //< DS2482 Select Channel IO2
#define DS2482_RCH_IO3          0xA3    //< DS2482 Select Channel IO3
#define DS2482_RCH_IO4          0x9C    //< DS2482 Select Channel IO4
#define DS2482_RCH_IO5          0x95    //< DS2482 Select Channel IO5
#define DS2482_RCH_IO6          0x8E    //< DS2482 Select Channel IO6
#define DS2482_RCH_IO7          0x87    //< DS2482 Select Channel IO7

// DS2482 read pointer code defines
#define DS2482_READPTR_SR       0xF0    //< DS2482 Status Register
#define DS2482_READPTR_RDR      0xE1    //< DS2482 Read Data Register
#define DS2482_READPTR_CSR      0xD2    //< DS2482 Channel Selection Register
#define DS2482_READPTR_CR       0xC3    //< DS2482 Configuration Register

#define DS2482_STATE_TIMEOUT 1
#define DS2482_STATE_SHORTEND 2
// DS2482 Funtion definition

class DS2482
{
public:

  DS2482 ();

  //Address is 0-3 for DS2482-100 and 0-7 for DS2482-800
  bool detect(uint8_t addr);

  bool configure(uint8_t config);
  bool reset(void);
	
  //DS2482-800 only
  bool selectChannel(uint8_t channel);
	
  bool wireReset(); // return true if presence pulse is detected
  uint8_t wireReadStatus(bool setPtr=false);
	
  void wireWriteByte(uint8_t b);
  uint8_t wireReadByte();
	
  void wireWriteBit(uint8_t bit);
  uint8_t wireReadBit();
  // Issue a 1-Wire rom select command, you do the reset first.
  void wireSelect( uint8_t rom[8]);
  // Issue skip rom
  void wireSkip();
	
  uint8_t getState() { return mState; };

#if ONEWIRE_SEARCH
  // Clear the search state so that if will start from the beginning again.
  void wireResetSearch();

  // Look for the next device. Returns 1 if a new address has been
  // returned. A zero might mean that the bus is shorted, there are
  // no devices, or you have already retrieved all of them.  It
  // might be a good idea to check the CRC to make sure you didn't
  // get garbage.  The order is deterministic. You will always get
  // the same devices in the same order.
  uint8_t wireSearch(uint8_t *newAddr);
  uint8_t wireSearchAlarms(uint8_t *newAddr);
#endif
#if ONEWIRE_CRC
  // Compute a Dallas Semiconductor 8 bit CRC, these are used in the
  // ROM and scratchpad registers.
  static uint8_t crc8( uint8_t *addr, uint8_t len);
#endif

private:
  uint8_t config;
  uint8_t mAddress;
  uint8_t mState;
  uint8_t readByte();
  void setReadPtr(uint8_t readPtr);
	
  uint8_t busyWait(bool setReadPtr=false); //blocks until

#if ONEWIRE_SEARCH
  uint8_t wireSearchInternal(uint8_t command, uint8_t *newAddr);
  uint8_t searchAddress[8];
  uint8_t searchLastDisrepancy;
  uint8_t searchExhausted;
#endif
	
};

#endif
