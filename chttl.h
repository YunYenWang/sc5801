#define SIZE_OF_IMSI 15

#define PDU_PAYLOAD_SIZE 256
#define PDU_MAGIC_HEAD_HI 0x16
#define PDU_MAGIC_HEAD_LO 0xA9

typedef struct {
  byte* payload;
  int len;
} type_packet;

typedef struct {
  char uid[SIZE_OF_IMSI];
  byte seq;
  byte function;
  int length;
  byte* data;
  byte crc;
} type_pdu;

void set_imsi(char* imsi);

char* get_imsi();

unsigned int get_seq();

unsigned int next_seq();

type_packet* new_pdu(int seq, byte function, byte* data, int len);

type_pdu* read_pdu(byte* packet, int* i, int s, char* err);

void free_packet(type_packet* packet);

void free_pdu(type_pdu* pdu);
