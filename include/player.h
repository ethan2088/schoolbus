#ifndef  __PLAYER__H__
#define  __PLAYER__H__

#define CONFIG_SOUND_THREAD		0				//Whether the voice is configured in thread mode

#define AUDIO_NBFRAGS_DEFAULT 8
#define AUDIO_FRAGSIZE_DEFAULT 8192 			//this for write
#define BUF_SIZE 		(AUDIO_FRAGSIZE_DEFAULT * AUDIO_NBFRAGS_DEFAULT)


#define SND_DSP 			"/dev/dsp"
#define SND_MIXER			"/dev/mixer"

#define SND_STEREO		2
#define SND_MONO		1
#define SPEED_8K		8000
#define SPEED_11K		11025
#define SPEED_22K		22050
#define SPEED_44K		44100

#define DirLenth 64			//The maximum length of the voice directory name



typedef  struct
{
	char *buffer;
	unsigned int size;
} VPInfo;


typedef struct tagWaveFormat
{ 
	char cRiffFlag[4]; 
	unsigned int nFileLen; 
	char cWaveFlag[4]; 
	char cFmtFlag[4]; 
	char cTransition[4]; 
	unsigned short nFormatTag ; 
	unsigned short nChannels; 
	unsigned int nSamplesPerSec; 
	unsigned int nAvgBytesperSec; 
	unsigned short nBlockAlign; 
	unsigned short nBitNumPerSample; 
	char cDataFlag[4]; 
	unsigned int nAudioLength; 
} WAVEFORMAT;

typedef struct tagWaveFormat2
{ 
	char cRiffFlag[4]; 
	unsigned int nFileLen; 
	char cWaveFlag[4]; 
	char cFmtFlag[4]; 
	char cTransition[4]; 
	unsigned short nFormatTag ; 
	unsigned short nChannels; 
	unsigned int nSamplesPerSec; 
	unsigned int nAvgBytesperSec; 
	unsigned short nBlockAlign; 
	unsigned short nBitNumPerSample; 
	char cReserverd[2]; 
	char cDataFlag[4]; 
	unsigned int nAudioLength; 
} WAVEFORMAT2;


/*
------------function:  Initialize the voice device, set the voice file directory
------------parameter: 	path is voice location,mixerpath and dsppath is voice device nodes
------------return value: :success return0 wrong return-1
*/
int Init_Player(char * Path,char * mixerpaht,char * dsppath);


/*
------------function:  Turn off the voice device
------------parameter: 	null
------------return value: :success return0 wrong return-1
*/
int Close_Player_Device(void);

/*
------------function:  Play a voice file
------------parameter: 	path is voice name
------------return value: :success return0 wrong return-1
*/
int Play_Music(char * path);

/*
------------function:  Stop playing voice files
------------parameter: 	null
------------return value: :success return0 wrong return-1
*/
int Stop_Music(void);



#endif
