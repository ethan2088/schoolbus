#ifndef __READ_H_
#define __READ_H_

//KEYA KEYB
#define	KEYA				0x0A					//card KEYA
#define	KEYB				0x0B					//card KEYB

#define 	MI_OK		        0x00     			//Read write cards succeed return 0 £»
#define 	MI_FAIL				0x01				//Read write cards failed return 1£»

#define     WRITE_KEY			1					//Write key
#define 	VERIFY_KEY			2					//Verify key
#define 	READ_ONLY			3					//no need to write key and verification key
#define 	WRITE_ONLY			3					//no need to write key and verification key

//Sector buffer
struct card_buf
{
        unsigned char key[6];
        unsigned char mode;
        unsigned char rwbuf[16];
        unsigned char money[4];
};


/*
*******************************************
-Structure Name: Key Management
-Included elements:
        unsigned char key[6];       Key value
        unsigned char mode;         Key mode
        unsigned char rwbuf[16];    read write value     
        unsigned char money[4];     read write value     
*******************************************        
*/        
//extern struct card_buf KeyInfo;//Sector buffer, card key, card mode etc.


/*
*************************************************************************************************************
- function name : int Initz_MF(void)
- function declaration : Open the reader writer device
- input parameter : path is read head device node
- output parameter : 0 -- succeed-1--failed
		   -1 -- turn on read head power supply failed
		   -2 -- turn on read head device failed
*************************************************************************************************************
*/

int Init_MF(char *path);

/*
*************************************************************************************************************
- function name :unsigned char CardReset(char *data,unsigned char *plen)
- function declaration :Reset card (find card, anti-collision, select card)
- input parameter :data:Card serial number£¬plen:serial number length
- output parameter :0x08 is M1 card£¬0x20 is CPU card,
*************************************************************************************************************
*/
unsigned char CardReset(char *data,unsigned char *plen);

/*
**************************************************************************************************************************
- function name : unsigned char ReadOneSectorDataFromCard(unsigned char *SectorBuf, unsigned char SectorNo, unsigned char BlockNo
             unsigned char *key,unsigned char mode)
- function declaration : From the M1 card read the data of a certain block of a sector
- input parameter :    
				SectorBuf:  block data to be read out	
				BufLen:     Buffer length  
				SectorNo:   the sector read out  
				BlockNo: 	read the certain block No. of the sector
				VerifyFlag: verification mark£¬range:1-3,input 1 and 2£¬need to input KEY value(6bytes) and mode value
				KEY:        input key
				mode:       Key authentication method£¬0x0a or 0x0b
- output parameter : 0:is execution succeed£¬ not 0 is execution failed 
**************************************************************************************************************************
*/
int ReadOneSectorDataFromCard(unsigned char *SectorBuf, unsigned char *BufLen ,
			  unsigned char SectorNo, unsigned char BlockNo, unsigned char VerifyFlag,unsigned char *key,unsigned char mode);


/*
*************************************************************************************************************
- function name : unsigned char TypeAPiccSendAPDU(uint8_t  *Send,uint8_t *Rcvdata,uint8_t Slen, uint8_t *Rlen)
- function declaration : CPU card instructions send and receive functions
- input parameter :    *Send: 	command to be sent
				*Rcvdata :	received return code
				Slen:	sent command length
				Rlen:	received return length
- output parameter :   0:is execution succeed£¬ not 0 is execution failed  
*************************************************************************************************************
*/
#if 1
extern unsigned char TypeAPiccSendAPDU(unsigned char  *Send,unsigned char  *Rcvdata,unsigned char  Slen, unsigned int  *Rlen);
#endif
/*
*************************************************************************************************************
- function name : unsigned char WriteOneSertorDataToCard(unsigned char *SectorBuf, unsigned char SectorNo, unsigned char BlockNo, unsigned char VerifyFlag)
- function declaration : write data into a certain block of a sector of M1 card
- input parameter :    
				SectorBuf: date to be written	
			    	SectorNo:  write into to that sector  
			    	BlockNo:   write into a certain No. of block of the sector
			    	VerifyFlag: verification mark£¬range:1-3,input 1 and 2£¬need to input KEY value(6bytes) and mode value
				KEY:       input key
				mode:       Key authentication method£¬0x0a or 0x0b
- output parameter : 0:is execution succeed£¬ not 0 is execution failed 
*************************************************************************************************************
*/
int WriteOneSertorDataToCard(unsigned char *SectorBuf, int len,unsigned char SectorNo, unsigned char BlockNo, unsigned char VerifyFlag,unsigned char *key,unsigned char mode);


/*
*************************************************************************************************************
- function name :unsigned char CardINCorDEC(unsigned char SectorNo, unsigned char BlockNo, 
    unsigned char keymode,unsigned char *value,unsigned char *key,unsigned char type);
- function declaration :Operate to a certain block of a certain sector to decrement or increment value 
- Input Parameter :
			SectorNo:  write into to that sector  
			BlockNo:   write into a certain No. of block of the sector			
		    key:       input key(6 bytes)
		    value:     Remain to operate value   4 bytes£¬Little-Endian£¬Low end at front(eg:100£¬0xe8 0x03 0x00 0x00)
			keymode:      Key authentication method£¬0x0a or 0x0b
			type:      1:INC operation  0:DEC operation
- Output Parameter :None
Remark:To Operate decrement and increment value£¬data block format must be:  x1 x2 x3 x4 y1 y2 y3 y4 x1 x2 x3 x4 01 fe 01 fe,and y1 y2 y3 y4 is the data reverse of x1 x2 x3 x4 

This function have edit into the danamic library,But it is not totally verified, just remove the comment if needed
*************************************************************************************************************
*/
unsigned char CardINCorDEC(unsigned char SectorNo, unsigned char BlockNo, 
    unsigned char keymode,unsigned char *value,unsigned char *key,unsigned char type);
/*
*************************************************************************************************************
- function name :OperateBlock(KEYA,KEY_buff.key,(4*LanSec.For+3));
- function declaration :operate to a which block
- input parameter :
- output parameter :null
*************************************************************************************************************
*/
#if 1
unsigned char OperateBlock(unsigned char SectorNo,struct card_buf* KeyInfor);
#endif



/*
*************************************************************************************************************
- function name :int Cpu_Send_Cmd(char * cmd,int CmdLen,char * recive,int * ReciveLen);
- function declaration : send command to cpu card£¬and retune related data
- input parameter :    
				cmd: command to be sent
			    	CmdLen: command length
			    	BlockNo:   write into a certain No. of block of the sector
			    	recive: returned data
				ReciveLen: returned data length
- output parameter : 0:is execution succeed£¬ not 0 is execution failed 
*************************************************************************************************************
*/

int Cpu_Send_Cmd(char * cmd,int CmdLen,char * recive,int * ReciveLen);





/*
*************************************************************************************************************
- function name :void RFCloseField(void)
- function declaration :Off the frequency field£¬off the read head power supply
- input parameter :null
- output parameter :null
*************************************************************************************************************
*/
void RFCloseField(void);

/*
*************************************************************************************************************
- function name :void RFOpenField(void)
- function declaration :turn on the frequency field£¬turn on the read head power supply
- input parameter :null
- output parameter :null
*************************************************************************************************************
*/
extern void RFOpenField(void);

/*
*************************************************************************************************************
- function name :void CloseRF(void)
- function declaration :turn off card read write device
- input parameter :null
- output parameter :null
*************************************************************************************************************
*/
extern void CloseRF(void);
#endif






