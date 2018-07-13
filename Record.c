
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
#include "Record.h"

#define MAX_REC	10
UPLOAD_DATA rec[MAX_REC];

int RecordInit(void)
{
	memset(&rec, 0, sizeof(UPLOAD_DATA)*MAX_REC);
	return 0;
}
int RecordPush(UPLOAD_DATA *prec)
{
	int i;

	// replace same
	for(i=0;i<MAX_REC;i++)
	{
		if(rec[i].tagid == prec->tagid)
		{
			memcpy(&rec[i], prec, sizeof(UPLOAD_DATA));
			return 0;
		}
	}

	// replace empty
	if(i==MAX_REC)
	{
		for(i=0;i<MAX_REC;i++)
		{
			if(rec[i].tagid == 0)
			{
				memcpy(&rec[i], prec, sizeof(UPLOAD_DATA));
				return 0;
			}
		}
		// memory move and replace last
		if(i==MAX_REC)
		{
			for(i=0;i<(MAX_REC-1);i++)
			{
				memcpy(&rec[i], &rec[i+1], sizeof(UPLOAD_DATA));
			}
			memcpy(&rec[i], prec, sizeof(UPLOAD_DATA));
		}
	}

	return 0;
}

int RecordPop(UPLOAD_DATA *prec)
{
	int i;
	for(i=0;i<MAX_REC;i++)
	{
		if(rec[i].tagid != 0)
		{
			memcpy(prec, &rec[i], sizeof(UPLOAD_DATA));
			return 0;
		}
	}
	return -1;
}

int RecordDel(uint32_t tagid)
{
	int i;
	for(i=0;i<MAX_REC;i++)
	{
		if(rec[i].tagid == tagid)
		{
			memset(&rec[i], 0, sizeof(UPLOAD_DATA));
			return 0;
		}
	}
	return -1;
}
int RecordGet(void)
{
	int i,ret = 0;
	for(i=0;i<MAX_REC;i++)
	{
		if(rec[i].tagid)
			ret++;
	}
	return ret;
}

