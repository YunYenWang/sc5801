#include <stdio.h> 
#include <string.h> 
#include "SC5801.h"
#include "chttl.h"

char imsi[SIZE_OF_IMSI];
unsigned int seq = 1;

void set_imsi(char* s) {
  strncpy(imsi, s, SIZE_OF_IMSI);
}

char* get_imsi() {
  return imsi;
}

unsigned int get_seq() {
  return seq;
}

int checksum(byte* data, int len) {
  int crc = 0;
  for (int i = 0;i < len;i++) {
    crc += (data[i] & 0x0FF);
  }

  return crc;
}

// caller's responsibilty to free memory
type_packet* new_pdu(byte function, byte* data, int len) {
  type_packet* packet = (type_packet*) malloc(sizeof(type_packet));
  packet->len = 2 + SIZE_OF_IMSI + 1 + 1 + 2 + len + 1;  
  packet->payload = (byte*) malloc(packet->len);
  
  int i = 0; 
  
  packet->payload[i++] = PDU_MAGIC_HEAD_HI;
  packet->payload[i++] = PDU_MAGIC_HEAD_LO;
  memcpy(packet->payload + i, imsi, SIZE_OF_IMSI);
  i += SIZE_OF_IMSI;
  packet->payload[i++] = seq;
  seq = (seq + 1) & 0x0FF;
  
  packet->payload[i++] = function;
  packet->payload[i++] = (char) ((len & 0x0FF00) >> 8);
  packet->payload[i++] = (char) (len & 0x0FF);
  memcpy(packet->payload + i, data, len);
  i += len;
  packet->payload[i++] = checksum(data, len) & 0x0FF;

  return packet;  
}

// caller's responsibilty to free memory
type_pdu* read_pdu(byte* packet, int* i, int s, char* err) {
  while (*i < s) {
    if ((packet[(*i)++] == PDU_MAGIC_HEAD_HI) && (packet[(*i)++] == PDU_MAGIC_HEAD_LO)) {
        type_pdu* pdu = (type_pdu*) malloc(sizeof(type_pdu));
        memcpy(pdu->uid, &packet[*i], SIZE_OF_IMSI);
        *i += SIZE_OF_IMSI;
        pdu->seq = packet[(*i)++];
        pdu->function = packet[(*i)++];
        pdu->length = ((packet[(*i)++] & 0x0FF) << 8) | (packet[(*i)++] & 0x0FF);
        if (pdu->length > (s - *i)) { // overflow ?
          sprintf(err, "incorrect length: %d", pdu->length);
          free(pdu);
          return NULL;
        }

        pdu->data = (byte*) malloc(pdu->length);
        memcpy(pdu->data, &packet[*i], pdu->length);
        *i += pdu->length;
        pdu->crc = packet[(*i)++];

        byte crc = checksum(pdu->data, pdu->length) & 0x0FF;
        if (pdu->crc != crc) {
          sprintf(err, "checksum is incorrect");
          free(pdu);
          return NULL;
        }

        return pdu;
    }
  }

  sprintf(err, "No PDU is found");

  return NULL;
}

void free_packet(type_packet* packet) {
  free(packet->payload);
  free(packet);
}

void free_pdu(type_pdu* pdu) {
  free(pdu->data);
  free(pdu);
}
