#include "panstamp.h"
#include "PanStream.h"

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
REGISTER panStream((byte*)&PanStream.send_message,(byte)sizeof(PanStreamStatusMessage), NULL, NULL);

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

void onStatusReceived(SWPACKET *status);

PanStreamClass::PanStreamClass() {
  panstamp.statusReceived = &onStatusReceived;
  send_message.num_bytes = 0;
  receive_pos = 0;
  receive_len = 0;
  master_id = 0;
  id = 0;
}

size_t PanStreamClass::write(uint8_t c) {

  uint8_t send_len = send_message.num_bytes;
  if (send_len == PANSTREAM_BUFFERSIZE) {
    return 0;
  }
  send_message.send_buffer[send_len++] = c;
  send_message.num_bytes = send_len;
  if (send_len >= PANSTREAM_MAXDATASIZE) {
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
    sendSwapStatus();
  }
};

void PanStreamClass::prepareSendMessage() {

  if (send_message.num_bytes > 0) {
    ++id;
    if (id==0) {
      id++;
    }
    send_message.send_id = id;
  } else {
    send_message.send_id = 0;
  }
}

void PanStreamClass::receiveMessage(PanStreamReceivedMessage* received) {

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
    receive_buffer[(receive_pos + receive_len + i) % PANSTREAM_BUFFERSIZE] = received->data[i];
  }
  receive_len+=receive_bytes;
  sendSwapStatus();
};

void PanStreamClass::sendSwapStatus() {
  SWSTATUS packet = SWSTATUS(REGI_STREAM, (byte*)&send_message, send_message.num_bytes+3);
  packet.send();
};

void onStatusReceived(SWPACKET *status) {
  PanStreamReceivedMessage message;
  byte *data = status->value.data;
  message.received_bytes = data[0];
  message.received_id = data[1];
  message.send_id = data[2];
  message.num_bytes = status->value.length-3;
  message.data = data+3;
  PanStream.receiveMessage(&message);
};

PanStreamClass PanStream;
