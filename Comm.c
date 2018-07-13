
#include <sys/time.h>

#include "include/CL1306.h"
#include "include/gpio.h"
#include "include/player.h"
#include "include/read.h"
#include "include/net.h"
#include "include/psam.h"
#include "include/display.h"
#include "include/keyboard.h"

#include "main.h"
#include "Config.h"


#define FRAME_OK	0
#define FRAME_ERR	1
#define FRAME_LENGTH_ERROR	2
#define FRAME_COMMAND_ERROR		3
#define FRAME_CHECKSUM_ERROR	4
#define FRAME_TERMINAL_ERROR	5

#define FRAME_HEADER	0xFD
#define FRAME_INDEX		0x01
#define FRAME_TERMINAL	0x0A



#pragma pack(1)
typedef struct _frame_data_t
{
	uint8_t header; // FD
	uint8_t index; // 01
	uint16_t length;
	uint8_t command;
	uint8_t *pdata;
	uint8_t crc;
	uint8_t end; // 0A
}FRAME_DATA,*PFRAME_DATA;

#pragma pack()

char RxBuffer[UART1_RXSIZE+1];
uint16_t RxPos; 
char TxBuffer[UART1_TXSIZE+7];
uint16_t TxSize;

uint8_t m_cmdret = 0;

uint8_t OnGetFrame(uint8_t *bufin, uint16_t len, PFRAME_DATA pframe, uint16_t *poffset)
{
	uint16_t i;
	uint16_t pos;
	uint8_t crc = 0;
	uint16_t length;

	if(len == 0)
		return FRAME_LENGTH_ERROR;
	// Get frame start position
	for(i=0;i<(len-1);i++)
		if((bufin[i] == FRAME_HEADER)&&(bufin[i+1] == FRAME_INDEX))
			break;
	if(i==(len-1))
		return FRAME_ERR;
	pos = i;


	// Check frame length
	memcpy(&length, bufin+pos+2, 2);
	pframe->length = length;
	if((len - pos)<pframe->length)
		return FRAME_LENGTH_ERROR;
	// Check frame checksum
	pframe->crc = bufin[pos+pframe->length+2];
	for(;i<(pframe->length+2);i++)
		crc ^= bufin[i];
	if(crc != pframe->crc)
		return FRAME_CHECKSUM_ERROR;

	// Check frame terminal
	if(bufin[pos+pframe->length+3] != FRAME_TERMINAL)
		return FRAME_TERMINAL_ERROR;

	// Get frame
	memcpy(pframe, bufin+pos, 5);
	pframe->pdata = &bufin[pos+5];
	pframe->crc = crc;
	pframe->end = FRAME_TERMINAL;

	// Get frame offset
	*poffset = pos;
	return FRAME_OK;
}
void OnSetFrame(uint8_t cmd, uint8_t *pdata, uint16_t len)
{// len<UART1_TXSIZE
	uint16_t i, pos=0;
	uint8_t crc=0;
	uint8_t bufout[UART1_TXSIZE+7];
	
	TxBuffer[pos++] = FRAME_HEADER; 
	TxBuffer[pos++] = FRAME_INDEX;
	TxBuffer[pos++] = (len + 3)&0xFF;
	TxBuffer[pos++] = ((len + 3)>>8)&0xFF;
	TxBuffer[pos++] = cmd;
	for(i=0;i<len;i++)
		TxBuffer[pos++] = pdata[i];
	for(i=0;i<pos;i++)
		crc ^= TxBuffer[i];
	TxBuffer[pos++] = crc;
	TxBuffer[pos++] = FRAME_TERMINAL;

	TxSize = pos;
//		gparam.sendtick = 0;
	OnTcpSend(TxBuffer, pos);
}

void OnRecvData(void)
{
	uint8_t ret;
	uint16_t framesize = 0;
	uint16_t offset = 0;
	FRAME_DATA fdi;

	sprintf(gszbuf, "recv %d bytes", RxPos);
	DebugOut(gszbuf);

while(RxPos)
{
	ret = OnGetFrame((uint8_t*)RxBuffer, RxPos, &fdi, &offset);


	//sprintf(gszbuf, "OnGetFrame=%d", ret);
	//DebugOut(gszbuf);
	if(ret == FRAME_OK)
	{
		switch(fdi.command)
		{
		case 0x01:
			{
				const char delimiters[] = "+";
				char *token;
				uint32_t timeout = 0;
				uint32_t servertime = 0;
				char szstr[256];

//					gparam.sendtick = 0xFFFFFFFF;
				//DebugOut("new timeout value");
				// new timeout value
				memcpy(&timeout, fdi.pdata+1, 4);
				if((timeout!=0)&&(timeout != gparam.timeout))
				{
					char *pstr;
					INI_CONFIG *pconfig;
					
					pconfig = ini_config_create_from_file(CONFIG_FILE, 0);
					if(pconfig != NULL)
					{
						pstr = ini_config_set_int(pconfig, NULL, "interval", strlen("interval"), timeout);
						
						ini_config_save(pconfig, CONFIG_FILE);
						ini_config_destroy(pconfig);
					}
//						FILE *pfile;
//						
//						pfile = fopen("/mnt/nand1-2/app/timeout.txt", "wb+");
//						if(pfile != NULL)
//						{
//							fwrite(&timeout, 1, 4, pfile);
//							fclose(pfile);
//						}
					gparam.timeout = timeout;
				}

				// server time
				if(0)
				{
					struct timeval tv;
					struct timezone tz;
					time_t t;
					time (&t);
					
					memcpy(&servertime, fdi.pdata+5, 4);
					if(__abs(servertime+28800, t)>30)
					{
						gettimeofday (&tv , &tz);
						tv.tv_sec = servertime+28800;
						tv.tv_usec = 0;
						settimeofday(&tv, &tz);
					}
				}
				
				//DebugOut("text info");
				// text info
				strcpy(szstr, fdi.pdata+9);
				if(strlen(szstr)>0)
				{
					token = strtok(szstr, delimiters); // tagid
					if(token != NULL)
					{
						char *ptr;
						gparam.user.id = strtoul(token, ptr, 10);
						RecordDel(gparam.user.id);
					
						//sprintf(gszbuf, "%s", token);
						//DebugOut(gszbuf);
	
						token = strtok(NULL, delimiters);// name
						if(token != NULL)
						{
							strcpy(gparam.user.name, token);
						
							//sprintf(gszbuf, "%s", token);
							//DebugOut(gszbuf);
						}
						
						gparam.draw |= DRAW_USRNAME;
						gparam.event |= EVENT_DRAW;
					}
					if(token != NULL)
					{
						token = strtok(NULL, delimiters);// phone
						if(token != NULL)
						{
							strcpy(gparam.user.phone, token);
						
							//sprintf(gszbuf, "%s", token);
							//DebugOut(gszbuf);
						}
					}
					gparam.timeout_display = 1;
				}
			}
			break;
		}
		framesize = fdi.length+4;
		memmove(RxBuffer, RxBuffer+offset+framesize, UART1_RXSIZE - offset - framesize);
		RxPos -= offset+framesize;
		memset(RxBuffer+RxPos, 0, UART1_RXSIZE - RxPos);
	}
	else
	{
		if(ret != FRAME_LENGTH_ERROR)
			RxPos = 0;
	}
}
}

