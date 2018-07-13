#ifndef _GPIO_H_
#define _GPIO_H_
 

/*****************************gpio*************************/

#define GPIO_PATH1	"/dev/fullgpio"
#define GPIO_PATH2	"/dev/mcugpio"


/* GPIO group definition */
#define GPIO_OUTPUT		0
#define GPIO_VALUE		1
#define GPIO_GROUP_A 0
#define GPIO_GROUP_B 1
#define GPIO_GROUP_C 2
#define GPIO_GROUP_D 3
#define GPIO_GROUP_E 4


#define SPI_SERVER_GPIO_LEDRED		0x01
#define SPI_SERVER_GPIO_LEDGREEN	0x02


/*
------------function: Check if gpio is already on
------------parameter: null
------------return value:On returns 1, otherwise returns 0
*/
int Is_Gpio_Open(void);

/*
------------function: gprs reset
------------parameter: null
------------return value:correct return0 wrong return	-1		
*/
int Gprs_Rest(void);


/*
------------function: initialize gpio
------------parameter:path Device node path
------------return value:correct return0 wrong return	-1		
*/
int  Init_Gpio(char *path);


/*
------------function: Set gpio in input mode
------------parameter: group is gpio group£¬num is io interface under this group£¬state is io state value
------------return value: null	
*/
void w55fa93_io(int group,int num,int state);

/*
------------function: Set gpio in output mode
------------parameter: group is gpio group£¬num is io interface under this group£¬state is io state value
------------return value: null	
*/
void w55fa93_setio(int group,int num,int state);


/*
------------function: buzzer is off
------------parameter: null
------------return value: null	
*/
void buzz_off();


/*
------------function: buzzer is on
------------parameter: null
------------return value: null	
*/
void buzz_on();


/*
------------function: Initialize LED
------------parameter: null
------------return value:success  return0 wrong return-1	
*/
int Init_Led(char * path);


/*
------------function: LED right light is on
------------parameter: null
------------return value:success  return0 wrong return-1	
*/
int LED_Right(unsigned char ON);

/*
------------function:  LED left light is on
------------parameter: null
------------return value: :success  return0 wrong return-1
*/
int LED_Left(unsigned char ON);









#if 0

/*
*************************************************************************************************************
- Function name : void SystemPowerOff()
- function declaration : Shutdown command
- input parameter :      
- output parameter :     
*************************************************************************************************************
*/
extern void SystemPowerOff();

/*
*************************************************************************************************************
- Function name : 	int ControlLed()
- function declaration : 	Control 4 LED small bulb switch
- input parameter : 	pick -- represent the bulb No. value range:1-4;
			on_off -- represent on or off;0£¬bulb off£»1£¬bulb on£»other£¬bulk on£»			
- output parameter :	0 -- normal
			-1 -- Operation device failed
			-2 -- bulb do not exist
*************************************************************************************************************
*/
extern int ControlLed(int pick, unsigned char on_off);

/*
*************************************************************************************************************
- Function name : 	int TwinkleLed()
- function declaration : 	LED bulk fliker
- input parameter : 	pick -- represent the bulb No. value range:1-4;		
- output parameter :	0 -- normal
			-1 -- Operation device failed
			-2 -- bulb do not exist
*************************************************************************************************************
*/
extern int TwinkleLed(int pick);

/*
*************************************************************************************************************
- Function name : 	int SetBackLight()
- function declaration : 	Set the backlight brightness
- input parameter : 	lcd_level -- represent brightness level;value range:1-150;	
- output parameter :	larger than 0 -- brightness level
			-1 -- Operation device failed
			-2 -- Beyond the brightness range
*************************************************************************************************************
*/
extern int SetBackLight(int lcd_level);

/*
*************************************************************************************************************
- Function name : 	int ControlBeep()
- function declaration : 	Switch buzzer
- input parameter : 	on_off -- 0£¬buzzer off£»1£¬buzzer on£»other£¬buzzer on£»	
- output parameter :	0 -- operation normal
			-1 -- Operation device failed
*************************************************************************************************************
*/
extern int ControlBeep(unsigned char on_off);
#endif

#endif

