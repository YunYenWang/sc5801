/*  
 *  Get input command & Send command to NBIOT
 *  Notice : need to choise "NL & CR" mode
 */ 

#include <stdio.h> 
#include <string.h> 
#include "SC5801.h"
#define SDK_VER_MAJOR    0
#define SDK_VER_MINER    6

#define SHOW_DEBUG    1

#if SHOW_DEBUG
  #define PRINTF    Report
#else
  #define PRINTF
#endif
  
extern NBIOT nbiot;
SC5801 sc5801;
DeviceSerial  com;

char* CHT_IOT_APN = "internet.iot";

char * host = "139.162.116.177";
int port = 9999;

char cmd[100];
int idx = 0;
int link_mode=0;        //0/1: UDP/TCP

void setup(){ 
  sc5801.init();
  com.SetSerialMode(COM_TYPE_RS485);
  com.begin(115200);
  Report("\n------ SC5801 SDK Sample Code (V%d.%d) ------------\n\r", SDK_VER_MAJOR, SDK_VER_MINER);

  //---- Initial User Data -----------
  nbiot.SetApn(CHT_IOT_APN);
  nbiot.SetPinCode("");
  nbiot.SetPinEnable(0);
  nbiot.SetBands(NBIOT_BAND_NB);      //1/2: CAT/NB // TODO - NBIOT_BAND_AUTO is better
 
  nbiot.Connect();        //Do Dial
}

void loop(){
  static int old_state;
  int ret;

  int state = nbiot.GetState();
  if (state != old_state) {
    PRINTF("NBIOT state changed from %d to %d\r\n", old_state, state);
    old_state = state;
  }

  unsigned long now = millis();
  if ((now - last) > 2000) {
    static unsigned long last = 0;
    static unsigned connecting = 0;
    if (state == NBIOT_GET_IP) {      
        if (connecting == 0) {
          nbiot.UDPConnect(host, port, 0); // 0 is binding port
          connecting = 1;
        }
      }
    } else if (state == NBIOT_CONNECT) {
      char data[64];
      sprintf(data, "Hello!\r\n");
      nbiot.SendUDP(0, data, strlen(data));
      
      int s = nbiot.RecvUDP(0, data, sizeof(data));
      if (s > 0) {
        PRINTF("RECV - %s\r\n", data);
      }
    }
    
    last = now;
  }
  
//  while(com.available())  {  
//    memset(cmd, 0, sizeof(cmd));
//    if((idx=com.readBytesUntil('\n',cmd,sizeof(cmd)-1))>0) {
//      com.print("COM get: ");
//      com.println(cmd);
//      ret = ProcCmd(cmd);
//      if (0 && ret < 0)           //Command not found
//        Serial1.write(cmd);
//    }
//  }

}

int ProcCmd(char *inp_str)
//--- Command Format: [CMD]+' '+[R/W]+' '+[Data]
{
  char cmd[10], mode, data[20], tmpbuf[100];
  int ret;
  int year, month, day, week, hour, minute, second;
  String valString;
  
  //--- Check Command ([CMD]+' '+[R/W]+' '+[Data])----------
  sscanf(inp_str, "%s %c %s\r\n", cmd, &mode, data);
  PRINTF("ProcCmd: str=%s, cmd=%s, mode=%c, data(%d)=%s\n\r", inp_str, cmd, mode, strlen(data), data); 
  if(strcmp(cmd,"SIM") == 0) {    //SIM State
    if (mode == 'R'|| mode == 'r') {      //Get Sim State
      ret = nbiot.GetSimState(data);
      PRINTF("GetSimState(%d):'%s'  \n\r", ret, data);
    }
  } else if(strcmp(cmd,"APN") == 0) {    //APN
    if (mode == 'W'|| mode == 'w') {      //Write APN
      ret = nbiot.SetApn(data);
      if (ret != 0) {     //Error
        PRINTF("Write APN:'%s' Fail. ret=%d\n\r", data, ret);
      } else {
        PRINTF("Write APN:'%s' success. ret=%d\n\r", data, ret);
      }
    } else {            //Read
      nbiot.GetApn(data);
      PRINTF("Get APN:'%s'  \n\r", data);
    }
  } else if(strcmp(cmd,"UDP") == 0) {    //UDP
    if (mode == 'W'|| mode == 'w') {      //
      link_mode = 0;          //0: UDP
      ret = nbiot.UDPConnect(data, 4660, 0);
    }
  } else if(strcmp(cmd,"TCP") == 0) {    //TCP
    if (mode == 'W'|| mode == 'w') {      //
      link_mode = 1;          //1: TCP
      ret = nbiot.TCPConnect(data, 4660, 0);
    }
  } else if(strcmp(cmd,"SEND") == 0) {    //Send TCP/UDP Data
    if (mode == 'W'|| mode == 'w') {      //
      if (link_mode == 0) 
        ret = nbiot.SendUDP(1, data, strlen(data));
      else
        ret = nbiot.SendTCP(1, data, strlen(data));      
    }
  } else if(strcmp(cmd,"NBIOT") == 0) {    //NBIOT class
    if (mode == 'W'|| mode == 'w') {      //
      if (!strcmp(data, "CONN")) {
        ret = nbiot.Connect();
      } else if (!strcmp(data, "DISCONN")) {
        ret = nbiot.DisConnect();
      }
    }
  } else if(strcmp(cmd,"COM") == 0) {    //COM
    if (mode == 'W'|| mode == 'w') {      //
      delay(2000);
      if (!strcmp(data, "RS232")) {
        com.SetSerialMode(COM_TYPE_RS232);
      } else if (!strcmp(data, "RS485")) {
        com.SetSerialMode(COM_TYPE_RS485);
      }
    }
  } else if(strcmp(cmd,"RTC") == 0) {    //RTC
    if (mode == 'W'|| mode == 'w') {      //SetRTC
      sscanf(data, "%02d%02d%02d%1d%02d%02d%02d", &year, &month, &day, &week, &hour, &minute, &second);
      sc5801.SetRTC(year, month, day, week, hour, minute, second);
    } else if (mode == 'R'|| mode == 'r') {      //GetRTC
      valString = sc5801.GetRTC();
      valString.toCharArray(data, 20);
      PRINTF("Get RTC: %s\n\r", data);
    }
/*    
  } else if(strcmp(cmd,"FILE") == 0) {    //FILE
    if (mode == 'W'|| mode == 'w') {      //writeFile
      ret = sc5801.WriteFile(0, data, strlen(data));
      PRINTF("WriteFile(%d): %s\n\r", ret, data);
    } else if (mode == 'R'|| mode == 'r') {      //ReadFile
      memset(tmpbuf, 0, sizeof(tmpbuf));
      ret = sc5801.ReadFile(0, tmpbuf, strlen(tmpbuf));
      PRINTF("ReadFile(%d): %s\n\r", ret, tmpbuf);
    }
*/    
  }
  
}

