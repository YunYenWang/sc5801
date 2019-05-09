/*  
 *  Get input command & Send command to NBIOT
 *  Notice : need to choise "NL & CR" mode
 */ 

#include <stdio.h> 
#include <string.h> 
#include "SC5801.h"
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
int port = 9999;
int serial_mode = COM_TYPE_RS485;
int serial_baudrate = 115200;

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
}

static unsigned int connecting = FALSE;

void loop() {
  static int old_state;

  int state = nbiot.GetState();
  if (state != old_state) {
    PRINTF("NB-IOT state is changed from %d to %d\r\n", old_state, state);
    old_state = state;
  }
  
  static unsigned long last = 0;
  unsigned long now = millis();
  if ((now - last) > 2000) { // FIXME - to variable        
    if (state == NBIOT_GET_IP) { // 3
      if (connecting == FALSE) {
        PRINTF("Create a UDP connection - %s:%d\r\n", host, port);
        nbiot.UDPConnect(host, port, 0); // 0 is binding port
        connecting = TRUE;
      }        
      
    } else if (state == NBIOT_CONNECT) { // 4
      char data[64];
      
      int s = nbiot.RecvUDP(0, data, sizeof(data));
      if (s > 0) {
        data[s] = 0; // zero end string
        PRINTF("RECV - %s\r\n", data);
      }
      
      sprintf(data, "Hello\n");
      nbiot.SendUDP(0, data, strlen(data));
      PRINTF("SEND - %s\r\n", data);
      
    } else if ((state == NBIOT_DISCONN) || (state == NBIOT_CLOSE)) { // 5 or 0
      if (connecting == TRUE) {      
        connecting = FALSE; // reconnect later
  
        PRINTF("NB-IoT is closed\r\n");
      }
    }
    
    last = now;
  }
}

