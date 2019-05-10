#include <string.h> 

#include "SC5801.h"
#include "cellular.h"

#define  DEBUG_MSG      0
#if DEBUG_MSG
  #define COM_PRN     Report 
#else
  #define COM_PRN  
#endif

NBIOT	nbiot;
SC5801 _sc5801;

static IOT_STAT iot_stat=IOT_START;

static char rcv_buf[100];
static int rcv_idx = 0;
static int service=0, conn=0;
static int lte_intit = 0;    //LTE AT Cmd Init.(0/1-UnInit/Inited)
int deactive=2;         //2: for send "AT+COPS=0" at first time.
int data_length=0;
char rcv_data[SIZE_RCV_DATA];

//---- 
void NBIOT_init(void)
{
    nbiot.nbstate = NBIOT_CLOSE;
/*    strcpy(nbiot.nbapn, "internet");
    strcpy(nbiot.nbpinCode, "");
    nbiot.nbpinFlag = 0;
    memset(nbiot.nbipaddr, 0, sizeof(nbiot.nbipaddr));
    nbiot.nblocal_port = 0;
    nbiot.nbdest_port = 0;
    memset(nbiot.nbdest_ip, 0, sizeof(nbiot.nbdest_ip));
    nbiot.nblink_mode = NBIOT_LINK_UDP;
    nbiot.nbband = NBIOT_BAND_NB;
*/    
    nbiot.nbrssi = 99;            //99: No Signal
    iot_stat = IOT_START;
    memset(rcv_data, 0, sizeof(rcv_data));
    data_length = 0;
    _sc5801.SetLED(LED3, 0);
}
// Setup
void NBIOT_setup()
{
    int i;
    
    nbiot.Reset();
    Serial1.begin(115200);
//    //Serial.println("NBIOT_setup");
    //COM_PRN("\nNBIOT_setup.\n\r");
    //--- Initial Variables -----
    NBIOT_init();
    //_sc5801.SetLED(LED1, 1);
    for(i = 0; i < 4; i++) {
      nbiot.nbdest_ip[i] = 0;
    }
    nbiot.nbdest_port = 0;
    //nbiot.nbdo_dail = 0;
    
}

// Loop
void NBIOT_loop()
{
  #define TMR_CHK_NBIOT     2000      //2000ms
  #define TMR_REPORT        8           //8*2000ms
	static unsigned long old_time=0;
	static int report_cnt=TMR_REPORT, run_led=1, wait_cnt=0, lte_led=1;						//
  static int at_round=0;
	unsigned long tmp;
  int i, cnt;
  char str[100], *ptr, *ptr2;
	
	//--- Check Check Time ----
	if ((millis() - old_time) > TMR_CHK_NBIOT) {
    //--- Change LED State -----
    if (run_led)   _sc5801.SetLED(LED1, 1);
    else   _sc5801.SetLED(LED1, 0);
    run_led ^= 1;
		old_time = millis();
    COM_PRN("NBIOT_loop: %d, report_cnt=%d(%d/%d)\r\n", iot_stat, report_cnt, nbiot.nbdo_dail, deactive);
    //-----
    if (at_round >= 2) {        //Module Hang
      nbiot.Reset();
      NBIOT_init();
      deactive = 2;
    }
    //--- Check State -----
    if (deactive == 1) {        //call NBIOT::DisConnect()
      deactive = 2;
      conn = 0;
      Serial1.print("AT+COPS=2\r\n");
//      Serial1.print("AT+GATT=0\r\n");
      delay(2000);
    } else if (nbiot.nbdo_dail == 1 && deactive == 2) {     //Do dail after call NBIOT::DisConnect()
      deactive = 3;
      Serial1.print("AT+COPS=0\r\n");
//      Serial1.print("AT+GATT=1\r\n");
      delay(2000);
      lte_intit = 0;
    }
    if (report_cnt == 0) {    
      at_round++;
      report_cnt = TMR_REPORT;
      Serial1.print("AT+CPSI?\r\n");
      delay(2000);
    } else if (report_cnt == 1) {
      Serial1.print("AT+CSQ\r\n");
      delay(2000);        
    } else if (report_cnt == 2) {
      Serial1.print("AT+CGREG?\r\n");
//      Serial1.print("AT+CGATT?\r\n");
      delay(2000);        
    } 
    report_cnt--;
    
    
		//--- Check Flag ---
		switch(iot_stat) {
		case IOT_START: 								//Initial, Wait "RDY"
      Serial1.print("AT\r\n");
      break;
    case IOT_RDY:                 //Get Ready, Idle
      if (nbiot.nbdo_dail == 0) break;     
//      sprintf(str, "AT+CGDCONT=1,\"IP\",\"%s\";+CNMP=38;+CMNB=%d;+CIPSPRT=2\r\n", nbiot.nbapn, nbiot.nbband); //
      sprintf(str, "AT+CGDCONT=1,\"IP\",\"%s\";+CNMP=38;+CMNB=%d;+CIPSPRT=2\r\n", nbiot.nbapn, nbiot.nbband); //
      Serial1.print(str);
      lte_intit = 1;          //LTE AT Cmd Init
      iot_stat = IOT_CPIN;
      service = 0;
      break;
    case IOT_CPIN:                 //Check PIN
      if (nbiot.nbpinFlag) {      //PIN Enable   
        sprintf(str, "AT+CPIN=%s\r\n", nbiot.nbpinCode);  //
      } else {
        strcpy(str, "AT+CPIN?\r\n");  //
      }
      Serial1.print(str);
      break;
    case IOT_IDLE:                 //Get Ready, Idle
// ###
      conn = 0;    
      break;
    case IOT_SERVICE:              
      iot_stat = IOT_ACT;
      Serial1.print("ATE0\r\n");
      delay(1000);
      strcpy(str, "AT+CIPMUX=0\r\n");  //
      Serial1.print(str);
      break;
    case IOT_ACT:     
      if (service == 1) {         
        strcpy(str, "AT+CGPADDR=1\r\n");  //
        Serial1.print(str);
      } 
      break;
    case IOT_ACTIP1:     
      nbiot.nbstate = NBIOT_GET_IP;        //For ACTIP2~4 Error
      strcpy(str, "AT+CIPSHUT\r\n");  //
      Serial1.print(str);
      break;
    case IOT_ACTIP2_0:     
      strcpy(str, "AT+CGATT?\r\n");
      Serial1.print(str);
      break;
    case IOT_ACTIP2:     
      sprintf(str, "AT+CSTT=\"%s\"\r\n", nbiot.nbapn);
//      sprintf(str, "AT+CSTT=\"%s\"\r\n", "CMNET");
      Serial1.print(str);
      COM_PRN("%s", str);        
      break;
    case IOT_ACTIP3:     
      strcpy(str, "AT+CIICR\r\n");  //
      Serial1.print(str);
      break;
    case IOT_ACTIP4:     
      strcpy(str, "AT+CIFSR\r\n");  //
      Serial1.print(str);
      wait_cnt = 0;
      break;
    case IOT_IP:     
      //--- Check Remote IP Format ----
      if (nbiot.nbdest_ip[0] == 0 || nbiot.nbdest_port == 0)       //Remote IP/Port Not Defined
        break;                  //IP/Port Invalid
      if (conn == 0 && wait_cnt == 0) {
        //sprintf(str, "AT+CLPORT=\"%s\",%d\r\n",(nbiot.nblink_mode==NBIOT_LINK_UDP)?"UDP":"TCP", nbiot.nblocal_port);
        //Serial1.print(str);
        //delay(2000);
        sprintf(str, "AT+CIPSTART=\"%s\",\"%d.%d.%d.%d\",%d\r\n",(nbiot.nblink_mode==NBIOT_LINK_UDP)?"UDP":"TCP", 
                nbiot.nbdest_ip[0], nbiot.nbdest_ip[1], nbiot.nbdest_ip[2], nbiot.nbdest_ip[3], nbiot.nbdest_port);
        Serial1.print(str);
        delay(2000);
        COM_PRN("%s", str);        
      }
      if (wait_cnt++ > 5) {     
        wait_cnt = 0;
        iot_stat = IOT_ACTIP1;    //IOT_DISCONNECT;
        //conn = 1;
      }
      break;
    case IOT_CONNECT:          
      if (lte_led)   _sc5801.SetLED(LED3, 1);
      else   _sc5801.SetLED(LED3, 0);
      lte_led ^= 1;
      
      break;
    case IOT_DISCONNECT:     
      if (conn) {
        Serial1.print("AT+CIPCLOSE\r\n");
        conn = 0;
        iot_stat = IOT_IP;
      } 
      break;
		}
	}

  //Serial1.setTimeout(1000);
  if (Serial1.available())  {  
    memset(rcv_buf, 0, sizeof(rcv_buf));
//    if ((rcv_idx=Serial1.readBytesUntil('\r', rcv_buf, sizeof(rcv_buf)-1))>0) {
    if ((rcv_idx=Serial1.readBytes(rcv_buf, sizeof(rcv_buf)-1))>0) {
      COM_PRN("NBIOT get: %s\r\n", rcv_buf);
      at_round = 0;
      //--- Check Response --------
      if (strstr(rcv_buf, "RDY") || strstr(rcv_buf, "SMS Ready")) {    //Ready
        iot_stat = IOT_RDY;
        nbiot.nbstate = NBIOT_READY;        
        service = 0;
        rcv_buf[0] = '\0';    //Clear Buffer
      } else if (strstr(rcv_buf, "+CPIN:")) {    //+CPIN:
        if (strstr(rcv_buf, "READY")) {    //SIM Ready
            if (iot_stat == IOT_CPIN) iot_stat = IOT_IDLE;
            nbiot.nbstate = NBIOT_READY;        
        } else if (strstr(rcv_buf, "SIM")) {    //PIN Error   
            nbiot.nbstate = NBIOT_PIN_ERR;        
        } else if (strstr(rcv_buf, "NOT READY") || strstr(rcv_buf, "NOT INSERTED")) {    //SIM Error   
            nbiot.nbstate = NBIOT_SIM_ERR;        
        } else {
            nbiot.nbstate = NBIOT_DAILING;        
            //iot_stat = IOT_IDLE;
        }
      } else if (ptr=strstr(rcv_buf, "+CPSI: ")) {    //+CPSI:  <System  Mode>,<Operation  Mode>,<MCC>-<MNC>,<TAC>
                                                      //  ,<SCellID>,<PCellID>,<Frequency  Band>,<earfcn>,<dlbw>,<ulbw>,<RSRQ>,<RSRP>,<RSSI>,<RSSNR> 
        if (ptr2=strstr(ptr, "\r")) {       
          *ptr2 = '\0';
          cnt = 0;
          for(i = strlen(ptr)-1; i > 0; i--) {
            if (ptr[i] == ',') {
              if (++cnt == 3) {       //Got <RSRP>,<RSSI>,<RSSNR>
                ptr = &ptr[i]+1;
                break;
              }
            }
          }
          if (ptr2=strstr(ptr, ",")) {    
            sscanf(ptr, "%d,", &nbiot.nbrsrp);   
          }          
        }
        //--- Period Check, Bypass "OK" ----
        cnt = strlen(rcv_buf);
        if (ptr=strstr(&rcv_buf[cnt+1], "OK")) {    //+CPSI: x,xx   OK
          //*(ptr+3+cnt) = '\0';    
          strcpy(rcv_buf, (ptr+4));
          COM_PRN("Pass OK(%d)=%s, cnt=%d, ptr=%d\r\n", strlen(rcv_buf), rcv_buf, cnt, (ptr-rcv_buf));
        }
      } else if (ptr=strstr(rcv_buf, "+CSQ: ")) {    //+CSQ:
        if (ptr = strstr(&rcv_buf[6], ",")) {
          *ptr = '\0';
          sscanf(&rcv_buf[6], "%d", &nbiot.nbrssi);
        }
        //--- Bypass "OK" ----
        cnt = strlen(rcv_buf);
        if (ptr=strstr(&rcv_buf[cnt+1], "OK")) {    //+CSQ: x,xx   OK
          //*(ptr+3+cnt) = '\0';
          strcpy(rcv_buf, (ptr+4));
          COM_PRN("Pass OK(%d)=%s, cnt=%d, ptr=%d\r\n", strlen(rcv_buf), rcv_buf, cnt, (ptr-rcv_buf));
        }
      } else if (ptr=strstr(rcv_buf, "+CGREG:")) {    //+CGREG:
        if (ptr[10] == '1') {
          _sc5801.SetLED(LED3, 1);
          COM_PRN("CGREG=1, service=%d/%d\r\n", service, iot_stat);
          if (service == 0 ) {
            service = 1;
            iot_stat = IOT_SERVICE;
          }
        } else if (lte_intit == 0) {     //Not Set APN. SIMCOM not get "RDY"
          iot_stat = IOT_RDY;
        } else {
          _sc5801.SetLED(LED3, 0);
          service = 0;
          conn = 0;
          iot_stat = IOT_IDLE;
          nbiot.nbstate = NBIOT_DAILING;       
        }
        //--- Bypass "OK" ----
        if (ptr=strstr(rcv_buf, "OK")) {    //+CGREG: x,xx   OK
          //*(ptr+3) = '\0';
          strcpy(rcv_buf, (ptr+4));
          COM_PRN("Pass OK=%s\r\n", rcv_buf);
        }
        
      } else if (ptr=strstr(rcv_buf, "+CGPADDR: ")) {    //+CGPADDR:
        memset(str, 0, sizeof(str));
        sscanf(rcv_buf, "%*[^,],%[^,]s", str);
        if (ptr=strchr(str, '\r'))  *ptr = '\0';
        strcpy(nbiot.nbipaddr, str);
        COM_PRN("IP addr2=%s\r\n", nbiot.nbipaddr);
        iot_stat = IOT_ACTIP1;
        nbiot.nbstate = NBIOT_GET_IP;        
      } 
      //-----
      if (strstr(rcv_buf, "SHUT OK")) {    //SHUT OK
        iot_stat = IOT_ACTIP2;
//        iot_stat = IOT_ACTIP2_0;
      } else if (strstr(rcv_buf, "+CGATT:")) {    //CGATT Response
        iot_stat = IOT_ACTIP2;
      } else if (strstr(rcv_buf, "CONNECT OK")) { //CONNECT OK
        iot_stat = IOT_CONNECT;
        nbiot.nbstate = NBIOT_CONNECT;        
        conn = 1;
        rcv_buf[0] = '\0';    //Clear Buffer
      } else if (strstr(rcv_buf, "Close OK") || strstr(rcv_buf, "CLOSED")) {   //Close OK or CLOSED
        nbiot.nbstate = NBIOT_DISCONN;        
        iot_stat = IOT_IP;
        conn = 0;
      } else if (ptr=strstr(rcv_buf, "CONNECT OK")) { //CONNECT OK
        iot_stat = IOT_CONNECT;
        conn = 1;
      } else if (strstr(rcv_buf, "OK")) {    //OK
        COM_PRN("rcv_buf OK: %d\r\n", iot_stat);
        switch(iot_stat) {
          case IOT_START:
            nbiot.nbstate = NBIOT_READY;        
            iot_stat = IOT_RDY;
            break;
          case IOT_RDY:         //
            iot_stat = IOT_CPIN;
            break;
          case IOT_ACTIP2:      //at+CSTT Response
            iot_stat = IOT_ACTIP3;
            break;
          case IOT_ACTIP3:      //at+CIICR Response
            iot_stat = IOT_ACTIP4;
            break;
          case IOT_IP:         //Wait Connected
            if (nbiot.nbdest_ip[0] != 0 && nbiot.nbdest_port != 0)       //Remote IP/Port Defined
              iot_stat = IOT_IDLE;
            break;
        }
      } 
      //-----
      if (strstr(rcv_buf, "ERROR")) {    //ERROR
        COM_PRN("rcv_buf ERROR: %d\r\n", iot_stat);
        switch(iot_stat) {
          case IOT_CPIN:        //at+CPIN? Response
            nbiot.nbstate = NBIOT_PIN_ERR;        
            break;
          case IOT_ACTIP2:      //at+CSTT Response
          case IOT_ACTIP3:      //at+CIICR Response
          case IOT_ACTIP4:      //at+CIFSR Response
          case IOT_IP:                //Connect Response
            if (iot_stat == IOT_IP) {
              Serial1.print("AT+CIPCLOSE\r\n");
            }
            iot_stat = IOT_ACTIP1;
            break;
        }
      } else if (strstr(rcv_buf, "CONNECT FAIL")) {    //ERROR
        iot_stat = IOT_ACTIP1;        
      } else if (strncmp(rcv_buf, "AT+CSQ", strlen("AT+CSQ")) &&
                strncmp(rcv_buf, "AT+CGREG?", strlen("AT+CGREG?")) &&
                strncmp(rcv_buf, "AT+CIPSEND", strlen("AT+CIPSEND")) &&
                (strncmp(rcv_buf, "\r\n", strlen("\r\n")) || (strlen(rcv_buf) > 2))) {
          COM_PRN("rcv_buf other: %d, str(%d)=%s\r\n", iot_stat, strlen(rcv_buf), rcv_buf);
         switch (iot_stat) {
           case IOT_ACTIP4:            //"AT+CIFSR" Response
             if (strlen(rcv_buf) < 10) break; //Not IP
             iot_stat = IOT_IP;
             break;
           case IOT_CONNECT:            //Remote Data
             //--- Receive Remote Data ---
// ###             
//             rcv_idx = strlen(rcv_buf);
             if (rcv_idx <= 0) break;
             if ((data_length+rcv_idx) > SIZE_RCV_DATA)  cnt = SIZE_RCV_DATA-data_length;
             else cnt = rcv_idx;
             memcpy(&rcv_data[data_length], rcv_buf, cnt);
             data_length += cnt;
            COM_PRN("Get Remote Data(%d): %s, len=%d\r\n", cnt, rcv_buf,data_length);
             break;
         }
      }  
    }
  }
}

