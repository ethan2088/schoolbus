#ifndef __GPS_H__
#define __GPS_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>
#include <signal.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>


#define u8 unsigned char
typedef struct
{
u8 UTCDateTime[6]; //YMDHMS
char Status[20]; //A/V
u8 Latitude[4]; //ddmm.mmmm
char NS[20]; //N/S
u8 Longitude[5]; //dddmm.mmmm
char EW[20]; //E/W
u8 Speed[3]; //speed000.0~999.9kts
u8 Course[3]; //course000.0~359.9degree
}stru_GPSRMC;

typedef struct
{
char UTCTime[20]; //HMS
char Status[20]; //A/V
char Latitude[20]; //ddmm.mmmm
char NS[20]; //N/S
char Longitude[20]; //dddmm.mmmm
char EW[20]; //E/W
char Speed[20]; //speed000.0~999.9kts
char Course[20]; //course000.0~359.9degree
char UTCDate[20];//YMD

}strubf_GPSRMC;

typedef struct
{
char UTCTime[20]; //HMS
char Latitude[20]; //ddmm.mmmm
char NS[20]; //N/S
char Longitude[20]; //dddmm.mmmm
char EW[20]; //E/W
char PositionFix[20];
char SatUsed[20];
}strubf_GPSGGA;

typedef struct
{
    u8 UTCTime[3];    //hhmmss.mmm
    u8 Latitude[4];    //ddmm.mmmm
    char NS[20];        //N/S
    u8 Longitude[5];    //dddmm.mmmm
    char EW[20];        //E/W
    u8 PositionFix;    //0,1,2,6
    u8 SatUsed;    //00~12
//    u8 HDOP[4];        //0.5~99.9
 //   u8 Altitude[7];    //-9999.9~99999.9
}stru_GPSGGA;

typedef struct
{
    u8 Mode;        //A/M
    u8 Mode2;    //0,1,2,3
    u8 SatUsedList[12];    
    u8 PDOP[4];
    u8 HDOP[4];
    u8 VDOP[4];
}stru_GPSGSA;

typedef struct
{
u8 SatID;
//    u8 Elevation[2];//0-90 degree
//    u8 Azimuth[3]; //0-359 degree
u8 SNR; //0-99 dbHz
}stru_SatInfo;

typedef struct
{
    u8 SatInView;
    stru_SatInfo SatInfo[12];
}stru_GPSGSV;

#define NMEA_NULL    0x00            //GPS statement type
#define NMEA_GPGGA    0x01
#define NMEA_GPGSA    0x02
#define NMEA_GPGSV    0x04
#define NMEA_GPRMC    0x08

extern stru_GPSRMC  GPS_RMC_Data;
extern stru_GPSGGA  GPS_GGA_Data;
extern stru_GPSGSA  GPS_GSA_Data;
extern stru_GPSGSV  GPS_GSV_Data;
extern strubf_GPSRMC GPSBF_RMC_DATA;
extern strubf_GPSGGA GPSBF_GGA_DATA;
extern unsigned char  buffer0[200];


void ShowGPSTime(void);
 void ParserGPGGA(void);
 unsigned char  ParserGPRMC(void);

 extern void * Gps_Pthread (void * args);
 extern void * sendGps_Pthread (void * args);

 
void InitGps();
void CloseGps();
int GpsGetLocation(char *outdat);
int hex2ascii(unsigned char *INdata, unsigned int len, char *strout);
int AsciiToHex( unsigned char *pInBuf, int nInLen,  unsigned char *pOutBuf);
int HexToAscii(unsigned char *pInBuf, int nInLen, char *pOutBuf);

#endif


