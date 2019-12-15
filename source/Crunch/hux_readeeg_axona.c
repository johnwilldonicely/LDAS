/************************************************************************
- reads Axona format EEG records (Jim Donnet's "DACQ" data format
- returns EEG sampling frequency as calculated from first 10 samples
- pass pointers to input filename, and float data
- first call hux_checkaxona and allocate required memory
- float is used for storage to facilitate data transformations
- eegscale can be used to invert EEG, if a value of -1 is chosen 
- NOTE!!: assumes memory has been allocated by calling function
*************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#define HEADERMAX 1000

void hux_error(char message[]);
int hux_getfileheader(FILE *fpin, char *header,	char *header_stop, int headermax);
int hux_getword(char *line, char *trigger, int word, char *target);
int hux_substring(char *source, char *target, int casesensitive);


int hux_readeeg_axona (
					   char *infile, 
					   double *eeg_time, 
					   float *eeg_val, 
					   float eegscale,
					   float *result)
{
char line[1000],header[HEADERMAX],header_stop[64],temp_str[256];
int i,rectot=0,tempcount=-1,num_chans=0,bytes_per_sample=0,headerlength=-1;
float sample_rate=-1.0;
char tempeeg;
double time=0.0,time0=0.0,timeprev=0.0,interval=0.0;
FILE *fpin;

sprintf(header_stop,"data_start\0");
fpin=fopen(infile,"rb");

/* read header to string array "line" and look for keywords */
if(fpin==NULL) {fprintf(stderr,"\n** ERROR in hux_readeeg_nlx: can't open \"%s\"\n\n",infile);return(-1);}
headerlength=hux_getfileheader(fpin,header,header_stop,HEADERMAX)*sizeof(char);
if(headerlength==-1) {sprintf(temp_str,"hux_checkaxona: no file header ending in %s",header_stop); hux_error(temp_str);}
hux_getword(header,"num_EEG_samples",1,line); rectot=atoi(line);
hux_getword(header,"sample_rate",1,line);sample_rate=atof(line);
hux_getword(header,"num_chans",1,line); num_chans=atoi(line); /* typically 1 for this data format */
hux_getword(header,"bytes_per_sample",1,line); bytes_per_sample=atoi(line);
interval=1.0/sample_rate;

//printf("HEADERLENGTH %d\n",headerlength);
//printf("NUM_CHANS %d\n",num_chans);
//printf("BYTES_PER_SAMPLE %d\n",bytes_per_sample);

/* now go back and read data */	
fseek(fpin,headerlength,SEEK_SET);
for(i=0;i<rectot;i++) {
    fread(&tempeeg,bytes_per_sample,num_chans,fpin); /* this should never fail if hux_axona has already been called to verify file length */
	time+=interval; /* assume samples are taken at precise regular intervals */
	eeg_time[i] = time;
	eeg_val[i] = (float) (tempeeg*eegscale);
} 
fclose(fpin);

result[0]=(float) rectot; /* number of EEG records */
result[1]=(float) num_chans; /* number of EEG samples per record */
result[2]=(float) sample_rate; /* EEG record sampling rate - accounts for number of valid samples per record */
return (rectot); /* tempcount = recordtot*validsamples */
}
