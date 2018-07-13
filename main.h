


#define CONFIG_FILE "/mnt/nand1-2/app/config.txt"

#define Color_lightblue     RGB565_(0,127,255)

#define __abs(a, b)	((a>b)?(a-b):(b-a))

#define DRAW_CAPTION	(1<<0)
#define DRAW_GPSINFO	(1<<1)
#define DRAW_USRNAME	(1<<2)
#define DRAW_SYSTIME	(1<<3)
#define DRAW_ALLITEM	(0x0F)
typedef enum _sys_event_e
{
	EVENT_DRAW = (1<<0),
	EVENT_KEYPRESS = (1<<1),
	EVENT_READTAG = (1<<2),
	EVENT_BEEP = (1<<3),
	EVENT_GPS = (1<<4),
	EVENT_TCPLINK = (1<<5),
	EVENT_MAX
}SYS_EVENT;

typedef struct _upload_data_t
{
	uint32_t uid;
	uint32_t tagid;
	uint32_t delay;
	uint32_t time;
	uint32_t latitude;
	uint32_t longitude;
}UPLOAD_DATA;


typedef struct _user_info_t
{
	uint32_t id;
	char name[16];
	char phone[16];
}USER_INFO;

typedef struct _gps_info_t
{
	char status; // 0 not active, 1 active
	uint32_t latitude; // wei du, N/S
	uint32_t longitude; // jing du, E/W
	uint32_t gpstime;
}GPS_INFO;

typedef struct _sys_param_t
{
	int key;
	uint32_t event;
	uint32_t draw;

	// config
	uint32_t uid;
	uint32_t timeout;

	uint32_t tagtick;
	uint32_t gpstick;
	uint32_t sendtick;
	uint32_t timeout_display; // display timeout

	USER_INFO user;
	GPS_INFO gps;

	char online;
	char serverip[16];
	uint16_t serverport;

}SYS_PARAM;


#define UART1_TXSIZE 128
#define UART1_RXSIZE 1024
extern char RxBuffer[UART1_RXSIZE+1];
extern uint16_t RxPos;
extern SYS_PARAM gparam;
extern char gszbuf[128];
extern char TxBuffer[UART1_TXSIZE+7];
extern uint16_t TxSize;

void DebugOut(char *str);
uint32_t GetTick();
void OnTcpSend(uint8_t *pbuf, uint16_t len);


