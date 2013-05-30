#include "panstamp.h"
#include "PanStream.h"

const void setPanStreamValue(byte rId, byte *v);

/**
 * Declaration of common callback functions
 */
DECLARE_COMMON_CALLBACKS()

/**
 * Definition of common registers
 */
DEFINE_COMMON_REGISTERS()

/*
 * Definition of custom registers
 */
REGISTER panStream((byte*)&PanStream.send_message,(byte)sizeof(PanStreamMessage), NULL, &setPanStreamValue);

/**
 * 
 * Initialize table of registers
 * 
 */
DECLARE_REGISTERS_START()
&panStream
DECLARE_REGISTERS_END()

/**
 * 
 * Definition of common getter/setter callback functions
 * 
 */
DEFINE_COMMON_CALLBACKS()

PanStreamClass::PanStreamClass() {
  send_len = 0;
  receive_pos = 0;
  receive_len = 0;
  master_id = 0;
  id = 0;
}

size_t PanStreamClass::write(uint8_t c) {

  if (send_len == PANSTREAM_BUFFERSIZE) {
    return 0;
  }
  send_buffer[send_len++] = c;
  if (send_len == PANSTREAM_BUFFERSIZE) {
    flush();
  }
  return 1;
};



int PanStreamClass::available() {

  return receive_len;
};



int PanStreamClass::read() {

  if (receive_len == 0) return -1;
  byte ret = receive_buffer[receive_pos++];
  if (receive_pos == PANSTREAM_BUFFERSIZE) receive_pos=0;
  receive_len--;
  return ret;
};



int PanStreamClass::peek() {

  if (receive_len == 0) return -1;
  return receive_buffer[receive_pos];
};



void PanStreamClass::flush() {

  if (send_message.send_id==0) {
    prepareSendMessage();
    getRegister(REGI_STREAM)->getData();
  }
};



void PanStreamClass::prepareSendMessage() {

  for (uint8_t i=0;i<send_len;i++) {
    send_message.buffer[i]=send_buffer[i];
  }
  send_message.num_bytes = send_len;
  if (send_len > 0) {
    ++id;
    if (id==0) {
      id++;
    }
    send_message.send_id = id;
  } else {
    send_message.send_id = 0;
  }
  send_len = 0;
}



void PanStreamClass::setValue(byte* v) {

  PanStreamMessage* received = (PanStreamMessage*)v;
  if (received->send_id==master_id) {
    return; // received this packet before, just send acknowledge again
  }
  if (received->received_id==send_message.send_id) { //previous package acknowledged by master -> prepare new package send data
    prepareSendMessage();
  }
  master_id = received->send_id;
  send_message.received_id = master_id; //acknowledge package
  uint8_t receive_bytes = //acknowledge number of bytes transfered to receive_buffer
      (received->num_bytes + receive_len > PANSTREAM_BUFFERSIZE) ?
          PANSTREAM_BUFFERSIZE - receive_len : received->num_bytes;
  send_message.received_bytes = receive_bytes;
  for (uint8_t i = 0; i < receive_bytes; i++) {
    receive_buffer[(receive_pos + receive_len + i) % PANSTREAM_BUFFERSIZE] = received->buffer[i];
  }
  receive_len+=receive_bytes;
};

const void setPanStreamValue(byte rId, byte *v) {
  PanStream.setValue(v);
};

PanStreamClass PanStream;
