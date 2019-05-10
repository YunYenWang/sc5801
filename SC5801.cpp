#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <stdarg.h>
#include <Wire.h>
#include <SPI.h>
#include <WiFi.h>
#include <ti/drivers/net/wifi/fs.h>
//#include "Energia.h"
#include "SC5801.h"

extern int deactive;
extern int data_length;
extern char rcv_data[1];
extern SC5801 _sc5801;

extern void NBIOT_init(void);
extern void RTC_Init(void);
extern byte RTC_Read(byte offset);
extern void RTC_Write(byte offset, byte val);

static byte sccom_mode;
static byte com_led=1;


void SC5801::init(void) 
{
  Wire.begin();  
  SetLED(LED1 | LED2 | LED3, 1);  
  
  pinMode(MODE0_RS, OUTPUT); 
  digitalWrite(MODE0_RS, 1);
  
  pinMode(TTL_CTL, OUTPUT);
  digitalWrite(TTL_CTL, 0);
  
  pinMode(TERM_RS485, OUTPUT);
  digitalWrite(TERM_RS485,0);
  
  delay(300);
  pinMode(MCU_PWR, OUTPUT);
  digitalWrite(MCU_PWR, 1);	//
  
  pinMode(MCU_RST, OUTPUT);
  digitalWrite(MCU_RST, 1);
  delay(300);
  digitalWrite(MCU_RST, 0);
  
  pinMode(MCU_BAT_RST, OUTPUT);
  digitalWrite(MCU_BAT_RST, 0);
 
  pinMode(UART0_RTS_DIR1, OUTPUT);
  digitalWrite(UART0_RTS_DIR1, 1);

  pinMode(EXP_INT_MCU, INPUT);

  RTC_Init();
  //--- Default RS485 Mode -------
#if 0  
  digitalWrite(MODE0_RS, 0);
  digitalWrite(TERM_RS485, 1);
  digitalWrite(UART0_RTS_DIR1, 0);
  sccom_mode = COM_TYPE_RS485;
#endif  
  SetLED(LED1 | LED2 | LED3, 0);  
}

void SC5801::SetRTC(byte year, byte month, byte day, byte week, byte hour, byte minute, byte second)
{
  RTC_Write(YEAR,year);
  RTC_Write(MONTH,month);
  RTC_Write(DATE,day);
  RTC_Write(WEEK,week);
  RTC_Write(HOUR,hour);
  RTC_Write(MIN,minute);
  RTC_Write(SEC,second); 
  
}
String SC5801::GetRTC(void)
{
  char tmp[20];
  String rtc_val;
  byte year, month, day, week, hour, minute, second;

  year = RTC_Read(YEAR);
  month = RTC_Read(MONTH);
  day = RTC_Read(DATE);
  week = RTC_Read(WEEK);
  hour = RTC_Read(HOUR);
  minute = RTC_Read(MIN);
  second = RTC_Read(SEC);
  sprintf(tmp, "%02d%02d%02d%1d%02d%02d%02d", year, month, day, week, hour, minute, second);
  //Serial.println(tmp);
  rtc_val = tmp;

  return rtc_val;
}
void SC5801::SetLED(byte ledIdx, byte ledState)
{
  byte c;
  
  Wire.requestFrom(I2C_GPIO_ADDR, 1);    // request 1 bytes from slave device #2
  while(Wire.available()) {    // slave may send less than requested
    c = Wire.read(); // receive a byte as character
  }

  Wire.beginTransmission(I2C_GPIO_ADDR);
  if (ledState) {
    Wire.write(c & (~ledIdx));
  } else {
    Wire.write(c | ledIdx);
  }
  Wire.endTransmission();
 
}
/*
int SC5801::WriteFile(int offset, char *buffer, int len)
{
  long    DeviceFileHandle = -1;
  long    RetVal;        //negative retval is an error
  uint32_t  MasterToken = 0;

  DeviceFileHandle = sl_FsOpen((unsigned char*)NAME_USER_FILE, 
                              SL_FS_CREATE | SL_FS_OVERWRITE | SL_FS_CREATE_NOSIGNATURE | SL_FS_CREATE_MAX_SIZE( SIZE_FILE_SPACE ), 0);
  Report("Write sl_FsOpen: file(%ld)=%s, MaxSize=%d\r\n",DeviceFileHandle, NAME_USER_FILE, SIZE_FILE_SPACE);    
  if (0 && DeviceFileHandle < 0) {
    Serial.print("ERROR creating /storage/mine.txt!  Error code: ");
    Serial.println(DeviceFileHandle);  // Report meaning of last SLFS API failure

    Serial.println("Halting.");
    //while(1) delay(1000);  // Infinite loop to stop the program
    return 0;
  } 

  // Write text to the file-
  RetVal = sl_FsWrite(DeviceFileHandle, offset, (unsigned char *)buffer, len);
//  Report("sl_FsWrite: ret=%d\r\n", RetVal);    
  RetVal = sl_FsClose(DeviceFileHandle, NULL, NULL , 0);
//  Report("sl_FsClose: ret=%d\r\n", RetVal);    

  return RetVal;
}

int SC5801::ReadFile(int offset, char *buffer, int buf_size)
{
  char    buf[1024];  // our RAM buffer to house the file's contents  
  long    DeviceFileHandle = -1;
  long    RetVal;        //negative retval is an error
  uint32_t  MasterToken = 0;

  DeviceFileHandle = sl_FsOpen((unsigned char*)NAME_USER_FILE, SL_FS_READ, 0);
  Report("Read: sl_FsOpen: file(%ld)=%s, MaxSize=%d\r\n",DeviceFileHandle, NAME_USER_FILE, SIZE_FILE_SPACE);    
  if (0 && DeviceFileHandle < 0) {
    // Some sort of error occurred!
    Serial.print("ERROR opening /storage/mine.txt!  Error code: ");
    Serial.println(DeviceFileHandle);
    Serial.println("Halting.");
//    while (1) delay(1000);  // Infinite loop to stop the program
    return 0;

  } 

  memset(buf, 0, sizeof(buf));
  RetVal = sl_FsRead( DeviceFileHandle, offset, (unsigned char *)buffer, buf_size);
  Report("sl_FsRead: ret=%d, data=%s\r\n", RetVal, buf);    
  if (RetVal >= 0) {
      Serial.print("Read ");
      Serial.print(RetVal);
      Serial.println(" bytes from /storage/mine.txt - displaying contents:");

      Serial.print(buffer);
      RetVal = sl_FsClose(DeviceFileHandle, NULL, NULL , 0);  
      Report("sl_FsClose: ret=%d\r\n", RetVal);    
  } else {
    Serial.print("There was an error reading from the /storage/mine.txt file! Error code: ");
    Serial.println(RetVal);
  }

  return RetVal;

}
*/
void Timer::begin(void (*timerFunction)(void), uint32_t timerPeriod_unit, uint32_t unit)
{
    Error_Block eb;
    Error_init(&eb);

    // xdc_UInt TimerId = 3; // OK=3, NOK=2,1,0 MSP432=4 timers, only timer 3 available
    // Timer_ANY to take any available timer
    xdc_UInt TimerId = Timer_ANY;
    
    Timer_Params params;
    Timer_Params_init(&params);
    params.periodType = Timer_PeriodType_MICROSECS;
    params.period = timerPeriod_unit * unit; // 1 ms = 1000 us
    params.startMode = Timer_StartMode_USER; // Timer_start

    TimerHandle = Timer_create(TimerId, (Timer_FuncPtr)timerFunction, &params, &eb);

    if (TimerHandle == NULL)
    {
        // Serial.println("*** Timer create failed");
        System_abort("Timer create failed");
    }
}

void Timer::start()
{
    Timer_start(TimerHandle);
}

void Timer::stop()
{
    Timer_stop(TimerHandle);
}

//--- DeviceSerial (COM) ------------
//#define DeviceSerial    Serial
void DeviceSerial::SetSerialMode(int mode)
{
 // Serial.end();
  if (mode == COM_TYPE_RS232) {
    digitalWrite(MODE0_RS, 1);
    digitalWrite(TERM_RS485, 0);
    digitalWrite(UART0_RTS_DIR1, 1);
    sccom_mode = COM_TYPE_RS232;
  } else {
    digitalWrite(MODE0_RS, 0);
    digitalWrite(TERM_RS485, 1);
    digitalWrite(UART0_RTS_DIR1, 0);
    sccom_mode = COM_TYPE_RS485;
  }       
//  Serial.begin(115200);
} 
  
int DeviceSerial::GetSerialMode(void)
{
  return sccom_mode;
}   

void DeviceSerial::begin(unsigned long speed)
{
  Serial.begin(speed);
}   
void DeviceSerial::begin(unsigned long speed, int config)
{
  Serial.begin(speed, config);
}   
int DeviceSerial::available(void)
{
  return Serial.available();
}   
void DeviceSerial::end(void)
{
  Serial.end();
}   
int DeviceSerial::print(const String &val)
{
	int ret;  
 
  _sc5801.SetLED(LED2, (com_led ^= 1));
	if (sccom_mode == COM_TYPE_RS485)  digitalWrite(UART0_RTS_DIR1, 1);
  ret = Serial.print(val);
  if (sccom_mode == COM_TYPE_RS485) {
    delay(val.length());
    digitalWrite(UART0_RTS_DIR1, 0);
    _sc5801.SetLED(LED2, (com_led ^= 1));
  }
  return ret;

}   
extern SC5801 sc5801;
int DeviceSerial::print(const char *val)
{
	int ret;  
	
  _sc5801.SetLED(LED2, (com_led ^= 1));
	if (sccom_mode == COM_TYPE_RS485)  digitalWrite(UART0_RTS_DIR1, 0);
  ret = Serial.print(val);
  if (sccom_mode == COM_TYPE_RS485) {
    delay(strlen(val));
    digitalWrite(UART0_RTS_DIR1, 0);
    _sc5801.SetLED(LED2, (com_led ^= 1));
  }
  return ret;
}   
int DeviceSerial::print(char val)
{
	int ret;  
	
  _sc5801.SetLED(LED2, (com_led ^= 1));
	if (sccom_mode == COM_TYPE_RS485)  digitalWrite(UART0_RTS_DIR1, 1);
  ret = Serial.print(val);
  if (sccom_mode == COM_TYPE_RS485) {
    delay(1);
    digitalWrite(UART0_RTS_DIR1, 0);
  }
  return ret;
}   
int DeviceSerial::print(unsigned char val, int base)
{
	int ret;  
	
  _sc5801.SetLED(LED2, (com_led ^= 1));
	if (sccom_mode == COM_TYPE_RS485)  digitalWrite(UART0_RTS_DIR1, 1);
  ret = Serial.print(val, base);
  if (sccom_mode == COM_TYPE_RS485) {
    delay(1);
    digitalWrite(UART0_RTS_DIR1, 0);
  }
  return ret;
}

int DeviceSerial::print(int val, int base)
{
	int ret;  
	
  _sc5801.SetLED(LED2, (com_led ^= 1));
	if (sccom_mode == COM_TYPE_RS485)  digitalWrite(UART0_RTS_DIR1, 1);
  ret = Serial.print(val, base);
  if (sccom_mode == COM_TYPE_RS485) {
    delay(1);
    digitalWrite(UART0_RTS_DIR1, 0);
  }
  return ret;
}

int DeviceSerial::print(unsigned int val, int base)
{
	int ret;  
	
  _sc5801.SetLED(LED2, (com_led ^= 1));
	if (sccom_mode == COM_TYPE_RS485)  digitalWrite(UART0_RTS_DIR1, 1);
  ret = Serial.print(val, base);
  if (sccom_mode == COM_TYPE_RS485) {
    delay(1);
    digitalWrite(UART0_RTS_DIR1, 0);
  }
  return ret;
}

int DeviceSerial::print(long val, int base)
{
	int ret;  
	
  _sc5801.SetLED(LED2, (com_led ^= 1));
	if (sccom_mode == COM_TYPE_RS485)  digitalWrite(UART0_RTS_DIR1, 1);
  ret = Serial.print(val, base);
  if (sccom_mode == COM_TYPE_RS485) {
    delay(1);
    digitalWrite(UART0_RTS_DIR1, 0);
  }
  return ret;
}

int DeviceSerial::print(unsigned long val, int base)
{
	int ret;  
	
  _sc5801.SetLED(LED2, (com_led ^= 1));
	if (sccom_mode == COM_TYPE_RS485)  digitalWrite(UART0_RTS_DIR1, 1);
  ret = Serial.print(val, base);
  if (sccom_mode == COM_TYPE_RS485) {
    delay(1);
    digitalWrite(UART0_RTS_DIR1, 0);
  }
  return ret;
}

int DeviceSerial::print(double val, int digits)
{
	int ret;  
	
  _sc5801.SetLED(LED2, (com_led ^= 1));
	if (sccom_mode == COM_TYPE_RS485)  digitalWrite(UART0_RTS_DIR1, 1);
  ret = Serial.print(val, digits);
  if (sccom_mode == COM_TYPE_RS485) {
    delay(1);
    digitalWrite(UART0_RTS_DIR1, 0);
  }
  return ret;
}

int DeviceSerial::print(const Printable& x)
{
	int ret;  
	
  _sc5801.SetLED(LED2, (com_led ^= 1));
	if (sccom_mode == COM_TYPE_RS485)  digitalWrite(UART0_RTS_DIR1, 1);
  ret = Serial.print(x);
  if (sccom_mode == COM_TYPE_RS485) {
    delay(1);
    digitalWrite(UART0_RTS_DIR1, 0);
  }
 return ret;
}

int DeviceSerial::println(const String &val)
{
	int ret;  
	
  _sc5801.SetLED(LED2, (com_led ^= 1));
	if (sccom_mode == COM_TYPE_RS485)  digitalWrite(UART0_RTS_DIR1, 1);
  ret = Serial.println(val);
  if (sccom_mode == COM_TYPE_RS485) {
    delay(val.length());
    digitalWrite(UART0_RTS_DIR1, 0);
    _sc5801.SetLED(LED2, (com_led ^= 1));
  }
  return ret;
}   
int DeviceSerial::println(const char *val)
{
	int ret;  
	
  _sc5801.SetLED(LED2, (com_led ^= 1));
	if (sccom_mode == COM_TYPE_RS485)  digitalWrite(UART0_RTS_DIR1, 1);
  ret = Serial.println(val);
  if (sccom_mode == COM_TYPE_RS485) {
    delay(strlen(val));
    digitalWrite(UART0_RTS_DIR1, 0);
    _sc5801.SetLED(LED2, (com_led ^= 1));
  }
  return ret;
}   
int DeviceSerial::println(char val)
{
	int ret;  
	
  _sc5801.SetLED(LED2, (com_led ^= 1));
	if (sccom_mode == COM_TYPE_RS485)  digitalWrite(UART0_RTS_DIR1, 1);
  ret = Serial.println(val);
  if (sccom_mode == COM_TYPE_RS485) {
    delay(1);
    digitalWrite(UART0_RTS_DIR1, 0);
  }
  return ret;
}   
int DeviceSerial::println(unsigned char val, int base)
{
	int ret;  
	
  _sc5801.SetLED(LED2, (com_led ^= 1));
	if (sccom_mode == COM_TYPE_RS485)  digitalWrite(UART0_RTS_DIR1, 1);
  ret = Serial.println(val, base);
  if (sccom_mode == COM_TYPE_RS485) {
    delay(1);
    digitalWrite(UART0_RTS_DIR1, 0);
  }
  return ret;
}

int DeviceSerial::println(int val, int base)
{
	int ret;  
	
  _sc5801.SetLED(LED2, (com_led ^= 1));
	if (sccom_mode == COM_TYPE_RS485)  digitalWrite(UART0_RTS_DIR1, 1);
  ret = Serial.println(val, base);
  if (sccom_mode == COM_TYPE_RS485) {
    delay(1);
    digitalWrite(UART0_RTS_DIR1, 0);
  }
  return ret;
}

int DeviceSerial::println(unsigned int val, int base)
{
	int ret;  
	
  _sc5801.SetLED(LED2, (com_led ^= 1));
	if (sccom_mode == COM_TYPE_RS485)  digitalWrite(UART0_RTS_DIR1, 1);
  ret = Serial.println(val, base);
  if (sccom_mode == COM_TYPE_RS485) {
    delay(1);
    digitalWrite(UART0_RTS_DIR1, 0);
  }
  return ret;
}

int DeviceSerial::println(long val, int base)
{
	int ret;  
	
  _sc5801.SetLED(LED2, (com_led ^= 1));
	if (sccom_mode == COM_TYPE_RS485)  digitalWrite(UART0_RTS_DIR1, 1);
  ret = Serial.print(val, base);
  if (sccom_mode == COM_TYPE_RS485) {
    delay(1);
    digitalWrite(UART0_RTS_DIR1, 0);
  }
  return ret;
}

int DeviceSerial::println(unsigned long val, int base)
{
	int ret;  
	
  _sc5801.SetLED(LED2, (com_led ^= 1));
	if (sccom_mode == COM_TYPE_RS485)  digitalWrite(UART0_RTS_DIR1, 1);
  ret = Serial.println(val, base);
  if (sccom_mode == COM_TYPE_RS485) {
    delay(1);
    digitalWrite(UART0_RTS_DIR1, 0);
  }
  return ret;
}

int DeviceSerial::println(double val, int digits)
{
	int ret;  
	
  _sc5801.SetLED(LED2, (com_led ^= 1));
	if (sccom_mode == COM_TYPE_RS485)  digitalWrite(UART0_RTS_DIR1, 1);
  ret = Serial.println(val, digits);
  if (sccom_mode == COM_TYPE_RS485) {
    delay(1);
    digitalWrite(UART0_RTS_DIR1, 0);
  }
  return ret;
}

int DeviceSerial::println(const Printable& x)
{
	int ret;  
	
  _sc5801.SetLED(LED2, (com_led ^= 1));
	if (sccom_mode == COM_TYPE_RS485)  digitalWrite(UART0_RTS_DIR1, 1);
  ret = Serial.println(x);
  if (sccom_mode == COM_TYPE_RS485) {
    delay(1);
    digitalWrite(UART0_RTS_DIR1, 0);
  }
  return ret;
}

int DeviceSerial::println(void)
{
  int ret;
   _sc5801.SetLED(LED2, (com_led ^= 1));
 if (sccom_mode == COM_TYPE_RS485)  digitalWrite(UART0_RTS_DIR1, 1);
  ret = Serial.println();
  if (sccom_mode == COM_TYPE_RS485) {
    delay(1);
    digitalWrite(UART0_RTS_DIR1, 0);
  }
  return ret;
}

byte DeviceSerial::read(void)
{
  int ret;
  
  ret =  Serial.read();
  if (ret > 0)   _sc5801.SetLED(LED2, (com_led ^= 1));
  return ret;
}   
int DeviceSerial::readBytes(char *buffer, int length)
{
  int ret;
  
  ret =  Serial.readBytes(buffer, length);
  if (ret > 0)   _sc5801.SetLED(LED2, (com_led ^= 1));
  return ret;
}   
int DeviceSerial::readBytesUntil(char character, char *buffer, int length)
{
  int ret;
  
  ret =  Serial.readBytesUntil(character, buffer, length);
  if (ret > 0)   _sc5801.SetLED(LED2, (com_led ^= 1));
  return ret;
}   
boolean DeviceSerial::find(char *target)
{
  return  Serial.find(target);
}   
boolean DeviceSerial::find(char *target, int length)
{
  return  Serial.find(target, length);
}   
boolean DeviceSerial::findUntil(char *target, char *terminator)
{
  return  Serial.findUntil(target, terminator);
}   
boolean DeviceSerial::findUntil(char *target, int targetlen, char *terminator, int termlen)
{
  return  Serial.findUntil(target, targetlen, terminator, termlen);
}   
void DeviceSerial::flush(void)
{
  Serial.flush();
}   
float DeviceSerial::parseFloat(void)
{
  return  Serial.parseFloat();
}   
int DeviceSerial::parseInt(void)
{
  return  Serial.parseInt();
}   
char DeviceSerial::peek(void)
{
  return  Serial.peek();
}   
void DeviceSerial::setTimeout(unsigned long timeout)
{
  Serial.setTimeout(timeout);
}   
int DeviceSerial::write(const unsigned char *buf, int len)
{
  int ret;  
  
  _sc5801.SetLED(LED2, (com_led ^= 1));
  if (sccom_mode == COM_TYPE_RS485)  digitalWrite(UART0_RTS_DIR1, 1);
  ret = Serial.write(buf, len);
  if (sccom_mode == COM_TYPE_RS485) {
    delay(len);
    digitalWrite(UART0_RTS_DIR1, 0);
  }
  
  return ret;
}


//--- NBIOT ------------
void NBIOT::Reset(void)
{
  nbstate = NBIOT_CLOSE;
  digitalWrite(MCU_RST, 1);
  delay(3000);
  digitalWrite(MCU_RST, 0);
  delay(1000);
  NBIOT_init();
}    

int NBIOT::GetState(void)
{
  return nbstate;
}    

void NBIOT::GetApn(char *apn)
{
  strcpy(apn, nbapn);
}		

int NBIOT::SetApn(char *apn)
{
  char tmp[50];
  int len, ret=1;
  
  memset(nbapn, 0, sizeof(nbapn));
  strcpy(nbapn, apn);
  return 0;
} 

int NBIOT::GetSimState(char *msg)
{
  char tmp[50], *ptr;
  int len, ret=-1;
  
  sprintf(tmp, "AT+CPIN?\r\n");  
 // Serial.print(tmp);
  Serial1.print(tmp);
  *msg = '\0';
  
  Serial1.setTimeout(2000);
  memset(tmp, 0, sizeof(tmp));
  while((len = Serial1.readBytesUntil('\n', tmp, sizeof(tmp)-1)) > 0) {
    Serial.print("GetSimState Get: ");
    Serial.println(tmp);
    if ((strstr(tmp, "+CPIN:"))) {
        //--- Get Data --------
        tmp[len-1] = '\0';
        strcpy(msg, &tmp[6]);
        //--- Check is "Ready" ----
        if (strstr(tmp, "READY")) {     //SIM Ready
          ret = 0;
          break;
        } else {
          ret = 1;
        }
    } else continue;
  } 
  return ret;
} 

void NBIOT::GetPinCode(char *pinCode)
{
  strcpy(pinCode, nbpinCode);
}		

int NBIOT::SetPinCode(char *pinCode)
{
	char tmp[50];
	int len, ret=0;
	
	memset(nbpinCode, 0, sizeof(nbpinCode));
	strcpy(nbpinCode, pinCode);
	return ret;
}	

int NBIOT::GetPinEnable(void)
{
  return nbpinFlag;
}		

int NBIOT::SetPinEnable(int pinFlag)
{
	int len, ret=1;
	
	nbpinFlag = pinFlag;
	return 0;
}	
int NBIOT::GetBands(void)
{
  return nbband;
}    

int NBIOT::SetBands(int band)
{
  int len, ret=1;
  
  nbband = band;
  return 0;
} 
int NBIOT::Connect(void)
{
  nbdo_dail = 1;
  return 0;
} 

int NBIOT::DisConnect(void)
{
  nbdo_dail = 0;
  deactive = 1;
  return 0;
} 

int NBIOT::UDPConnect(char *dest_ip, int dest_port, int local_port)
{
  int len, ret=1;

  sscanf(dest_ip, "%d.%d.%d.%d", &nbdest_ip[0], &nbdest_ip[1], &nbdest_ip[2], &nbdest_ip[3]);
  nbdest_port = dest_port;
  nblocal_port = local_port;
  nblink_mode = NBIOT_LINK_UDP;

  return 1;
} 

int NBIOT::SendUDP(int socket, char *data, int len)
{
  char str[30];
  
  sprintf(str, "AT+CIPSEND=%d\r\n",len);
  Serial1.print(str);
  delay(500);
// ###
//  Serial1.print(data);
  Serial1.write((uint8_t*) data, len);
  Serial1.println("\r\n");
  
  return len;
} 

int NBIOT::RecvUDP(int socket, char *data, int len)
{
  int cnt=0;
  
  if (data_length > 0) {
    if (data_length > len) cnt = len;
    else cnt = data_length;
    memcpy(data, rcv_data, cnt);
    if (cnt != data_length) {
      memmove(rcv_data, &rcv_data[cnt], data_length-cnt);
    } else {
      memset(rcv_data, 0, sizeof(rcv_data));
    }
    data_length -= cnt;
  }
  
  return cnt;
} 

int NBIOT::TCPConnect(char *dest_ip, int dest_port, int local_port)
{
  int len, ret=1;

  sscanf(dest_ip, "%d.%d.%d.%d", &nbdest_ip[0], &nbdest_ip[1], &nbdest_ip[2], &nbdest_ip[3]);
  nbdest_port = dest_port;
  nblocal_port = local_port;
  nblink_mode = NBIOT_LINK_TCP;

  return 1;
} 

int NBIOT::SendTCP(int socket, char *data, int len)
{

  len = SendUDP(socket, data, len);  
  return len;
} 

int NBIOT::RecvTCP(int socket, char *data, int len)
{

  len = RecvUDP(socket, data, len);  
  return len;
} 

void NBIOT::GetIPAddr(char *ptr)
{
  strcpy(ptr, nbipaddr);
} 
int NBIOT::GetSignalRSRP(void)
{
  return nbrssi;
} 
int NBIOT::GetSignalRSSI(void)
{
  return nbrsrp;
} 

//================================================================  
int Report(const char *pcFormat, ...)
{
    int     iRet = 0;
    char    *pcBuff;
    char    *pcTemp;
    int     iSize = 256;
    va_list list;

    pcBuff = (char*)malloc(iSize);
    if(pcBuff == NULL)
    {
        return -1;
    }
    while(1)
    {
        va_start(list,pcFormat);
        iRet = vsnprintf(pcBuff, iSize, pcFormat, list);
        va_end(list);
        if((iRet > -1) && (iRet < iSize))
        {
            break;
        }
        else
        {
            iSize *= 2;
            if((pcTemp = (char *)realloc(pcBuff, iSize)) == NULL)
            {
                Serial.print("Could not reallocate memory\n\r");
                iRet = -1;
                break;
            }
            else
            {
                pcBuff = pcTemp;
            }
        }
    }
    if (sccom_mode == COM_TYPE_RS485)  digitalWrite(UART0_RTS_DIR1, 1);
    _sc5801.SetLED(LED2, (com_led ^= 1));
    Serial.print(pcBuff);
    if (sccom_mode == COM_TYPE_RS485) {
      delay(strlen(pcBuff));
      digitalWrite(UART0_RTS_DIR1, 0);
      _sc5801.SetLED(LED2, (com_led ^= 1));
    }
    free(pcBuff);

    return iRet;
}
	
