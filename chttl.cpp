#include <stdio.h> 
#include <string.h> 
#include "SC5801.h"
#include "chttl.h"

#define SIZE_OF_IMSI 15

char imsi[SIZE_OF_IMSI];
unsigned int seq = 1;

void set_imsi(char* s) {
  strncpy(imsi, s, SIZE_OF_IMSI);
}

unsigned int get_seq() {
  return seq;
}

// caller's responsibilty to free memory
type_pdu* new_pdu(byte function, byte* data, int len) {
  type_pdu* pdu = (type_pdu*) malloc(sizeof(type_pdu));
  pdu->len = 2 + SIZE_OF_IMSI + 1 + 1 + 2 + len + 1;  
  pdu->payload = (byte*) malloc(pdu->len);
  
  int i = 0; 
  
  pdu->payload[i++] = 0x16;
  pdu->payload[i++] = 0xA9;
  memcpy(pdu->payload + i, imsi, SIZE_OF_IMSI);
  i += SIZE_OF_IMSI;
  pdu->payload[i++] = seq;
  seq = (seq + 1) % 0x0FF;
  
  pdu->payload[i++] = function;
  pdu->payload[i++] = (char) ((len & 0x0FF00) >> 8);
  pdu->payload[i++] = (char) (len & 0x0FF);
  memcpy(pdu->payload + i, data, len);
  i += len;
  pdu->payload[i++] = 0; // TODO - checksum

  return pdu;  
}

void free_pdu(type_pdu* pdu) {
  free(pdu->payload);
  free(pdu);
}
