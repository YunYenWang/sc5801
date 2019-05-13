#define PDU_PAYLOAD_SIZE 256

typedef struct {
  byte* payload;
  int len;
} type_pdu;

void set_imsi(char* imsi);

unsigned int get_seq();

type_pdu* new_pdu(byte function, byte* data, int len);

void free_pdu(type_pdu* pdu);

