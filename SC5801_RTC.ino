/* 
 *  Use I2C interface to set/get RTC data
 *  
 *  Function :
 *  byte DECtoHEX(byte val)    : Convert DEC to HEX
 *  byte HEXtoDEC(byte val)    : Convert HEX to DEC
 *  void RTC_Write(byte offset, byte val) : Setting RTC time
 *  byte RTC_Read(byte offset) : Read RTC time
 *  void ShowTime()            : Print RTC time
 *  void RTC_Init()            : RTC initial
 *  void SetTime(byte year, byte month, byte date, byte week, byte hour, byte min, byte sec) : Set RTC time
 */ 

#include "SC5801.h"
#include <Wire.h>

byte DECtoHEX(byte val){
  return ((val/10)*16) + (val%10);
}

byte HEXtoDEC(byte val){
  return ((val/16)*10) + (val%16);
}

void RTC_Init(){
  // CTRL1 & CTRL2 must be set 0
  Wire.beginTransmission(RTC_ADDR);
  Wire.write(CTRL1);
  Wire.write(0x0);
  Wire.write(0x0);
  Wire.endTransmission();
}

void RTC_Write(byte offset, byte val)
{
  Wire.beginTransmission(RTC_ADDR);
  Wire.write(offset);
  Wire.write(DECtoHEX(val));
  Wire.endTransmission();
}

byte RTC_Read(byte offset)
{
  Wire.beginTransmission(RTC_ADDR);
  Wire.write(offset);
  Wire.endTransmission();
  Wire.requestFrom(RTC_ADDR,1);

  switch (offset){
    case SEC :
      return HEXtoDEC(Wire.read()&0x7F);
      break;
    
    case MIN :
      return HEXtoDEC(Wire.read()&0x7F);
      break;
    
    case HOUR :
      return HEXtoDEC(Wire.read()&0x3F);
      break;
    
    case DATE :
      return HEXtoDEC(Wire.read()&0x3F);
      break;
    
    case WEEK :
      return HEXtoDEC(Wire.read()&0x07);
      break;
    
    case MONTH :
      return HEXtoDEC(Wire.read()&0x1F);
      break;
    
    case YEAR :
      return HEXtoDEC(Wire.read());
      break;
    
  }
}

