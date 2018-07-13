#ifndef __DISPLAY__H__
#define __DISPLAY__H__

typedef unsigned short PPB;	//Pixel bits wide	

#define HZK_MAX	10			//Max supported font No.
#define HZK_CHZ	0x0a		//Chinese font
#define HZK_ENZ	0x0b		//English font
#define HZK_MIXUER	0x0c	//Chinese and English mixed font

#define USE_BACK_IMG	1	  //use background picture
#define DELET_ALL_HZK	0x55  // delete all fonts	


/*supported fonts available id,units digit represent the type of font code table£¬1 represents gb2312 ,2 represents gbk*/
/* top digit present font size*/
#define GB2312_16			161
#define GB2312_32			321
#define GB2312_24			241		//Do not support currently, do not use
#define GBK_16				162


//You do not need to know about the parameters of the font, and do not try to modify them
/*   _base suffix represents the offset base of the search code table
???? _sz suffix represents the size of the corresponding character in the font file
???? GB represents the GB2312 font, _CH represents Chinese characters, _EN represents English or digital
???? Currently only provide No.16 No.24 No.32 gb2312 font and No.16 gbk font, including No.16 and 32 gb2312 font
???? For the mixed font, 24 is divided into Chinese and English font, No.16 gbk font is Chinese font
???? Please refer to the example program for specific use
*/


#define GB16_EN_OFF_BASE	(0x45080)
#define GB24_EN_OFF_BASE	0
#define GB32_EN_OFF_BASE	(0x114200)
#define GB16_CH_OFF_BASE	0
#define GB24_CH_OFF_BASE    0
#define GB32_CH_OFF_BASE    0
#define GB16_FONT_EN_SZ		(16*16/2/8)
#define GB24_FONT_EN_SZ		48
#define GB32_FONT_EN_SZ		(32*32/2/8)
#define GB16_FONT_CH_SZ		(16*16/8)
#define GB24_FONT_CH_SZ		((24 + 7) / 8 * 24)
#define GB32_FONT_CH_SZ		((32+7)/8*32)

#define GBK16_EN_OFF_BASE	(0x45080)
#define GBK16_CH_OFF_BASE  	(0)
#define GBK16_FONT_EN_SZ   (16*16/2/8)
#define GBK16_FONT_CH_SZ   (16*16/8)


//When you need to set the color, you need to provide these color parameters to the interface

#define Color_black        	0       
#define Color_blue         	RGB565_(0,0,255)
#define Color_red     	   	RGB565_(255,0,0)
#define Color_green   		RGB565_(0,255,0)
#define Color_purple  		RGB565_(255,0,255)
#define Color_yellow  		RGB565_(255,255,0)
#define Color_white   		RGB565_(255,255,255)


//you don't need to know about the follow structures and fix them
	
//14byte
typedef struct
{
  char cfType[2];         /* The file type must be "BM" (0x4D42)*/		//	1 charater
  char cfSize[4];         /* The size of the file (bytes) */					//	2-3	
  char cfReserved[4];     /* Reserved, must be 0 */							//	4-5
  char cfoffBits[4];      /* The offset of the bitmap array relative to the file header (bytes)*/ //	6-7
}__attribute__((packed)) BITMAPFILEHEADER;       /* File header structure */      

//40byte
typedef struct
{
  char ciSize[4];         /* size of BITMAPINFOHEADER */				//8-9
  char ciWidth[4];        /* bitmap width (pixels) */				//10-11	
  char ciHeight[4];       /* Bitmap height (pixels) */				//12-13
  char ciPlanes[2];       /* Bit-plane No. of the target device must be set to 1 */	//14
  char ciBitCount[2];     /* the number of bits per pixel, 1, 4, 8 or 24 */				//15  color depth		
  char ciCompress[4];     /* bitmap array compression method, 0 = uncompressed */	//16-17
  char ciSizeImage[4];    /* image size (bytes) */							//18-19
  char ciXPelsPerMeter[4];/* Target device pixel No.per meter horizontally  */		//20-21	
  char ciYPelsPerMeter[4];/* Target device pixel No.per meter vertically */		//22-23
  char ciClrUsed[4];      /* Actual number of colors bitmap used in the color table *///24-25
  char ciClrImportant[4]; /* the number of important color index */					//26-27
}__attribute__((packed)) BITMAPINFOHEADER;       /* Bitmap header structure */

typedef struct
{
  unsigned int blue:5;
  unsigned int green:5;
  unsigned int red:5;
  unsigned int rev:1;
}__attribute__((packed)) PIXEL;		//16-bit pixels

typedef struct
{
	 unsigned char re;
	 unsigned char gr;
	 unsigned char bl;

}__attribute__((packed)) PIXEL24;//24-bit pixels


typedef struct 
{
	short width;
	short hieght;
	char bitcount;
	long int sz;
	char * data;

}BMP_INFO;

typedef struct HZK_INFO_{

			int id;					//id is the font recognition code, that is font size, font size is related to font size
			int sz;					//font size
			int higth;				//character height
			int width;				//character width
			int type;				//Chinese font or English font or mixed font
			char *content;
			
}HZK_INFO;


/*
-------------function:	Initialize fonts array,the array records the basic information of each font
-------------parameter:null
-------------return value:success return0 wrong return-1
*/
int Init_Hzk(void);


/*
------------function: Insert a new font
------------parameter:path is font file path£¬HzkId is font id£¬type is font type£¬mainly Chines English and mixed
				    3 types
------------return value:correct return0 wrong return	-1		
*/
int Insert_Hzk(char * path,int HzkId,int type);

/*
------------function: Delete the font according to the font id
------------parameter: hzkid is font id, when cmd is DELET_ALL_HZK, delete all fonts
------------return value:correct return0wrong return	-1		
*/

int Delet_Hzk(int hzkId,int cmd);


/*
------------function: open frambuffer£¬And initialize the allocation of memory buffer to display the content data
------------parameter: path is fambuffer Device node path
------------return value:correct return0wrong return	-1		
*/

int Open_Frambuffer(char * path);

/*
------------function: chose frambuffer£¬And release the memory of the buffer display data
------------parameter: null
------------return value:correct return0wrong return	-1		
*/

int Close_Frambuffer(void);


/*
------------function: Set the background color or background image, currently only supports the display bmp format images
------------parameter: ImageName is background image route£¬Backclor is background color£¬cmd is USE_BACK_IMG represent use all image
------------return value:correct return0wrong return	-1		
*/

int Set_Background(char * ImageName,PPB Backclor,int cmd);

/*
------------function: Set the font color
------------parameter: color is font color
------------return value:correct return0wrong return	-1		
*/

int Set_Font_Color(PPB color);


/*
------------function: clear screen
------------parameter: null
------------return value:correct return0wrong return	-1		
*/
int Clear_Display(void);


/*
------------function: Show a string of text
------------parameter: x1 y1 display the position upper left corner coordinates£¬PzText is text content£¬font is font id,
				    
------------return value:correct return0wrong return	-1		
*/
int TextOut(int x1,int y1,char * pzText,int font);

/*
------------function:In the specified location to open the display bmp picture, the size of the picture itself size
------------parameter:is the upper left corner of the display position, and bmpfile is the image path
------------return value:correct return0wrong return	-1		
*/
int BmpOut(int x,int y,char * bmpfile);



/*
------------function:Draw a rectangle
------------parameter:x1 y1 is the coordinates of the upper left corner of the display position, x2 y2 is the lower right corner coordinates, colidx is the fill color
------------return value:null	
*/

void fillrectcoli(int x1,int y1,int x2,int y2,unsigned short colidx);


/*
------------function:Fill the rectangle with the background image or background color
------------parameter:x1 y1 is the coordinates of the upper left corner of the display position, x2 y2 is the lower right corner coordinates
------------return value:null	
*/
void fillrectusebackground(int x1,int y1,int x2,int y2);


/*
------------function:Draw a rectangle
------------parameter:x1 y1 is the coordinates of the upper left corner of the display position, x2 y2 is the lower right corner coordinates, colidx is the fill color
------------return value:correct return0wrong return	-1		
*/
void Update_Frambuffer(int x1,int y1,int x2,int y2);


/*
------------function:Constructs a color value
------------parameter:r red component, g green component, b blue component
------------return value: return color value
*/
PPB RGB565_(unsigned char r,unsigned char g,unsigned char b);

/*
------------function:open read bmp image data and save it to specified buff
------------parameter:bmpfile is image route£¬buff is data buffer
------------return value:success return0wrong return-1
*/
int Load_Bmp_Img(char * bmpfile,BMP_INFO * buff);


/*
------------function:display the buffered bmp image data to specified position
------------parameter:x1,y1 image top left conner£¬bmp is buffered data
------------return value:success return0wrong return-1
*/
int Show_Bmp(int x1,int y1,BMP_INFO * bmp);

#endif
