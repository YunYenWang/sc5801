/*  
 *  Get input command & Send command to NBIOT
 *  Notice : need to choise "NL & CR" mode
 */ 

#include <stdio.h> 
#include <string.h> 
#include "SC5801.h"
#include "chttl.h"
#define SDK_VER_MAJOR    0
#define SDK_VER_MINER    6

#define DEBUG_ENABLED    1

#if DEBUG_ENABLED
  #define PRINTF    Report
#else
  #define PRINTF
#endif
  
extern NBIOT nbiot;
SC5801 sc5801;
DeviceSerial  com;

char* cht_iot_apn = "internet.iot";
char * host = "139.162.116.177";
int port = 5801;
int serial_mode = COM_TYPE_RS485;
int serial_baudrate = 115200;

//char imsi[15];
//int size_of_imsi = 15;
//extern unsigned int seq = 1;

void setup() { 
  sc5801.init();
  
  com.SetSerialMode(serial_mode);
  com.begin(serial_baudrate);
  
  PRINTF("\n------ SC5801 SDK Sample Code (V%d.%d) ------------\n\r", SDK_VER_MAJOR, SDK_VER_MINER);
    
  nbiot.SetApn(cht_iot_apn);
  nbiot.SetPinCode("");
  nbiot.SetPinEnable(0);
  nbiot.SetBands(NBIOT_BAND_AUTO);
  nbiot.Connect();

  // TODO
  set_imsi("0123456789ABCDE");
}

//// caller's responsibilty to free memory
//type_pdu* new_pdu(byte function, byte* data, int len) {
//  type_pdu* pdu = (type_pdu*) malloc(sizeof(type_pdu));
//  pdu->len = 2 + size_of_imsi + 1 + 1 + 2 + len + 1;  
//  pdu->payload = (byte*) malloc(pdu->len);
//  
//  int i = 0; 
//  
//  pdu->payload[i++] = 0x16;
//  pdu->payload[i++] = 0xA9;
//  memcpy(pdu->payload + i, imsi, size_of_imsi);
//  i += size_of_imsi;
//  pdu->payload[i++] = seq;
//  seq = (seq + 1) % 0x0FF;
//  
//  pdu->payload[i++] = function;
//  pdu->payload[i++] = (char) ((len & 0x0FF00) >> 8);
//  pdu->payload[i++] = (char) (len & 0x0FF);
//  memcpy(pdu->payload + i, data, len);
//  i += len;
//  pdu->payload[i++] = 0; // TODO - checksum
//
//  return pdu;  
//}
//
//void free_pdu(type_pdu* pdu) {
//  free(pdu->payload);
//  free(pdu);
//}

bool connecting = FALSE;

void loop() {
  static int old_state;

  int state = nbiot.GetState(); // NBIOT_SIM_ERR, NBIOT_PIN_ERR, NBIOT_CLOSE, NBIOT_READY, NBIOT_DAILING, NBIOT_GET_IP, NBIOT_CONNECT, NBIOT_DISCONN
  if (state != old_state) {
    PRINTF("NB-IOT state is changed from %d to %d\r\n", old_state, state);
    old_state = state;

    if (state == NBIOT_GET_IP) { // 3
      if (connecting == FALSE) {
        PRINTF("Establish a UDP connection - %s:%d\r\n", host, port);
        nbiot.UDPConnect(host, port, 0); // 0 is binding port
        connecting = TRUE;
      }
    } else if (state == NBIOT_CLOSE) { // 0
      if (connecting == TRUE) {      
        connecting = FALSE; // reconnect later
  
        PRINTF("NB-IoT is closed\r\n");
      }
    }    
  }

  if (state == NBIOT_CONNECT) {  
    static unsigned long last = 0;
    unsigned long now = millis();
    if ((now - last) > 1000) { // FIXME - to variable      
      char data[256];

      int s = nbiot.RecvUDP(0, data, sizeof(data));
      if (s > 0) {
        data[s] = 0; // zero end string
        PRINTF("RECV - %d\r\n", s);

        for (int i = 0;i < s;i++) {
          PRINTF("%02X ", data[i]);
        }
        PRINTF("\r\n");        
      }
      
      sprintf(data, "%05d", get_seq());
      type_pdu* pdu = new_pdu(1, (byte*) data, strlen(data));

//      for (int i = 0;i < pdu->len;i++) {
//        PRINTF("%02X ", pdu->payload[i]);
//      }
//      PRINTF("\r\n");
      
      nbiot.SendUDP(0, (char*) pdu->payload, pdu->len);
      PRINTF("SEND - %d\r\n", pdu->len);      

      free_pdu(pdu);
      
      last = now;
    }
  }
}

