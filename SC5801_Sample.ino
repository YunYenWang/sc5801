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

#define VERSION 1

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

int rx_interval = 1000;
int heartbeat_interval = 10000;

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
    static unsigned long last_rx = 0;
    static unsigned long last_heartbeat = 0;
    unsigned long now = millis();

    if ((now - last_rx) > rx_interval) {
      char data[PDU_PAYLOAD_SIZE];

      int s = nbiot.RecvUDP(0, data, sizeof(data));
      if (s > 0) {
        xlog("RECV - [%d]", s);
        handle_packet(data, s);        
      }

      last_rx = now;
    }

    if ((now - last_heartbeat) > heartbeat_interval) {
      type_packet* packet = new_packet_heartbeat();      
      nbiot.SendUDP(0, (char*) packet->payload, packet->len);      
      free_packet(packet);
      xlog("SEND - 'Heartbeat'");

      last_heartbeat = now;
    }      
  }  
}

void handle_packet(char* packet, int s) {
  int i = 0;
  while (i < s) {
    char err[128];

    type_pdu* pdu = read_pdu((byte*) packet, &i, s, err);
    if (pdu == NULL) {
      xlog("Error: %s", err);

    } else {
      xlog("Packet funcation: %d", pdu->function);

      if (pdu->function == 1) { // echo
        type_packet* reply = new_packet_echo_reply(pdu->seq, pdu->data, pdu->length);
        nbiot.SendUDP(0, (char*) reply->payload, reply->len);
        free(reply);
        xlog("REPLY - 'Echo'");
      }

      free(pdu);
    }
  }
}

byte* int2bytes(byte* bytes, int value) {
  bytes[0] = (value & 0x0FF00) >> 8;
  bytes[1] = value & 0x0FF;

  return bytes;
}

type_packet* new_packet_heartbeat() {
  int seq = next_seq();
  byte data[6];

  int2bytes(&data[0], nbiot.GetSignalRSSI());
  int2bytes(&data[2], nbiot.GetSignalRSRP());
  int2bytes(&data[4], VERSION);

  return new_pdu(seq, 0, data, 6); // function = 0
}

type_packet* new_packet_echo_reply(int seq, byte* data, int s) {  
  return new_pdu(seq, 1, data, s); // function = 1
}

// ======

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
