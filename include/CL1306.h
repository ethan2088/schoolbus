#ifndef __CL1306__H__
#define __CL1306__H__


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
#include <pthread.h>
#include <semaphore.h>
#include <popt.h>
#include <ctype.h>
#include <sys/socket.h>
#include <stdint.h>
#include <assert.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <asm/types.h>          /* for videodev2.h */
#include <linux/videodev2.h>
#include <linux/input.h>
#include <linux/fb.h>


#define CARDLAN_DEBUG 1

#if CARDLAN_DEBUG
	#define	DBG_PRINTF		printf
	#define DBG_AUDIO_PRINTF printf
#else
	#define	DBG_PRINTF(...)
	#define DBG_AUDIO_PRINTF(...)
#endif

#define  Chinese    0

#if Chinese
	#define	CH		1	
#else
	#define	CH      0	
#endif

#endif
