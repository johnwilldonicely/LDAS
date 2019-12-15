/************************************************************************
- read_eeg_csicsvari reads EEG records (.eeg files)
- typcal sample frequency is 1250 Hz (20KHz/16)
	- so, sample 0 is sample 0 from the .dat file, sample 1 is sample 16, 2 is 32, and so on
- first call hux_checkcsicsvari and allocate required memory
- eegscale can be used to invert EEG, if a value of -1 is chosen 
- NOTE!!: assumes memory has been allocated by calling function
- NOTE!!: when selecting a channel (channelchoice) remember values are zero-offset
	- hence if channeltot=64, channels range from 0-63
*************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#define BLOCKSIZE 1000
#define CHANNELMAX 256

void hux_error(char message[]);
int hux_readeeg_csicsvari (
   char *infile,	/* input file name */
   double *eeg_time,	/* storage for timestamps */
   float *eeg_val,	/* storage for EEG A/D values */
   float eegscale,	/* for scaling or optionally inverting EEG */
   int channelchoice,	/* selected input EEG channel (0-255) */
   int channeltot,	/* number of EEG channels in each record - (default=64, must include sync channels)*/
   float samplefreq,	/* samples per second for eeg */
   float *result)
{
char temp_str[256];
int i,size_of_short,bytesread,bytestarget,blocksread,eegtot=-1;
short int tempbuf[256*BLOCKSIZE];
long headerlength=0; /* size of ascii header on binary files */
float eegfreq=-1.0;
double time=0.0,timeprev=0.0,interval;
FILE *fpin;

size_of_short=sizeof(short int);
interval= 1.0/samplefreq;
time=-interval;

if(channeltot<1||channeltot>CHANNELMAX) {
	sprintf(temp_str,"hux_readeeg_csicsvari: channel total (%d) outside permissable range (1-%d)\0",channeltot,CHANNELMAX);
	hux_error(temp_str);
}
if(channelchoice<0||channelchoice>=channeltot) {
	sprintf(temp_str,"hux_readeeg_csicsvari: selected channel (%d) is outside acceptable range (0-%d)\0",channelchoice,channeltot-1); 
	hux_error(temp_str);
}

fpin=fopen(infile,"rb");
if(fpin==NULL) {sprintf(temp_str,"hux_readeeg_csicsvari: can't open \"%s\"\n\n",infile);hux_error(temp_str);}

/* read data (note that time starts at -interval and eegtot starts at -1 */	
fseek(fpin,headerlength,SEEK_SET);
bytestarget=channeltot*BLOCKSIZE;
while((bytesread=fread(tempbuf,size_of_short,bytestarget,fpin))==bytestarget){
	for(i=0;i<BLOCKSIZE;i++) {
		eegtot++;
		time+=interval; /* assume samples are taken at precise regular intervals */
		eeg_time[eegtot] = time;
		eeg_val[eegtot] = (float) (eegscale*tempbuf[i*channeltot+channelchoice]);
	}
}
/* Now output remaining records (<BLOCKSIZE) which were read in the final (unassigned) iteration in the above loop */
blocksread = (int)(1.0*bytesread/channeltot);
for(i=0;i<blocksread;i++) {
	eegtot++;
	time+=interval; /* assume samples are taken at precise regular intervals */
	eeg_time[eegtot] = time;
	eeg_val[eegtot] = (float) (eegscale*tempbuf[i*channeltot+channelchoice]);
}

fclose(fpin);
result[0]=(float) eegtot; /* number of EEG records (each record includes n=channeltot samples plus one 2-byte tracking record (eg. for 32 channels, 66 bytes) */
result[1]=(float) 1.00; /* number of EEG samples per record for a given channel */
result[2]=(float) samplefreq; /* sampling frequency... interval (seconds) between records */
return (eegtot); /* eegtot */
}
