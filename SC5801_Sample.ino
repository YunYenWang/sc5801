/*  
 * NB-IoT Modbus Collector
 * @author: Rick
 */ 

#include <stdio.h> 
#include <stdarg.h> 
#include <string.h> 

#include <WiFi.h>
#include "SC5801.h"
#include "chttl.h"

void xlog(const char* format, ...);

extern NBIOT nbiot;
SC5801 sc5801;
DeviceSerial  com;

char* wifi_ap_ssid;
char* wifi_ap_key = "ienet1308";

WiFiUDP wifi_udp_client;

char* cht_iot_apn = "internet.iot";
char * host = "139.162.116.177";
int port = 5801;
int serial_mode = COM_TYPE_RS485; // TODO - from configuration
int serial_baudrate = 115200;

int reporting_interval = 1000;

void setup() {
  sc5801.init();
  
  com.SetSerialMode(serial_mode);
  com.begin(serial_baudrate);
  
  nbiot.SetApn(cht_iot_apn);
  nbiot.SetPinCode("");
  nbiot.SetPinEnable(0);
  nbiot.SetBands(NBIOT_BAND_AUTO);
  nbiot.Connect();
  
  set_imsi("0123456789ABCDE"); // FIXME - from nbiot

  wifi_ap_setup();
}

void wifi_ap_setup() {
  wifi_ap_ssid = get_imsi();

  sl_Stop(0);
  sl_WlanSetMode(ROLE_AP);
  sl_Start(NULL, NULL, NULL);
  WiFi.beginNetwork(wifi_ap_ssid, wifi_ap_key);

  while (WiFi.localIP() == INADDR_NONE) {
    delay(300);
  }

  wifi_udp_client.begin(0);
}

bool connecting = FALSE;

void loop() {
  static int old_state;

  int state = nbiot.GetState(); // NBIOT_SIM_ERR, NBIOT_PIN_ERR, NBIOT_CLOSE, NBIOT_READY, NBIOT_DAILING, NBIOT_GET_IP, NBIOT_CONNECT, NBIOT_DISCONN
  if (state != old_state) {
    xlog("NB-IOT state is changed from %d to %d", old_state, state);
    old_state = state;

    if (state == NBIOT_GET_IP) { // 3
      if (connecting == FALSE) {
        xlog("Establish a UDP connection - %s:%d", host, port);
        nbiot.UDPConnect(host, port, 0); // 0 is binding port
        connecting = TRUE;
      }
    } else if (state == NBIOT_CLOSE) { // 0
      if (connecting == TRUE) {      
        connecting = FALSE; // cellular will reconnect later
  
        xlog("NB-IoT is closed");
      }
    }    
  }

  if (state == NBIOT_CONNECT) {  
    static unsigned long last = 0;
    unsigned long now = millis();
    if ((now - last) > reporting_interval) {
      char data[PDU_PAYLOAD_SIZE];

      int s = nbiot.RecvUDP(0, data, sizeof(data));
      if (s > 0) {
        xlog("RECV [%d]", s);
        handle_packet(data, s);        
        // DUMP(data, s);
      }
      
      sprintf(data, "%05d", get_seq()); // FIXME - the 'data' is seq number now for test
      type_packet* packet = new_pdu(1, (byte*) data, strlen(data));
      
      nbiot.SendUDP(0, (char*) packet->payload, packet->len);
      xlog("SEND [%d]", packet->len);      
      // DUMP((char*) pdu->payload, pdu->len);

      free_packet(packet);
      
      last = now;
    }
  }  
}
 
void handle_packet(char* packet, int s) {
  int i = 0;
  while (i < s) {
    if ((packet[i++] == PDU_MAGIC_HEAD_HI) && (packet[i++] == PDU_MAGIC_HEAD_LO)) {
      type_pdu* pdu = (type_pdu*) malloc(sizeof(type_pdu));
      memcpy(pdu->uid, &packet[i], SIZE_OF_IMSI);
      i += SIZE_OF_IMSI;
      pdu->seq = packet[i++];
      pdu->function = packet[i++];
      pdu->length = ((packet[i++] & 0x0FF) << 8) | (packet[i++] & 0x0FF);
      pdu->data = (byte*) malloc(pdu->length);
      memcpy(pdu->data, &packet[i], pdu->length);
      i += pdu->length;
      pdu->crc = packet[i++];

      xlog("uid: %s, seq: %d, fun: %d, len: %d, data: %s, crc: %d",
          pdu->uid, pdu->seq, pdu->function, pdu->length, pdu->data, pdu->crc
        );

      free_pdu(pdu);
    }
  }
}

// ======

// void DUMP(char *bytes, int size) {
//   xlog("[%d]", size);
//   // for (int i = 0; i < size; i++) {
//   //   xlog("%02X ", bytes[i]);
//   // }
// }

void xlog(const char* format, ...) {
  int r = 0;
  char *buf;  
  int s = 256;
  va_list list;

  buf = (char*) malloc(s);
  if (buf == NULL) {
    return; // OOM
  }

  for (;;) {
      va_start(list, format);
      r = vsnprintf(buf, s, format, list);
      va_end(list);
      if ((r > -1) && (r < s)) {
        break;

      } else { // OOM
        s = s * 2; // expend
        char *ref;
        if ((ref = (char*) realloc(buf, s)) == NULL) {
          break;

        } else {
          buf = ref;
        }
      }
  }

  wifi_udp_client.beginPacket("255.255.255.255", 514);
  wifi_udp_client.write(buf); // FIXME - character only?
  wifi_udp_client.endPacket();

  free(buf);
}
