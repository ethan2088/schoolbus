#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>
#include <signal.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include <errno.h>
#include <netdb.h>
#include <sys/wait.h>
#include <dirent.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <popt.h>
#include <ctype.h>
#include <sys/socket.h>


#include "include/CL1306.h"
#include "include/gpio.h"
#include "include/player.h"
#include "include/read.h"
#include "include/net.h"
#include "include/psam.h"
#include "include/display.h"
#include "include/keyboard.h"
#include "gps.h"


#define FALSE 0
#define TRUE  1


#ifdef DEBUG_PRINTF
#define DebugPrintf(format, ...) fprintf(stdout,"[%s][%s][%d]"format,__FILE__,__FUNCTION__,__LINE__,##__VA_ARGS__)
#else
#define DebugPrintf(format, ...)
#endif

int gps_fd;
int gpssockfd=-1;
unsigned char GPSSIG;

volatile float GPS_longitude = 0;
volatile float GPS_latitude = 0;

volatile float DUSUGAO_GPS_longitude = 0;
volatile float DUSUGAO_GPS_latitude = 0;
int speed_arrgps[] =
{
	B115200, B38400, B19200, B9600, B4800, B2400, B1200, B300,
	B38400, B19200, B9600,  B4800, B2400, B1200,B600, B300,
};

int name_arrgps[] =
{
	115200, 38400, 19200, 9600, 4800, 2400, 1200, 300,
	38400, 19200, 9600, 4800, 2400,  1200,600, 300,
};
char NGPS[15];
char EGPS[15];
 /*
 *************************************************************************************************************
 - function name : void set_speedgps (int fd, int speed)
 - function declaration : set connect GPS serial port baud rate
 - input parameter : fd :serial device handle£¬speed£ºbaud rate£ºnoramlly set as 9600
 - output parameter : null
 *************************************************************************************************************
 */
void set_speedgps (int fd, int speed)
{
	int i;
	int status;
	struct termios Opt;
	tcgetattr (fd, &Opt);
	for (i = 0; i < sizeof (speed_arrgps) / sizeof (int); i++)
	{
		if(speed == name_arrgps[i])
		{
			tcflush (fd, TCIOFLUSH);
			if(cfsetispeed (&Opt, speed_arrgps[i])==0) printf(" gps baurate sucess\n");
			else printf("set speed failed");
			cfsetospeed (&Opt, speed_arrgps[i]);
			status = tcsetattr (fd, TCSANOW, &Opt);
			if (status != 0)
			{
				perror ("tcsetattr fd1");
				return;
			}
			tcflush (fd, TCIOFLUSH);
		}
	}
}
 
 
int AsciiToHex( unsigned char *pInBuf, int nInLen,  unsigned char *pOutBuf) 
{ 
	int i,j,tmp_len; 
	unsigned char tmpData; 
	unsigned char *pIn  = pInBuf; 
	unsigned char *pOut = pOutBuf; 

	for(i = 0; i < nInLen; i++) 
	{ 
		if ((pIn[i] >= '0') && (pIn[i] <= '9')) 
		{ 
			tmpData = pIn[i] - '0'; 
		} 
		else  if ((pIn[i] >= 'A') && (pIn[i] <= 'F'))	//A....F 
		{ 
			tmpData = pIn[i] - 0x37; 
		} 
		else if((pIn[i] >= 'a') && (pIn[i] <= 'f'))  //a....f 
		{ 
			tmpData = pIn[i] - 0x57; 
		} 
		else 
		{ 
			return -1; 
		} 

		pIn[i] =  tmpData; 
	} 

	for(tmp_len = 0,j = 0; j < i; j+=2) 
	{ 
		pOut[tmp_len++] = (pIn[j]<<4) | pIn[j+1]; 
	} 

 return tmp_len;  
}

int HexToAscii(unsigned char *pInBuf, int nInLen, char *pOutBuf) 
{ 
 const char  ascTable[17] = {"0123456789ABCDEF"}; 
 char *p = pOutBuf; 

 int i, pos = 0;

 for(i = 0; i < nInLen; i++) 
 { 
	 p[pos++] = ascTable[pInBuf[i] >> 4]; 
	 p[pos++] = ascTable[pInBuf[i] & 0x0f];  
 } 

 p[pos] = '\0'; 

 return pos;		
}
int hex2ascii(unsigned char *INdata, unsigned int len, char *strout)
{
	const char ascTable[17] = {"0123456789ABCDEF"};
	char *tmp_p = strout;
	unsigned int i, pos;

	pos = 0;
	for(i = 0; i < len; i++)
	{
		tmp_p[pos++] = ascTable[INdata[i] >> 4];
		tmp_p[pos++] = ascTable[INdata[i] & 0x0f];
	}
	tmp_p[pos] = '\0';
	return pos;
}
 /*
 *************************************************************************************************************
 - function name : int set_Paritygps (int fd, int databits, int stopbits, int parity)
 - function declaration : set connect GPS serial port attribute
 - input parameter : fd :serial device handle£¬databits£ºdata bits£¬normally set as 8;stopbits£¬stop bit£¬normally set as 'N'£»parity£ºparity bit£¬normally set as 1
 - output parameter : null
 *************************************************************************************************************
 */
int set_Paritygps (int fd, int databits, int stopbits, int parity)
{
	struct termios options;
	if (tcgetattr (fd, &options) != 0)
	{
		perror ("SetupSerial 1");
		return (FALSE);
	}
	options.c_cflag &= ~CSIZE;
	switch (databits)
	{
	case 7:
		options.c_cflag |= CS7;
		break;
	case 8:
		options.c_cflag |= CS8;
		break;
	default:
		fprintf (stderr, "Unsupported data size\n");
		return (FALSE);
	}
	switch (parity)
	{
	case 'n':
	case 'N':
		options.c_cflag &= ~PARENB;		/* Clear parity enable */
		options.c_iflag &= ~INPCK;		/* Enable parity checking */
		break;
	case 'o':
	case 'O':
		options.c_cflag |= (PARODD | PARENB);
		options.c_iflag |= INPCK;		/* Disnable parity checking */
		break;
	case 'e':
	case 'E':
		options.c_cflag |= PARENB;		/* Enable parity */
		options.c_cflag &= ~PARODD;
		options.c_iflag |= INPCK;		/* Disnable parity checking */
		break;
	case 'S':
	case 's':					/*as no parity */
		options.c_cflag &= ~PARENB;
		options.c_cflag &= ~CSTOPB;
		break;
	default:
		fprintf (stderr, "Unsupported parity\n");
		return (FALSE);
	}
	switch (stopbits)
	{
	case 1:
		options.c_cflag &= ~CSTOPB;
		break;

	case 2:
		options.c_cflag |= CSTOPB;
		break;

	default:
		fprintf (stderr, "Unsupported stop bits\n");
		return (FALSE);
	}
	if (parity != 'n')
		options.c_iflag |= INPCK;
	tcflush (fd, TCIFLUSH);
	options.c_cc[VTIME] = 50;
	options.c_cc[VMIN] = 0;				/* Update the options and do it NOW */

	options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	options.c_iflag &= ~( ICRNL | IXON);
	options.c_oflag &= ~( ICRNL | IXON);
	options.c_oflag &= ~OPOST;
	if (tcsetattr (fd, TCSANOW, &options) != 0)
	{
		perror ("SetupSerial 3");
		return (FALSE);
	}
	return (TRUE);
}

 /*
 *************************************************************************************************************
 - function name : int OpenDevgps (char *Dev)
 - function declaration : tur on GPS serial port
 - input parameter : Dev :device pointer
 - output parameter : null
 *************************************************************************************************************
 */
int OpenDevgps (char *Dev)
{
	int fd = open (Dev, O_RDWR );
	if (fd == -1)
	{
		printf ("can not open uart!");
		return -1;
	}
	else
	{
		printf ("open ok!\n");
		return fd;
	}
}
 /*
 *************************************************************************************************************
 - function name : void CloseDevgps (int fd)
 - function declaration : close GPS serial port
 - input parameter : fd :serial device handle
 - output parameter : null
 *************************************************************************************************************
 */
void CloseDevgps (int fd)
{
	close (fd);
}

/*
 *************************************************************************************************************
 - function name : int read_datas_ttygps(int fd,char *rcv_buf,int *len)
 - function declaration : read serial port data
 - input parameter : fd :serial device handle£»rcv_buf:receive data buffer area pointer £»len£ºreceive data length pointer
 - output parameter : null
 *************************************************************************************************************
 */
int read_datas_ttygps(int fd,char *rcv_buf,int *len)
{
	int retval;
	fd_set rfds;
	struct timeval tv;
	int ret=0,pos=0;
	tv.tv_sec = 0;//set the rcv wait time
	tv.tv_usec = 100000;//200000us = 0.3s
	pos = 0;
	if ((fd < 0) ||( NULL == rcv_buf))
    {
        printf("Read Buffer is Nc\n");
        return -1;
    }
	
	while(1)
	{
		FD_ZERO(&rfds);
		FD_SET(fd,&rfds);
		retval = select(fd+1,&rfds,NULL,NULL,&tv);
		if(retval ==-1)
		{
			perror("select()");
			return -1;
		}
		else if(retval)
		{
			ret= read(fd,rcv_buf+pos,100);
			if(ret>0)
			{
				pos += ret;
				*len = pos;
				
				if(pos >= (strlen(rcv_buf)-16))
						return 0;
			}
			else
				return -1;
		}
		else      //timeout
		{
				return 1;
		}
	}
	return 0;
}
/*
 *************************************************************************************************************
 - function name : int initializegps (const char *com, int speed)
 - function declaration : initialize serial port
 - input parameter : com:serial port name pointer£»speed:baud rate£¬normally 9600
 - output parameter : null
 *************************************************************************************************************
 */
int initializegps (const char *com, int speed)
{
	char serial[16] = "/dev/";
	strcat(serial, com);
	int fd = OpenDevgps (serial);
	if (fd > 0)
	{
		set_speedgps (fd, speed);
		if (set_Paritygps (fd, 8,1,'n') == FALSE)
		{
			printf ("set_parity error\n");
		}
		return fd;
	}
	return -1;
}
/*
 *************************************************************************************************************
 - function name : void uninitializegps (int fd)
 - function declaration : uninstall initialized serial port
 - input parameter : fd:serial device pointer
 - output parameter : null
 *************************************************************************************************************
 */
void uninitializegps (int fd)
{
	CloseDevgps(fd);
}

#if 1
/*
 *************************************************************************************************************
 - function name : int gps_date_GGA(char *string,char *okdate)
 - function declaration : query GGA signal string
 - input parameter : *string: received string£»*okdate: queried string
 - output parameter : 0 or -1
 *************************************************************************************************************
 */
int gps_date_GGA(char *string,char *okdate)
{
	char *pointer = NULL;
	char *end = NULL;
	int  ch;
	char tmp[1024];
	bzero(tmp,sizeof(tmp));
	pointer = strstr(string, "$GPGGA");
	if (pointer != NULL)
	{
		strncpy(tmp, pointer, 100);
		strncpy(okdate, tmp, 100);			
		return 0;
		}
	return -1;
}

#endif

/*
 *************************************************************************************************************
 - function name : int gps_date_GGA(char *string,char *okdate)
 - function declaration : query RMC signal string
 - input parameter : *string:received string£»*okdate:queried string
 - output parameter : 0 or -1
 *************************************************************************************************************
 */
int gps_date_RMC(char *string,char *okdate)
{
	char *pointer = NULL;
	char *end = NULL;
	int  ch;
	char tmp[1024];
	bzero(tmp,sizeof(tmp));
	pointer = strstr(string, "RMC");
	if (pointer != NULL)
	{
		strncpy(tmp, pointer, 100);
		strncpy(okdate, tmp, 100);
		return 0;
	}
	return -1;
}

 /*
 *************************************************************************************************************
 - function name : int search_char(char *str,char ch)
 - function declaration : query a certain character in string
 - input parameter : *str:string pointer£»ch£ºsearched character
 - output parameter : null
 *************************************************************************************************************
 */
int search_char(char *str,char ch)
{
	int n;
	for(n=0; str[n]; n++) {
		if(str[n] == ch)
			return n;
	}
	return -1;
}


/*fill des string 6 bytes with str. if str not enough 6 bytes ,use '0' filled*/
int cent_sixbit(char *des, char *str)
{
	int n,str_len,point_index;

	point_index = search_char(str,'.');
	point_index++;

	for(n=0; n<6; n++) {
		if(str[point_index]) {			//use str char
			des[n] = str[point_index];
			point_index++;
		}
		else {					//use '0' char
			des[n] = '0';
		}

	}
	
	//des[6] = 0;	 //may be beyone
}


int conver_degree(char *des,char *str)
{
	float temp_float,float_d1,float_d2;
	int   temp_int,des_index=0,str_index=0;
	char temp_char[5],cent_char[15];


	temp_int = search_char(str,'.');
	if(temp_int==4) {
		des[des_index++] = str[str_index++];
		des[des_index++] = str[str_index++];
		des[des_index++] = '.';
	}
	else if(temp_int==5) {
		des[des_index++] = str[str_index++];
		des[des_index++] = str[str_index++];
		des[des_index++] = str[str_index++];
		des[des_index++] = '.';
	}
	else {
		printf("wrong format! strstr return %d",temp_int);
		return -1;
	}

	// deal cent
	memcpy(temp_char,&str[str_index],2);
	str_index +=2;
	temp_char[2]=0;
	temp_int = atoi(temp_char);
	temp_float = temp_int/60.0;
	float_d1 = temp_float;

	//skip '.' in str
	str_index++;

	//deal float cent
	memcpy(temp_char,&str[str_index],4);
	temp_char[4]=0;
	temp_int = atoi(temp_char);
	temp_float = temp_int/10000.0;
	temp_float = temp_float/60.0;
	float_d2 = temp_float;

	temp_float = float_d1+float_d2;

	sprintf(cent_char,"%.6f",temp_float);
	cent_sixbit(&des[des_index],cent_char);

}




static u8 NMEA_CMD=NMEA_NULL;         //NMEA sentence
static u8 NMEA_CMD_Buff[]="$GPxxx,"; //NMEA statement type buffer
static u8 NMEA_CMD_Index=0;         //read CMD character number
        //CMD type resolution complete
static u8 NMEA_DAT_Block=0;         //NMEA data field number start from 0
static u8 NMEA_DAT_BlockIndex=0;     //NMEA data each field character index start from 0
            //data receive complete. last GPRMC statement send complete mark turn to 1

static u8 ucTempA=0;                 //storage analysis two digits used 10 temporary variable 
static u8 SateInfoIndex=0;            //
//static u8 ucTemp[5];

stru_GPSRMC  GPS_RMC_Data;
stru_GPSGGA  GPS_GGA_Data;
stru_GPSGSA  GPS_GSA_Data;
stru_GPSGSV  GPS_GSV_Data;
strubf_GPSRMC GPSBF_RMC_DATA;
strubf_GPSGGA GPSBF_GGA_DATA;
unsigned char  buffer0[200];

 /*
 *************************************************************************************************************
 - function name : void ShowGPSTime(void) 
 - function declaration :  gps time utc turn to Beijing time
 - input parameter : null
 - output parameter : null
 *************************************************************************************************************
 */
void ShowGPSTime(void)    
{
    GPS_RMC_Data.UTCDateTime[3]+=8;
    if (GPS_RMC_Data.UTCDateTime[3]>23)
    {
        GPS_RMC_Data.UTCDateTime[3]-=24; //Hour
        GPS_RMC_Data.UTCDateTime[2]++; //Day
        if (((GPS_RMC_Data.UTCDateTime[1]==1)||\
             (GPS_RMC_Data.UTCDateTime[1]==3)||\
             (GPS_RMC_Data.UTCDateTime[1]==5)||\
             (GPS_RMC_Data.UTCDateTime[1]==7)||\
             (GPS_RMC_Data.UTCDateTime[1]==8)||\
             (GPS_RMC_Data.UTCDateTime[1]==10)||\
             (GPS_RMC_Data.UTCDateTime[1]==12))&&\
             (GPS_RMC_Data.UTCDateTime[2]>31))
        {
            GPS_RMC_Data.UTCDateTime[2]=1;//Day
            GPS_RMC_Data.UTCDateTime[1]++;//Month
        }
        if (((GPS_RMC_Data.UTCDateTime[1]==4)||\
             (GPS_RMC_Data.UTCDateTime[1]==6)||\
             (GPS_RMC_Data.UTCDateTime[1]==9)||\
             (GPS_RMC_Data.UTCDateTime[1]==11))&&\
             (GPS_RMC_Data.UTCDateTime[2]>30))
        {
            GPS_RMC_Data.UTCDateTime[2]=1;
            GPS_RMC_Data.UTCDateTime[1]++;
        }
        if ((GPS_RMC_Data.UTCDateTime[1]==2)&&(GPS_RMC_Data.UTCDateTime[2]>28))
        {
            GPS_RMC_Data.UTCDateTime[2]=1;
            GPS_RMC_Data.UTCDateTime[1]++;
        }

        if(GPS_RMC_Data.UTCDateTime[1]>12)
        {
            GPS_RMC_Data.UTCDateTime[1]=1;
            GPS_RMC_Data.UTCDateTime[0]++;
        }
    }
return;
}
 /*
 *************************************************************************************************************
 - function name : void ParserGPGGA(void)   
 - function declaration :  GPS gga positioning information
 - input parameter : null
 - output parameter : null
 *************************************************************************************************************
 */
 void ParserGPGGA(void)       
{
	int i,iLen,time,status,used,ttyr;
	char rx_string[1024] = {0};
	char lcdtest[200];
	char *comma[20];
	float fn,fe;
	
	bzero(rx_string,1024);
	signal(SIGPIPE,SIG_IGN);  //close SIGPIPE signal£¬anti-lockup
	read_datas_ttygps(gps_fd,rx_string,&iLen);
	if(iLen>0)
	{
		bzero(lcdtest,sizeof(lcdtest));
	    if(gps_date_GGA(rx_string,lcdtest) == 0)
		{
			printf("%s\n",lcdtest);
			comma[0] = strstr(lcdtest, ",");  //UTC time£¬hhmmss(hour minute second)format
			if(NULL!=comma[0])
			{
				for(i=1;i<6;i++)
				{
					comma[i] = strstr(comma[i-1]+1, ",");  
					if(NULL==comma[i])
					{
						printf("GPGGA info incomplete\n");
						return;
					}
				}
			}
			else
			{
				printf("GPGGA info incomplete\n");
				return;
			}
#if 0			
			comma[0] = strstr(lcdtest, ",");        //UTC time£¬hhmmss(hour minute second)format
			comma[1] = strstr(comma[0]+1, ",");		//latitude£¬format ddmm.mmmm(firt zero will be also be sent)£»
			comma[2] = strstr(comma[1]+1, ",");     //longitude dddmm.mmmm(degree minute)format(firt zero will be also be sent)
			comma[3] = strstr(comma[2]+1, ",");     //longitude dddmm.mmmm(degree minute)format(firt zero will be also be sent)
			comma[4] = strstr(comma[3]+1, ",");     //Longitude hemisphereE(Eastern longitude) or W(Western longitude)
			comma[5] = strstr(comma[4]+1, ",");     // positioning quality indication£¬0= invalid positioning£¬1=valid positioning£»
			comma[6] = strstr(comma[5]+1, ",");     //used satellite No.£¬from 00 to 12(first zero will also be sent)
			comma[7] = strstr(comma[6]+1, ",");     //level accuracy£¬0.5 to 99.9
			comma[8] = strstr(comma[7]+1, ",");     //antenna height away from sea level£¬-9999.9 to 9999.9M		
			comma[9] = strstr(comma[8]+1, ",");		
#endif
			memcpy(GPSBF_GGA_DATA.UTCTime,comma[0]+1,(comma[1]-comma[0]-1));
			memcpy(GPSBF_GGA_DATA.Latitude,comma[1]+1,(comma[2]-comma[1])-1);
			memcpy(GPSBF_GGA_DATA.NS,comma[2]+1,(comma[3]-comma[2]-1));
			memcpy(GPSBF_GGA_DATA.Longitude,comma[3]+1,(comma[4]-comma[3]-1));
			memcpy(GPSBF_GGA_DATA.EW,comma[4]+1,(comma[5]-comma[4]-1));
		//	memcpy(GPSBF_GGA_DATA.PositionFix,comma[5]+1,(comma[6]-comma[5]-1));
		//	memcpy(GPSBF_GGA_DATA.SatUsed,comma[6]+1,(comma[7]-comma[6]-1));
		//	memcpy(GPSBF_RMC_DATA.Course,comma[7]+1,(comma[8]-comma[7]-1));
		//	memcpy(GPSBF_RMC_DATA.UTCDate,comma[8]+1,(comma[9]-comma[8]-1));

			memcpy(GPS_GGA_Data.EW,GPSBF_GGA_DATA.EW,sizeof(GPSBF_GGA_DATA.EW));
		    memcpy(GPS_GGA_Data.NS,GPSBF_GGA_DATA.NS,sizeof(GPSBF_GGA_DATA.NS));

							//time date
			time=atoi(GPSBF_GGA_DATA.UTCTime);
				
			GPS_GGA_Data.UTCTime[0]=(u8)((time/10000)%100);
			GPS_GGA_Data.UTCTime[1]=(u8)((time/100)%100);
			GPS_GGA_Data.UTCTime[2]=(u8)(time%100);

			GPS_GGA_Data.UTCTime[0]+=8;
			if (GPS_GGA_Data.UTCTime[0]>23)
			{
				GPS_GGA_Data.UTCTime[0]-=24;
			}

							//latitude
			fn=atof(GPSBF_GGA_DATA.Latitude);
			GPS_GGA_Data.Latitude[0]=(u8)(fn/100);
			GPS_GGA_Data.Latitude[1]=(u8)(((int)fn)%100);
			GPS_GGA_Data.Latitude[2]=(u8)((fn-(float)((int)fn))/0.01);
			GPS_GGA_Data.Latitude[3]=(u8)((int)((fn-(float)((int)fn))/0.0001)%100);
			//longitude
			fe=atof(GPSBF_GGA_DATA.Longitude);
			GPS_GGA_Data.Longitude[0]=(u8)(fe/10000);
			GPS_GGA_Data.Longitude[1]=(u8)(((int)(fe/100))%100);
			GPS_GGA_Data.Longitude[2]=(u8)(((int)fe)%100);
			GPS_GGA_Data.Longitude[3]=(u8)((fe-(float)((int)fe))/0.01);
			GPS_GGA_Data.Longitude[4]=(u8)((int)((fe-(float)((int)fe))/0.0001)%100);
#if 0
			status=atoi(GPSBF_GGA_DATA.PositionFix);
			used=atoi(GPSBF_GGA_DATA.SatUsed);
			GPS_GGA_Data.PositionFix=status;
			GPS_GGA_Data.SatUsed=used;
			printf("status=%d\n",status);
			if(1==status)           //valid positioning
			{			
				printf(" GPGGA GPS valid positioning info:\n");
			}
			else
			{
				printf("GPGGA GPS invalid positioning info:\n");
			}
#endif
			printf("GGA GPS positioning info:\n");
printf("latitude: %d%d.%d%d    %c\n",GPS_GGA_Data.Latitude[0],GPS_GGA_Data.Latitude[1],GPS_GGA_Data.Latitude[2],GPS_GGA_Data.Latitude[3],GPS_GGA_Data.NS[0]);
printf("longitude: %d%d%d.%d%d   %c\n",GPS_GGA_Data.Longitude[0],GPS_GGA_Data.Longitude[1],GPS_GGA_Data.Longitude[2],GPS_GGA_Data.Longitude[3],GPS_GGA_Data.Longitude[4],GPS_GGA_Data.EW[0]);
			printf("satellites No: %d\n",5);
			printf("time: %d:%d:%d\n",GPS_GGA_Data.UTCTime[0],GPS_GGA_Data.UTCTime[1],GPS_GGA_Data.UTCTime[2]);						
		}	
	}
		
return;
}



 
 /*
 *************************************************************************************************************
 - function name : int write_datas_gprs(int fd, unsigned char *buffer, int buf_len)
 - function declaration : send data to serial port
 - input parameter : fd:serial device handle£»buffer:need to be sent data buffer area pointer£»buf_len£ºdata length
 - output parameter : null
 *************************************************************************************************************
 */
 static int write_datas_tty(int fd, unsigned char *buffer, int buf_len)
 {
	 struct timeval tv;
	 fd_set w_set;
	 int bytes_to_write_total = buf_len;
	 int bytes_have_written_total = 0;
	 int bytes_write = 0;
	 int result = -1;
	 unsigned char *ptemp = buffer;
 
	 if ((fd<0) ||( NULL == buffer )|| (buf_len <=0))
	 {
		 printf("Send Buffer is Nc\n");
		 return -1;
	 }
 
	 while (bytes_to_write_total > 0)
	 {
		 FD_ZERO(&w_set);
		 FD_SET(fd,&w_set);
		 tv.tv_sec = 10;
		 tv.tv_usec = 0;
		 result = select(fd+1, NULL, &w_set, NULL, &tv);
		 if (result < 0)
		 {
			 if (EINTR == errno)
			 {
				 continue;
			 }
			 perror("Write socket select()");
			 return -1;
		 }
		 else if (0 == result)	  //this means timeout, it is not an error, so we return 0.
		 {
			 printf("Send Data Timeout --->3\n");
			 return 0;
		 }
		 else
		 {
			 if (FD_ISSET(fd, &w_set))
			 {
 
				 printf("W socket=%03d::%s \n",bytes_to_write_total,ptemp);
 
				 bytes_write = send(fd, ptemp, bytes_to_write_total, 0);
 
				 printf("bytes_write = %02X\n",bytes_write);
 
				 if (bytes_write < 0)
				 {
					 if (EAGAIN == errno || EINTR == errno)
					 {
						 continue;
					 }
					 printf("open Send Macine is error2\n");
					 return -1;
				 }
				 else if (0 == bytes_write)
				 {
					 printf("Send Data Timeout --->2\n");
					 return -1; 
				 }
				 else
				 {
					 bytes_to_write_total -= bytes_write;
					 bytes_have_written_total += bytes_write;
					 ptemp += bytes_have_written_total;
					 printf("Write GPS data\n");
				 }
			 }
		 }
	 }

	 printf("----------------->Send Data OK\n");
	 return 0;
 }




int myPrintfChar(char * name ,char *buf, int len)
{	
	int i;
	printf("%s : ", name);
	for(i=0; i<len; i++)
		printf("%c ", buf[i]);
	printf("\n");
}

 /*
 *************************************************************************************************************
 - function name :void ParserGPRMC(void)
 - function declaration : Parsing the RMC data format
 - input parameter : null
 - output parameter : null
 *************************************************************************************************************
 */
 unsigned char ParserGPRMC(void)
{
	int i,iLen=0,time,date;
	char lcdtest[300];
	char rx_string[2048] = {0};
	char *comma[20];
	char *UTCTime;
	float fn,fe,sp,co, TempPosition;
	int ret;
	

	bzero(rx_string,2048);
	signal(SIGPIPE,SIG_IGN);  //close SIGPIPE sinal anti-lockup
	ret=read_datas_ttygps(gps_fd,rx_string,&iLen);
	if(ret){
		//printf("read gps none\n");
		return 0;
	}
/*
	printf("GPS read data No.:%d\n",iLen);
    for(i=0;i<iLen;i++)
    {
        printf("%c",rx_string[i]);
        }
    printf("\n");
    */
	
	if(iLen > 0)
	{
		bzero(lcdtest,sizeof(lcdtest));
		if(gps_date_RMC(rx_string,lcdtest) == 0)
		{
			//TextOut(1,192,lcdtest,GB2312_16);
			//printf("---- : %s\n",lcdtest);
			comma[0] = strstr(lcdtest, ",");  //UTC time£¬hhmmss(hour minute second)format
			if(NULL!=comma[0])
			{
				for(i=1;i<10;i++)
				{
					comma[i] = strstr(comma[i-1]+1, ",");
                   
					if(NULL==comma[i])
					{
						DebugPrintf("GPRMC info incomplete1\n");
						return 0;
					}
				}
			}
			else
			{
				DebugPrintf("GPRMC info incomplete2\n");
				return 0;
			}
			memcpy(GPSBF_RMC_DATA.UTCTime,comma[0]+1,(comma[1]-comma[0]-1));
			memcpy(GPSBF_RMC_DATA.Status,comma[1]+1,(comma[2]-comma[1])-1);
			memcpy(GPSBF_RMC_DATA.Latitude,comma[2]+1,(comma[3]-comma[2]-1));
			memcpy(GPSBF_RMC_DATA.NS,comma[3]+1,(comma[4]-comma[3]-1));
			memcpy(GPSBF_RMC_DATA.Longitude,comma[4]+1,(comma[5]-comma[4]-1));
			memcpy(GPSBF_RMC_DATA.EW,comma[5]+1,(comma[6]-comma[5]-1));
			memcpy(GPSBF_RMC_DATA.Speed,comma[6]+1,(comma[7]-comma[6]-1));
			memcpy(GPSBF_RMC_DATA.Course,comma[7]+1,(comma[8]-comma[7]-1));
			memcpy(GPSBF_RMC_DATA.UTCDate,comma[8]+1,(comma[9]-comma[8]-1));

			memcpy(GPS_RMC_Data.Status,GPSBF_RMC_DATA.Status,sizeof(GPSBF_RMC_DATA.Status));


			memcpy(GPS_RMC_Data.EW,GPSBF_RMC_DATA.EW,sizeof(GPSBF_RMC_DATA.EW));
			memcpy(GPS_RMC_Data.NS,GPSBF_RMC_DATA.NS,sizeof(GPSBF_RMC_DATA.NS));
				
			//time date
			time=atoi(GPSBF_RMC_DATA.UTCTime);
			date=atoi(GPSBF_RMC_DATA.UTCDate);
			
			GPS_RMC_Data.UTCDateTime[3]=(u8)((time/10000)%100);
			GPS_RMC_Data.UTCDateTime[4]=(u8)((time/100)%100);
			GPS_RMC_Data.UTCDateTime[5]=(u8)(time%100);
			GPS_RMC_Data.UTCDateTime[2]=(u8)((date/10000)%100);
			GPS_RMC_Data.UTCDateTime[1]=(u8)((date/100)%100);
			GPS_RMC_Data.UTCDateTime[0]=(u8)(date%100);
			//latitude
			fn=atof(GPSBF_RMC_DATA.Latitude);
			GPS_RMC_Data.Latitude[0]=(u8)(fn/100);
			GPS_RMC_Data.Latitude[1]=(u8)(((int)fn)%100);
			GPS_RMC_Data.Latitude[2]=(u8)((fn-(float)((int)fn))/0.01);
			GPS_RMC_Data.Latitude[3]=(u8)((int)((fn-(float)((int)fn))/0.0001)%100);
			//longitude
			fe=atof(GPSBF_RMC_DATA.Longitude);
			GPS_RMC_Data.Longitude[0]=(u8)(fe/10000);
			GPS_RMC_Data.Longitude[1]=(u8)(((int)(fe/100))%100);
			GPS_RMC_Data.Longitude[2]=(u8)(((int)fe)%100);
			GPS_RMC_Data.Longitude[3]=(u8)((fe-(float)((int)fe))/0.01);
			GPS_RMC_Data.Longitude[4]=(u8)((int)((fe-(float)((int)fe))/0.0001)%100);

			//speed
			sp=atof(GPSBF_RMC_DATA.Speed);
			GPS_RMC_Data.Speed[0]=(u8)(sp/100);
			GPS_RMC_Data.Speed[1]=(u8)(((int)sp)%100);
			GPS_RMC_Data.Speed[2]=(u8)((sp-(float)((int)sp))/0.01);

			//course
			co=atof(GPSBF_RMC_DATA.Course);
			GPS_RMC_Data.Course[0]=(u8)(co/100);
			GPS_RMC_Data.Course[1]=(u8)(((int)co)%100);
			GPS_RMC_Data.Course[2]=(u8)((co-(float)((int)co))/0.01);
			ShowGPSTime();
			
			if(0==strcmp(GPS_RMC_Data.Status,"A"))           //valid positioning
			{			
//				printf("GPRMC GPS valid positioning info:\n");
			}
			else
			{
			//	printf("GPRMC GPS invalid positioning info:\n");
                memset(GPS_RMC_Data.UTCDateTime,0,sizeof(stru_GPSRMC));
                return 0;
			}
			TempPosition = (GPS_RMC_Data.Latitude[1] + GPS_RMC_Data.Latitude[2]*0.01 + GPS_RMC_Data.Latitude[3]*0.0001)/60;
			DUSUGAO_GPS_latitude = GPS_RMC_Data.Latitude[0] + TempPosition;
			
			TempPosition = (GPS_RMC_Data.Longitude[2] + GPS_RMC_Data.Longitude[3]*0.01 + GPS_RMC_Data.Longitude[4]*0.0001)/60;
			DUSUGAO_GPS_longitude = GPS_RMC_Data.Longitude[0]*100 + GPS_RMC_Data.Longitude[1] + TempPosition;
#if 0			
			printf("original RMC GPS positioning info:\n");
			GPS_latitude = GPS_RMC_Data.Latitude[0] + GPS_RMC_Data.Latitude[1]*0.01 + GPS_RMC_Data.Latitude[2]*0.0001 + GPS_RMC_Data.Latitude[3]*0.000001;
			GPS_longitude = GPS_RMC_Data.Longitude[0]*100 + GPS_RMC_Data.Longitude[1] + GPS_RMC_Data.Longitude[2]*0.01 + GPS_RMC_Data.Longitude[3]*0.0001 + GPS_RMC_Data.Longitude[4]*0.000001;
			printf("latitude: %f %c\n", GPS_latitude, GPS_RMC_Data.NS[0]);
			printf("longitude: %f %c\n", GPS_longitude, GPS_RMC_Data.EW[0]);
			
			//DebugPrintf("latitude: %d%d.%d%d    %c\n",GPS_RMC_Data.Latitude[0],GPS_RMC_Data.Latitude[1],GPS_RMC_Data.Latitude[2],GPS_RMC_Data.Latitude[3],GPS_RMC_Data.NS[0]);
			//DebugPrintf("longitude: %d%d%d.%d%d   %c\n",GPS_RMC_Data.Longitude[0],GPS_RMC_Data.Longitude[1],GPS_RMC_Data.Longitude[2],GPS_RMC_Data.Longitude[3],GPS_RMC_Data.Longitude[4],GPS_RMC_Data.EW[0]);
			printf("speed: %d%d.%d\n",GPS_RMC_Data.Speed[0],GPS_RMC_Data.Speed[1],GPS_RMC_Data.Speed[2]);
			//DebugPrintf("course: %d%d.%d\n",GPS_RMC_Data.Course[0],GPS_RMC_Data.Course[1],GPS_RMC_Data.Course[2]);
			//DebugPrintf("time: 20%d-%d-%d %d:%d:%d\n",GPS_RMC_Data.UTCDateTime[0],GPS_RMC_Data.UTCDateTime[1],GPS_RMC_Data.UTCDateTime[2],GPS_RMC_Data.UTCDateTime[3],GPS_RMC_Data.UTCDateTime[4],GPS_RMC_Data.UTCDateTime[5]);
#else
			DebugPrintf("UTCTime = %s\n",GPSBF_RMC_DATA.UTCTime);
			DebugPrintf("Status = %s\n",GPSBF_RMC_DATA.Status);
			DebugPrintf("Latitude = %s\n",GPSBF_RMC_DATA.Latitude);
			DebugPrintf("NS = %s\n",GPSBF_RMC_DATA.NS);
			DebugPrintf("Longitude = %s\n",GPSBF_RMC_DATA.Longitude);
			DebugPrintf("EW = %s\n",GPSBF_RMC_DATA.EW);
			DebugPrintf("Speed = %s\n",GPSBF_RMC_DATA.Speed);
			DebugPrintf("Course = %s\n",GPSBF_RMC_DATA.Course);
			DebugPrintf("UTCDate = %s\n",GPSBF_RMC_DATA.UTCDate);	
#endif		
            return 1;
		}
		return 0;
	}

    return 0;
}



int d2hex(u8 data)
{
	int a=0;
	a=(data/10)*16+((int)data%10);
	return a;
}

/*
*************************************************************************************************************
- function name : char * Rd_time (char* buff)
- function declaration : read time
- input parameter : null
- output parameter : null
*************************************************************************************************************
*/
char  GpsGetRmc(unsigned char* buff)
{
	time_t t;
	struct tm * tm;
	char ret = 0;
	char ns = 0;
	char ew = 0;
	int la=0,lo=0,speed=0,Course=0;
	float n;
	
	time (&t);
	tm = localtime (&t);
	char buf[30];
	buff[0] = d2hex((tm->tm_year+1900)/100);//HEX2BCD((unsigned char)20);		
	buff[1] = d2hex((tm->tm_year+1900)%100);//HEX2BCD((unsigned char)GPS_RMC_Data.UTCDateTime[0]);
	buff[2] = d2hex(tm->tm_mon+1);//HEX2BCD((unsigned char)GPS_RMC_Data.UTCDateTime[1]);
	buff[3] = d2hex(tm->tm_mday);//HEX2BCD((unsigned char)GPS_RMC_Data.UTCDateTime[2]);
	buff[4] = d2hex(GPS_RMC_Data.UTCDateTime[3]);//HEX2BCD((unsigned char)GPS_RMC_Data.UTCDateTime[3]);
	buff[5] = d2hex(GPS_RMC_Data.UTCDateTime[4]);//HEX2BCD((unsigned char)GPS_RMC_Data.UTCDateTime[4]);
	buff[6] = d2hex(GPS_RMC_Data.UTCDateTime[5]);//HEX2BCD((unsigned char)GPS_RMC_Data.UTCDateTime[5]);
	buff[7] = GPS_RMC_Data.NS[0];
	ns = GPS_RMC_Data.NS[0];
	la = (GPS_RMC_Data.Latitude[0]*1000000)+(GPS_RMC_Data.Latitude[1]*10000)+(GPS_RMC_Data.Latitude[2]*100)+GPS_RMC_Data.Latitude[3];	
	if(la<0)
	{
		la=0;
	}
	n = (la%1000000)/600000.0L + (float)(la/1000000);
	la = n*1000000;
	buff[8] = (la>>0)&0xff;  //GPS_GGA_Data.Latitude[0];
	buff[9] = (la>>8)&0xff;  //GPS_GGA_Data.Latitude[1];
	buff[10] = (la>>16)&0xff;  //GPS_GGA_Data.Latitude[2];
	buff[11] = (la>>24)&0xff;  //GPS_GGA_Data.Latitude[3];
	buff[14] = GPS_RMC_Data.EW[0];
	ew = GPS_RMC_Data.EW[0];
	lo=(GPS_RMC_Data.Longitude[0]*100000000)+(GPS_RMC_Data.Longitude[1]*1000000)+(GPS_RMC_Data.Longitude[2]*10000)+(GPS_RMC_Data.Longitude[3]*100)+GPS_RMC_Data.Longitude[4];
	if(lo<0)
	{
		lo=0;
	}
	n = (lo%1000000)/600000.0L + (float)(lo/1000000);
	lo = n*1000000;
	buff[15] = (lo>>0)&0xff; //GPS_GGA_Data.Longitude[0];
	buff[16] = (lo>>8)&0xff;  //GPS_GGA_Data.Longitude[1];
	buff[17] = (lo>>16)&0xff;  //GPS_GGA_Data.Longitude[2];
	buff[18] = (lo>>24)&0xff;  //GPS_GGA_Data.Longitude[3];
	speed=(GPS_RMC_Data.Speed[0]*10000)+(GPS_RMC_Data.Speed[1]*100)+GPS_RMC_Data.Speed[2];
	buff[21] = 0x00;
	buff[22] = (speed>>16)&0xff;
	buff[23] = (speed>>8)&0xff;
	buff[24] = speed&0xff;
	Course=(GPS_RMC_Data.Course[0]*10000)+(GPS_RMC_Data.Course[1]*100)+GPS_RMC_Data.Course[2];
	buff[25] = (Course>>8)&0xff;
	buff[26] = Course&0xff;

	if(buff[1] != 0 && buff[2] != 0 && buff[3] != 0)      //year month day can not be 0
	{
		if((ns==78||ns==83)&&(ew==69||ew==87))    //latitude longitude must be N S E W
		{
			if((la!=0)&&(lo!=0))//latitude and longitude can not be 0
			{
            			ret = 1;
			}
		}
	}
    return ret;
}

void InitGps()
{
	gps_fd = initializegps("ttyC2", 9600);
	
	tcflush (gps_fd, TCIFLUSH);
}

void CloseGps()
{
	if( gps_fd > 0 )
	{
		uninitializegps(gps_fd);
	}
}

int GpsGetLocation(char *outdat)
{
	int ret = 0;	
	if(ParserGPRMC())
	{
		ret = GpsGetRmc(outdat);
	}
	return ret;
}
#if 0
 /*
 *************************************************************************************************************
 - function name : int main(int argc,char *argv[])
 - function declaration : main function
 - input parameter : null
 - output parameter : null
 *************************************************************************************************************
 */
int main(int argc,char *argv[])
{
    printf("GPS pthread run!\n");
    unsigned char status;
    int num;
	unsigned char  buff1[200];	
	gps_fd = initializegps("ttyC2", 9600);
	
	tcflush (gps_fd, TCIFLUSH);
	printf("Begin to Read:\n");
    num=0;
	for(;;)
	{
		//ParserGPGGA();
		status = ParserGPRMC();
        if(status)
		{
            memset(buff1,0xFF,sizeof(buff1));
    		gpsRd_time(buff1);
    		//buff1[7] = 78;
    		//buff1[14]= 69;
    		//printf("current address%s\n",buff1);
    		if(buff1[1] != 0 && buff1[2] != 0 && buff1[3] != 0)      //year month day can not be0
    		{
    			if((buff1[7]==78||buff1[7]==83)&&(buff1[14]==69||buff1[14]==87))    //latitude longitude must be N S E W
    			{
    				if((buff1[8]!=0||buff1[9]!=0||buff1[10]!=0||buff1[11]!=0)&&(buff1[15]!=0||buff1[16]!=0||buff1[17]!=0||buff1[18]!=0))//latitude and longitude can not be 0
    				{
                        GPSSIG = 0;
                        num=0;    					
    					memset(buffer0,0,sizeof(buffer0));
    					memcpy(buffer0,buff1,sizeof(buff1));
    				}
    				else
    				{
                        num++;
                        if(num>500)
                        {
                            num=0;            
                            GPSSIG = 1;
                        }    					
    				}
    			}
    			else
    			{
                    num++;
                    if(num>500)
                    {
                        num=0;            
                        GPSSIG = 1;
                        };
    			//	gpsDisplay_signal(1);
    			}
    		}
    		else
    		{
                num++;
                if(num>500)
                {
                    num=0;            
                    GPSSIG = 1;
                }
    		//	gpsDisplay_signal(1);
    		}
    		//sleep(1);
        }
        else
        {
            num++;
            if(num>500)
            {
                num=0;            
                GPSSIG = 1;
            }
          //  gpsDisplay_signal(1);
        }
    }
	
	if( gps_fd > 0 )
	{
		uninitializegps(gps_fd);
	}
return 0;
}
#endif
