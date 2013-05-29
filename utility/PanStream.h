#ifndef _PANSTREAM_H
#define _PANSTREAM_H

#define PANSTREAM_BUFFERSIZE 64

#include "Arduino.h"
#include "register.h"
#include "commonregs.h"

#define SWAP_MANUFACT_ID 1
#define SWAP_PRODUCT_ID 2
#define HARDWARE_VERSION 3
#define FIRMWARE_VERSION 4

/**
 * 
 * Register indexes
 * 
 */

DEFINE_REGINDEX_START()
REGI_STREAM
DEFINE_REGINDEX_END()

struct PanStreamMessage {
  uint8_t received_bytes;
  uint8_t received_id;
  uint8_t send_id;
  uint8_t num_bytes;
  byte buffer[PANSTREAM_BUFFERSIZE];
};

class PanStreamClass : public Stream

{
public:

  PanStreamClass();
  
  size_t write(uint8_t c);
  int available();
  int read();
  int peek();
  void flush();

  PanStreamMessage send_message;
  void setValue(byte *v);

protected:

private:
  byte send_buffer[PANSTREAM_BUFFERSIZE];
  uint8_t send_len;
  byte receive_buffer[PANSTREAM_BUFFERSIZE];
  uint8_t receive_pos;
  uint8_t receive_len;
  uint8_t master_id;
  uint8_t id;
  void prepareSendMessage();
};

extern PanStreamClass PanStream;

#endif

