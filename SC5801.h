#include <Energia.h>
#include <xdc/runtime/Error.h>
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/hal/Timer.h>

#ifndef SC5801_h
#define SC5801_h
//#define BOARD_V2 			1

#define LED1 			1
#define LED2 			2
#define MCU_PWR 		3
#define LED3 			4
#define DI1  			4 
#define TERM_RS485  	15
#define MODE0_RS  		18
#define UART0_RTS_DIR1  50
#define WIFI_RESET 		53
#define MCU_RST 		58
#define RTC_INT_MCU 	59
#define DI2  			60
#define EXP_INT_MCU 	62
#define MCU_BAT_RST 	63
#define TTL_CTL  		64

#define I2C_GPIO_ADDR 0x27
#define RS_OE 			0x80
#define ON  			1
#define OFF  			0
//---- RTC -----------------
// OffSet Setting
#define RTC_ADDR 0x51
#define CTRL1    0x00
#define CTRL2    0x01
#define SEC      0x02
#define MIN      0x03
#define HOUR     0x04
#define DATE     0x05
#define WEEK     0x06
#define MONTH    0x07
#define YEAR     0x08

//---- File System ---------
#define SIZE_FILE_SPACE 1024        //1024 bytes
#define NAME_USER_FILE    "config.txt"

//---- COM State -----------
#define COM_TYPE_RS232  1
#define COM_TYPE_RS485  2

//---- NBIOT State -----------
#define NBIOT_SIM_ERR  -2
#define NBIOT_PIN_ERR  -1
#define	NBIOT_CLOSE			0
#define	NBIOT_READY			1
#define	NBIOT_DAILING		2
#define NBIOT_GET_IP    3
#define NBIOT_CONNECT   4
#define NBIOT_DISCONN   5

//---- NBIOT Link Mode ----------
#define NBIOT_LINK_UDP    0
#define NBIOT_LINK_TCP    1
#define NBIOT_LINK_LISTEN 2

//---- NBIOT Band ----------
#define NBIOT_BAND_CAT  1
#define NBIOT_BAND_NB   2
#define NBIOT_BAND_AUTO 3

#define  SIZE_RCV_DATA    256

typedef struct {
	int socket;
	char *dest_ip;
	int dest_port;
	char *data;
} NBIOT_RCV;

extern int Report(const char *pcFormat, ...);

class SC5801
{
	public:
		void init(void);
    void SetRTC(byte year, byte month, byte day, byte week, byte hour, byte minute, byte second);
    String GetRTC(void);
		void SetLED(byte ledIdx, byte ledState);
    int WriteFile(int offset, char *buffer, int len);
    int ReadFile(int offset, char *buffer, int len);
};

class NBIOT
{
	public:
    void Reset(void);
    int GetState(void);
		void GetApn(char *apn);
		int SetApn(char *apn);
    int GetSimState(char *Msg);
    void GetPinCode(char *pinCode);
		int SetPinCode(char *pinCode);
		int SetPinEnable(int pinFlag);
		int GetPinEnable(void);
		int Connect(void);
		int DisConnect(void);
		int Connected(void);
		int GetSignal(void);
		void GetIPAddr(char *);
    int UDPConnect(char *dest_ip, int dest_port, int local_port);
    int SendUDP(int socket, char *data, int len);
    int RecvUDP(int socket, char *data, int len);
    int TCPConnect(char *dest_ip, int dest_port, int local_port);
    int TCPListen(int local_port);
    int SendTCP(int socket, char *data, int len);
    int RecvTCP(int socket, char *data, int len);
		int SetBands(int band);
		int GetBands(void);
    int GetSignalRSSI(void);
    int GetSignalRSRP(void);

    int nbstate;
    char nbapn[20];
    char nbpinCode[20];
    int nbpinFlag;
    char nbipaddr[20];
    int nblocal_port;
    int nbdest_port;
    int nbdest_ip[4];
    int nblink_mode;
    int nbband;
    int nbdo_dail;
    int nbrssi;
    int nbrsrp;
};

class DeviceSerial
{
	public:
    int GetSerialMode(void);
    void SetSerialMode(int mode);
    void begin(unsigned long speed);
    void begin(unsigned long speed, int config);
		int available(void);
		void end(void);
    int print(const String &);
    int print(const char[]);
    int print(char);
    int print(unsigned char, int );
    int print(int, int );
    int print(unsigned int, int );
    int print(long, int );
    int print(unsigned long, int );
    int print(double, int );
    int print(const Printable&);

    int println(const String &s);
    int println(const char[]);
    int println(char);
    int println(unsigned char, int = DEC);
    int println(int, int = DEC);
    int println(unsigned int, int = DEC);
    int println(long, int = DEC);
    int println(unsigned long, int = DEC);
    int println(double, int = 2);
    int println(const Printable&);
    int println(void);
    
    byte read(void); 
    int readBytes(char *buffer, int length); 
    int readBytesUntil(char character, char *buffer, int length); 
    boolean find(char *target);
    boolean find(char *target, int length);
    boolean findUntil(char *target, char *terminator);
    boolean findUntil(char *target, int targetlen, char *terminator, int termlen);
		void flush(void);
		float parseFloat(void);
		int parseInt(void);
    char peek(void); 
    void setTimeout(unsigned long timeout);
//    int write(const unsigned char *str);
//    int write(unsigned char val);
    int write(const unsigned char *buf, int len);

//    int sccom_mode;

};


class Timer
{
	private:
	    Timer_Handle TimerHandle;
	    
	public:
	    void begin(void (*timerFunction)(void), uint32_t timerPeriod_unit, uint32_t unit = 1000);
	    void start();
	    void stop();
};

#endif



