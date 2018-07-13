
#include <sys/time.h>

#include "include/CL1306.h"
#include "include/gpio.h"
#include "include/player.h"
#include "include/read.h"
#include "include/net.h"
#include "include/psam.h"
#include "include/display.h"
#include "include/keyboard.h"

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <linux/sockios.h>
#include <errno.h>


#include "main.h"
#include "Config.h"



SYS_PARAM gparam;

char gszbuf[128];
int gsocket = -1;
pthread_t gdialid; // thread id of gprs dial
pthread_t gdialrun = 0; // thread runing flag

void InitFont()
{
	if(Open_Frambuffer( "/dev/fb0")==MI_OK)
	{
		Init_Hzk(); 	
		
		if(Insert_Hzk("/mnt/nand1-2/app/font/hzk16c_ASC.DZK",GB2312_16,HZK_MIXUER)==0)
			printf("load font library 16 succeed\n");
		if(Insert_Hzk("/mnt/nand1-2/app/font/hzk32c_ASC.DZK",GB2312_32,HZK_MIXUER)==0)
			printf("load font library 32 succeed\n");
		if(Insert_Hzk("./hzk16c_GBK.DZK",GBK_16,HZK_CHZ)==0)
			printf("load gbk font library 16 succeed\n");
		
		if(Insert_Hzk("/mnt/nand1-2/app/font/hzk24z_ASC.DZK",GB2312_24,HZK_CHZ)==0)
					printf("load Chinses font library 24 succeed\n");
		if(Insert_Hzk("/mnt/nand1-2/app/font/hzk24c_ASC.DZK",GB2312_24,HZK_ENZ)==0)
			printf("load English font library 24 succeed\n");
	}

}

#define ETHTOOL_GLINK        0x0000000a /* Get link status (ethtool_value) */
typedef enum {IFSTATUS_UP, IFSTATUS_DOWN, IFSTATUS_ERR} interface_status_t;
struct ethtool_value
{
	uint32_t cmd;
	uint32_t data;
};

void DebugOut(char *str)
{
	fillrectusebackground(1,208,160-1, 240-16);
	Update_Frambuffer(1,208,160-1, 240-16);
	TextOut(1, 208, str,GB2312_16);
}
interface_status_t interface_detect_beat_ethtool(int fd, char *iface)
{  
    struct ifreq ifr;  
    struct ethtool_value edata;  
     
    memset(&ifr, 0, sizeof(ifr));  
    strncpy(ifr.ifr_name, iface, sizeof(ifr.ifr_name)-1);  
  
    edata.cmd = ETHTOOL_GLINK;  
    ifr.ifr_data = (caddr_t) &edata;  
  
    if (ioctl(fd, SIOCETHTOOL, &ifr) == -1)  
    {  
        perror("ETHTOOL_GLINK failed ");  
        return IFSTATUS_ERR;  
    }  
  
    return edata.data ? IFSTATUS_UP : IFSTATUS_DOWN;  
}  
int GetLinkStatus()
{
	interface_status_t status;
	int fd;
	if((fd = socket(AF_INET, SOCK_STREAM, 0)) != -1)
	{
		char hw_name[10] = {0};
		status = interface_detect_beat_ethtool(fd, hw_name);
		close(fd);
		if(status == IFSTATUS_UP)
			return 1;
		else if(status == IFSTATUS_DOWN)
			return 0;
	}
	return -1;
}
int GetLinkStatus2()
{
	FILE *fp;  
	int status = -1;  
	char buf[512] = {'\0'};  
	char hw_name[10] = {'\0'};  
	char *token = NULL;  

	/* get adapter name */  
	if ((fp = fopen("/proc/net/dev", "r")) != NULL)  
	{  
		while (fgets(buf, sizeof(buf), fp) != NULL)  
		{  
			if(strstr(buf, "ppp") != NULL)  // ppp0, NOT eth0
			{         
				token = strtok(buf, ":");  
				while (*token == ' ') ++token;  
				strncpy(hw_name, token, strlen(token));  
			}  
		}  
	}  
	fclose(fp);  

	// open file
	char carrier_path[512] = {'\0'};  

	memset(buf, 0, sizeof(buf));   
	snprintf(carrier_path, sizeof(carrier_path), "/sys/class/net/%s/carrier", hw_name);  
	if ((fp = fopen(carrier_path, "r")) != NULL)  
	{  
		while (fgets(buf, sizeof(buf), fp) != NULL)  
		{  
			if (buf[0] == '0')  
			{  
				status = 0;  
			}  
			else  
			{  
				status = 1;  
			}  
		}  
	}  
	else  
	{  
		perror("Open carrier ");  
	}  
	fclose(fp);  

	return status;
}
uint32_t GetTick()
{
	uint32_t nms;

	struct timeb
	{
		time_t time;
		unsigned short millitm;
		short timezonel;
		short dstflag;
	};
	struct timeb tb;
	
	ftime(&tb);
	nms = tb.time*1000 + tb.millitm;

	return nms;
}

int DisplayIfconfig()
{  
	FILE *fp;  
	char buf[512];
	int y = 32;

	fp=popen("ifconfig","r");  
	if(fp != NULL)  
	{
		while(fgets(buf,sizeof(buf),fp)!=NULL)     // get line
		{  
			TextOut(1,y, buf,GB2312_16);
			y += 16;
		}
		pclose(fp);
	}
	return 0;  
}
int GetIfconfig()
{  
	FILE *fp;  
	char buf[512];
	int ret = 0;

	fp=popen("ifconfig | grep RUNNING","r");  
	if(fp != NULL)  
	{
		while(fgets(buf,sizeof(buf),fp)!=NULL)     // get line
		{  
			ret = 1;
		}
		pclose(fp);
	}
	return ret;  
}

void InitConfig()
{
	INI_CONFIG *pconfig;
	
	pconfig = ini_config_create_from_file(CONFIG_FILE, 0);
	if(pconfig != NULL)
	{
		char *pstr;
		pstr = ini_config_get_string(pconfig, NULL, "uid", "DDCCBBAA");
		strcpy(gszbuf, pstr);
		AsciiToHex(gszbuf, strlen(gszbuf), &gparam.uid);
		
		gparam.timeout = ini_config_get_int(pconfig, NULL, "interval", 60);
		
		pstr = ini_config_get_string(pconfig, NULL, "serverip", "103.46.128.47");
		strcpy(gparam.serverip, pstr);
		gparam.serverport = ini_config_get_int(pconfig, NULL, "serverport", 30365);

		ini_config_destroy(pconfig);
	}
	else
	{
		char *pstr = "uid = AABBCCDD \n interval = 60 \n serverip = 103.46.128.47 \n serverport = 30365 \n \n ";
		gparam.uid = 0xDDCCBBAA;
		gparam.timeout = 60;
		strcpy(gparam.serverip, "103.46.128.47");
		gparam.serverport = 30365;

		pconfig = ini_config_create_from_string(pstr, 0, 0);
		if(pconfig != NULL)
		{
			ini_config_save(pconfig, CONFIG_FILE);
			ini_config_destroy(pconfig);
		}

	}
	
//		FILE *pfile;
//		pfile = fopen("/mnt/nand1-2/app/uid.txt", "rb");
//		if(pfile != NULL)
//		{
//			fread(gszbuf, 1, 8, pfile);
//			fclose(pfile);
//			AsciiToHex(gszbuf, 8, &gparam.uid);
//		}
//		else
//		{
//			gparam.uid = 0xAABBCCDD;
//		}
//		
//		pfile = fopen("/mnt/nand1-2/app/timeout.txt", "rb");
//		if(pfile != NULL)
//		{
//			fread(&gparam.timeout, 1, 4, pfile);
//			fclose(pfile);
//		}
//		else
//			gparam.timeout = 60;
//			
//		pfile = fopen("/mnt/nand1-2/app/tcpserver.txt", "rb");
//		if(pfile != NULL)
//		{
//			fread(&gparam.timeout, 1, 4, pfile);
//			fclose(pfile);
//		}
//		else
//			gparam.timeout = 60;
}
void InitGprs()
{
	//DisplayIfconfig();

	gdialrun = 1;
	
	DebugOut("Gprs Init");
	Gprs_Killpppd();
	
	DebugOut("Gprs Reset");
	if(Gprs_Rest()==MI_OK)
	{
		DebugOut("Gprs Pppd");
		Gprs_Callpppd();
		
		//printf("ifconfig:\n");
		//system("ifconfig");
		//DisplayIfconfig();
	}
	gdialrun = 0;
}

void InitTcp()
{
	pid_t fd;
	unsigned long flag = 0;
	fd_set fdr, fdw;
	int res;
	struct timeval timeout;

	struct sockaddr_in serveraddr;

	gsocket = socket(AF_INET,SOCK_STREAM,0);
	if(gsocket<0)
	{
		DebugOut("socket create fail");
		return;
	}
	bzero(&serveraddr,sizeof(serveraddr)); 
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(gparam.serverport);
	inet_pton(AF_INET, gparam.serverip, &serveraddr.sin_addr); 

	// none-blocking setting
	flag = fcntl(gsocket, F_GETFL, 0);
	fcntl(gsocket, F_SETFL, flag|O_NONBLOCK);

	DebugOut("socket connecting...");
	if(connect(gsocket,(struct sockaddr *)&serveraddr,sizeof(serveraddr))!=0)
	{
		if(errno != EINPROGRESS) // EINPROGRESS 
		{
			close(gsocket);
			gsocket = -1;
			DebugOut("socket connect fail");
			return;
		}
	}

	FD_ZERO(&fdr);
	FD_ZERO(&fdw);
	FD_SET(gsocket, &fdr);
	FD_SET(gsocket, &fdw);

	timeout.tv_sec = 5;
	timeout.tv_usec = 0;

	res = select(gsocket + 1, &fdr, &fdw, NULL, &timeout);
	if(res < 0)
	{
		DebugOut("socket select fail");
		close(gsocket);
		return;
	}
	else if(res == 0)
	{
		DebugOut("socket connect timeout");
		close(gsocket);
		return ;
	}
	else if(res == 1)
	{
		if(FD_ISSET(gsocket, &fdw))
		{
			DebugOut("socket connected");
		}
	}
	else if(res == 2)
	{
		DebugOut("socket connect timeout");
		close(gsocket);
		return ;
	}
}

void OnTcpSend(uint8_t *pbuf, uint16_t len)
{
	int n;
	if(gsocket!=-1)
	{
		n = send(gsocket,pbuf,len, MSG_DONTWAIT);
	}
	if(n<0)
	{
		close(gsocket);
		gsocket = -1;
		
		gparam.online = 0;
		gparam.draw |= DRAW_CAPTION;
		gparam.event |= EVENT_DRAW;
	}
}

void OnTcpLink()
{
	static unsigned int tcptick = 0;
	
	tcptick++;
	if((tcptick%100)==0) // 5s
	{
		if(GetIfconfig() == 1)
		{
			if(gsocket != -1)
			{
				if(gparam.online == 0)
				{
					gparam.online = 1;
					gparam.draw |= DRAW_CAPTION;
					gparam.event |= EVENT_DRAW;
				}
			}
			else
			{
				if(gparam.online)
				{
					gparam.online = 0;
					gparam.draw |= DRAW_CAPTION;
					gparam.event |= EVENT_DRAW;
				}
				InitTcp();
			}																								 
		}
		else
		{
			if(gparam.online)
			{
				gparam.online = 0;
				gparam.draw |= DRAW_CAPTION;
				gparam.event |= EVENT_DRAW;
			}
			if(gdialrun==0)
			{
				pthread_create(&gdialid, NULL, (void*)InitGprs, NULL);
			}
		}
	}
	
	if((gsocket != -1))//&&(gparam.online))
	{
		int ret = -1;
		#if 0
		默认recv函数socket 是阻塞的
		阻塞与非阻塞recv返回值没有区分，都是
		<0 出错
		=0 连接关闭
		>0 接收到数据大小，

		特别：返回值<0时并且(errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN)的情况下认为连接是正常的，继续接收。
		只是阻塞模式下recv会阻塞着接收数据，非阻塞模式下如果没有数据会返回，不会阻塞着读，因此需要循环读取）。

		返回说明：
		成功执行时，返回接收到的字节数。另一端已关闭则返回0。失败返回-1，errno被设为以下的某个值
		EAGAIN：11,套接字已标记为非阻塞，而接收操作被阻塞或者接收超时
		EBADF：sock不是有效的描述词
		ECONNREFUSE：111,远程主机阻绝网络连接
		EFAULT：内存空间访问出错
		EINTR：操作被信号中断
		EINVAL：参数无效
		ENOMEM：内存不足
		ENOTCONN：与面向连接关联的套接字尚未被连接上
		ENOTSOCK：sock索引的不是套接字

		当返回值是0时，为正常关闭连接；
		#endif
		
		ret = recv(gsocket, RxBuffer, UART1_RXSIZE, MSG_DONTWAIT);
		if(ret > 0)
		{

			RxPos = ret;
			//sprintf(gszbuf, "recv %d bytes:%02X %02X...", RxPos, RxBuffer[0], RxBuffer[1]);
			//DebugOut(gszbuf);

			// debug print
			#if 0
			{
				int i,j,y=16;
				char sztmp[8];

				sprintf(gszbuf, "recv %d bytes:", RxPos);
				TextOut(1, y, gszbuf,GB2312_16);

				
				for(i=0;i<=(ret/8);i++)
				{
					y += 16;
					gszbuf[0] = 0;
					for(j=0;j<8;j++)
					{
						sprintf(sztmp, "%02X ", RxBuffer[i*8+j]);
						strcat(gszbuf, sztmp);
					}
					TextOut(1, y, gszbuf,GB2312_16);
				}
					gszbuf[0] = 0;
					for(j=0;j<(ret%8);j++)
					{
						sprintf(sztmp, "%02X ", RxBuffer[i*8+j]);
						strcat(gszbuf, sztmp);
					}
					TextOut(1, y, gszbuf,GB2312_16);
			}
			#endif

			OnRecvData();
		}
		else if(ret == 0)
		{
			DebugOut("recv close");
			close(gsocket);
			gsocket = -1;
			gparam.online = 0;
		}
		else
		{
			//sprintf(gszbuf, "recv ret=%d, errno=%d", ret, errno); //EAGAIN
			//DebugOut(gszbuf);
		}
	}
}


void OnBeep()
{
	gparam.event |= EVENT_BEEP;
}
void OnKey(int key)
{
	if(key != -1)
	{
		//sprintf(gszbuf, "key:%d", key);
		//TextOut(1,200, gszbuf,GB2312_16);
		OnBeep();
		switch(key)
		{
		case '1': // up
			break;
		case '2': // down
			break;
		case '3': // ok
			break;
		case '4': // exit
			break;
		}
	}

}

void OnDraw()
{
	//Clear_Display();
	if(gparam.draw & DRAW_CAPTION)
	{
		gparam.draw &= ~DRAW_CAPTION;

		// header
		fillrectusebackground(1,1,320-1, 8+16);
		Update_Frambuffer(1,1,320-1, 8+16);
		TextOut(1,8, "孩子的校园",GB2312_16);
		if(gparam.online == 0)
		{
			BmpOut(256,8,"./res/connecting.bmp");
			TextOut(272,8, "连接中",GB2312_16);
		}
		else
		{
			BmpOut(256,8,"./res/connected.bmp");
			TextOut(272,8, "已连接",GB2312_16);
		}

		// footer
		fillrectusebackground(160,208,320-1, 240-16);
		Update_Frambuffer(160,208,320-1, 240-16);
		sprintf(gszbuf, "%08X", gparam.uid);
		TextOut(256,208, gszbuf,GB2312_16);
	}
	if(gparam.draw & DRAW_USRNAME)
	{
		gparam.draw &= ~DRAW_USRNAME;
		
		fillrectusebackground(1,16+8,320-1, 240-32);
		Update_Frambuffer(1,16+8,320-1, 240-32);
		
		Set_Font_Color(Color_green);
		TextOut(112,32, "请刷卡",GB2312_32);
		Set_Font_Color(Color_white);

		// y offset 40
		if(gparam.user.id)
			sprintf(gszbuf, "卡号:%010u", gparam.user.id);
		else
			sprintf(gszbuf, "卡号:");
		TextOut(32,80, gszbuf,GB2312_32);
		fillrectcoli(112, 112+4, 288, 114+4, Color_white);
		Update_Frambuffer(112, 112+4, 288, 114+4);
		
		// y offset 40
		sprintf(gszbuf, "姓名:%s", gparam.user.name);
		TextOut(32,120, gszbuf,GB2312_32);
		fillrectcoli(112, 152+4, 288, 154+4, Color_white);
		Update_Frambuffer(112, 152+4, 288, 154+4);
		
		// y offset 40
		sprintf(gszbuf, "电话:%s", gparam.user.phone);
		TextOut(32,160, gszbuf,GB2312_32);
		fillrectcoli(112, 192+4, 288, 194+4, Color_white);
		Update_Frambuffer(112, 192+4, 288, 194+4);
	}

	if(gparam.draw & DRAW_GPSINFO)
	{
		gparam.draw &= ~DRAW_GPSINFO;
		
		fillrectusebackground(1,240-16,160-1, 240-1);
		Update_Frambuffer(1,240-16,160-1, 240-1);
		if(gparam.gps.status)
		{
			float latitude, longitude;
			BmpOut(1,224,"./res/gprson.bmp");

			latitude = gparam.gps.latitude;
			longitude = gparam.gps.longitude;
			sprintf(gszbuf, "%f,%f", latitude/1000000, longitude/1000000);
			TextOut(16,224, gszbuf,GB2312_16);
		}
		else
		{
			BmpOut(1,224,"./res/gprsoff.bmp");
			TextOut(16,224, "定位中...",GB2312_16);
		}
	}
	
	if(gparam.draw & DRAW_SYSTIME)
	{
		struct tm * st;
		time_t curr;
		
		time(&curr);
		st = localtime(&curr);
		
		gparam.draw &= ~DRAW_SYSTIME;
		
		//fillrectusebackground(176,240-16,320-1, 240-1);
		//Update_Frambuffer(176,240-16,320-1, 240-1);
		sprintf(gszbuf, "%4d-%02d-%02d %02d:%02d", st->tm_year + 1900, st->tm_mon + 1, 
			st->tm_mday, st->tm_hour, st->tm_min);
		
		BmpOut(176,224,"./res/clock.bmp");
		TextOut(192,224, gszbuf,GB2312_16);
	}
	

}

void OnEvent()
{ // 50ms loop
	
	if(gparam.event & EVENT_DRAW)
	{
		gparam.event &= ~EVENT_DRAW;
		OnDraw();
	}
	if(gparam.event & EVENT_READTAG)
	{
		static unsigned int readtick = 0;
		//gparam.event &= ~EVENT_READTAG;

		readtick++;
		if((readtick%5) == 0) // 250ms
		{
			static unsigned int oldtagid = 0;
			unsigned int tagid;
			char csn[32];
			int csnLen;
			if((CardReset(csn,&csnLen))== 0x08)
			{
				memcpy(&tagid, csn, 4);

				if(oldtagid != tagid)
				{
					oldtagid = tagid;

					memset(&gparam.user, 0, sizeof(USER_INFO));
					gparam.user.id = tagid;
					gparam.draw |= DRAW_USRNAME;
					gparam.event |= EVENT_DRAW;
					gparam.gpstick = 0;
					
					
					{
					time_t t;
					UPLOAD_DATA ud;
					time (&t);
					ud.uid = gparam.uid; // should be read from file
					ud.tagid = tagid;
					ud.delay = gparam.timeout;
					ud.time = t-28800;//gparam.gps.gpstime;
					ud.latitude = gparam.gps.latitude;
					ud.longitude = gparam.gps.longitude;

					RecordPush(&ud);

					if(gparam.online)
					OnSetFrame(1, &ud, sizeof(UPLOAD_DATA));
					}

					gparam.tagtick = 0;
					
					OnBeep();
				}
			}
			else
			{
				oldtagid = 0;
			}
		}
	}

	if(gparam.event & EVENT_BEEP)
	{
		static unsigned int beeptick = 0;
		
		beeptick++;
		if(beeptick == 1)
		{
			buzz_on();
		}
		else// if(beeptick >= 50)
		{
			beeptick = 0;
			buzz_off();
			gparam.event &= ~EVENT_BEEP;
		}
	}
	if(gparam.event & EVENT_GPS)
	{
		static unsigned int gpstick = 0;
		
		gpstick++;
		if((gpstick%20)==0) // 1s
		{
			static int gpsdelay = 0;
			char dat[128]={0};
			if(GpsGetLocation(dat))
			{
				gpsdelay = 0;
				gparam.gps.status = 1;
				memcpy(&gparam.gps.latitude, dat+8, 4); 
				memcpy(&gparam.gps.longitude, dat+15, 4); 
				gparam.draw |= DRAW_GPSINFO;
				gparam.event |= EVENT_DRAW;

				
			}
			else
			{
				if(gparam.gps.status)
				{
					gpsdelay++;
					if(gpsdelay>60)
					{
						gpsdelay = 0;
						
						gparam.gps.status = 0;
						memset(&gparam.gps.latitude, 0, 4); 
						memset(&gparam.gps.longitude, 0, 4); 
						gparam.draw |= DRAW_GPSINFO;
						gparam.event |= EVENT_DRAW;
					}
				}
			}
			// send data every 60s
			gparam.gpstick++;
			if((gparam.gpstick%gparam.timeout)==0)
			{
				if(gparam.online)
				if(gparam.gps.status)
				{
					time_t t;
					UPLOAD_DATA ud;
					
					time (&t);
					
					ud.uid = gparam.uid; // should be read from file
					ud.tagid = 0;
					ud.delay = gparam.timeout;
					ud.time = t-28800;//gparam.gps.gpstime;
					ud.latitude = gparam.gps.latitude;
					ud.longitude = gparam.gps.longitude;
					
					OnSetFrame(1, &ud, sizeof(UPLOAD_DATA));
				}
			}
		}
	}
	if(gparam.event & EVENT_TCPLINK)
	{
		OnTcpLink();
	}
	
}

void OnTimer()
{ // 1s
	if(gparam.timeout_display) // 60s
	{
		gparam.timeout_display++;
		if(gparam.timeout_display>60)
		{
			DebugOut("");
			gparam.timeout_display = 0;
			memset(&gparam.user, 0, sizeof(USER_INFO));
			gparam.draw |= DRAW_USRNAME;
			gparam.event |= EVENT_DRAW;
		}
	}

	if(gparam.online)
	if(gparam.tagtick<600)
	{
		UPLOAD_DATA rec;

		gparam.tagtick++;
		if((gparam.tagtick%10)==0)
		if(RecordPop(&rec)==0)
		{
			OnSetFrame(1, &rec, sizeof(UPLOAD_DATA));
		}
		if(gparam.tagtick==600)
			RecordInit();
	}
	//else
	//	RecordInit();

	{
		fillrectusebackground(150,1,150+32, 16);
		Update_Frambuffer(150,1,150+32, 16);
		sprintf(gszbuf, "%d", RecordGet());
		TextOut(150, 1, gszbuf,GB2312_16);
	}

//		if(gparam.sendtick<30)
//		{
//			gparam.sendtick++;
//			if((gparam.sendtick % 10)==0)
//			{
//				OnTcpSend(TxBuffer, TxSize);
//			}
//			else
//			{
//				UPLOAD_DATA rec;
//				if(RecordPop(&rec)==0)
//				{
//					OnSetFrame(1, &rec, sizeof(UPLOAD_DATA));
//				}
//			}
//			if(gparam.sendtick == 30)
//			{
//				if(gsocket != -1)
//				{
//					close(gsocket);
//					gsocket = -1;
//					gparam.online = 0;
//					gparam.draw |= DRAW_CAPTION;
//					gparam.event |= EVENT_DRAW;
//				}
//			}
//		}
}
int main(int argc,char *argv[])
{
	uint32_t oldtick;
	uint32_t newtick;

	memset(&gparam, 0, sizeof(gparam));

	InitFont();
	Init_Gpio("/dev/fullgpio");
	Init_MF("/dev/typea");
	Open_KeyBoard( "/dev/input/event0");
	Init_Player("/mnt/nand1-2/app/sound",SND_MIXER,SND_DSP);
	//Play_Music("/mnt/nand1-2/app/sound/music02.wav");
	InitGps();
	//InitGprs();
	//InitTcp();
	InitConfig();

	RecordInit();

	gparam.event |= EVENT_READTAG;
	gparam.event |= EVENT_TCPLINK;
	gparam.event |= EVENT_GPS;

	//Set_Background("./res/background.bmp",0,USE_BACK_IMG);
	Set_Background(NULL,Color_blue,0);
	Set_Font_Color(Color_white);
	gparam.draw = DRAW_ALLITEM;
	gparam.event |= EVENT_DRAW;
	gparam.tagtick = 0xFFFFFFFF;

	oldtick = GetTick();
	while(1)
	{
		newtick = GetTick();
		if(__abs(newtick, oldtick)>=1) // actually 50ms
		{
			static unsigned int secondtick = 0;
			secondtick++;
			if((secondtick%100)==0) // 5s
			{
				gparam.draw |= DRAW_SYSTIME;
				gparam.event |= EVENT_DRAW;
			}
			if((secondtick%20)==0) // 1s
			{
				OnTimer();
			}
			
			oldtick = newtick;
			OnKey(Get_KeyCode());
			OnEvent();
		}
	}

	if(gsocket != -1)
		close(gsocket);
	//free(bmp.data);
	CloseKeyboard();
	Close_Player_Device();
	Delet_Hzk(0,DELET_ALL_HZK);
	Close_Frambuffer();
	CloseRF();
	return 0;

}
