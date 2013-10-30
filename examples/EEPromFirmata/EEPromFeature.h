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

#ifndef EEPROMFeature_h
#define EEPROMFeature_h

#include <Firmata.h>
#include <utility/FirmataFeature.h>

#define RESERVED_COMMAND 0x00
//subcommands
#define FIRMATA_READ_EEPROM          0
#define FIRMATA_WRITE_EEPROM         1
#define FIRMATA_READ_EEPROM_RESPONSE 2

class EEPromFeature:
public FirmataFeature
{
public:
  void handleCapability(byte pin);
  boolean handlePinMode(byte pin, int mode);
  boolean handleSysex(byte command, byte argc, byte* argv);
  void reset();
  void report();
};

#endif

