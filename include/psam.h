#ifndef __PSAM__H__
#define __PSAM__H__


#define   RESPONSE			0x61
#define	  POWER_DOWN		0x09
#define	  WARM_RESET		0x10  
#define	  COLD_RESET		0x11  //Cold reset command
#define   RECEIVE_LEN		0x12  //Read the command to receive the data length
#define   BAUD_CMD			0x13  //Change the baud rate command
#define   PPS_CMD			0x14  //PPS
#define   SEL_CARD			0x15
#define	  CHECK_PRESENCE	0x16

#define   BAUD_2400       0x30
#define   BAUD_3200       0x24
#define   BAUD_4800       0x18
#define   BAUD_6400       0x12
#define   BAUD_9600       0x0C
#define   BAUD_12800      0x09
#define   BAUD_19200      0x06
#define   BAUD_28800      0x04
#define   BAUD_38400      0x03
#define   BAUD_57600      0x02
#define   BAUD_115200     0x01


/********psam cmd*************

----------- get psamID:0x00,0xa4,0x00,0x00,0x02,0x80,0x11
----------- select psamAPP:0x00,0xb0,0x96,0x00,0x06
----------- select psamKEYINDEX:0x00,0xb0,0x97,0x00,0x01
		    ect.
*/



/*
------------function:  Initialize the psam card device
------------parameter: Path is device node, baurate is baud rate, select is card slot number
------------return value: :success return0 wrong return-1
*/
int Init_Psam(char * path,int baurate,int select);

/*
------------function:  close psam card device
------------parameter:  null
------------return value: :success return0 wrong return-1
*/
int ClosePsam(void);

/*
------------function:  get psam card No.
------------parameter:  cmd send command to psam card£¬cmdlen is command length£¬out is return data£¬lenth is data length
------------return value: :success return0 wrong return-1
*/
int GetPsamID(char * cmd,int cmdlen,unsigned char * out,int * lenth);

/*
------------function:  select psam card application
------------parameter:  cmd send command to psam card£¬is command length
------------return value: :success return0 wrong return-1
*/
int Select_Psam_App(char * selectapp,int cmdlen);


/*
------------function:  Select the psam card key index
------------parameter:  selectindex send command to psam card£¬cmdlen is command length£¬out is key index
------------return value: :success return0 wrong return-1
*/
int Select_Psam_KeyIndex(char * selectindex,int cmdlen,unsigned char * out);



/*
------------function:  psam card command sending
------------parameter:  indata send command to psam card£¬outdata is return data£¬len is return data length
------------return value: :success return0 wrong return-1
*/
int PsamCos(char *Intdata, char *Outdata,int *len);

#endif

