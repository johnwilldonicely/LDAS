/************************************************************************
Read_pos_dacq reads DACQ position records
- if either x or y value is invalid, the other is set to invalid too
- returns position sampling frequency
- pass pointers to input filename, and float data
- first call hux_checkdacq and allocate required memory
- float is used for storage to facilitate data transformations
- 
- NOTE!!: assumes memory has been allocated by calling function
- NOTE!!: assumes single spot tracking at this time (June 29, 1004)#

To Do	- ask Jim about window min/max values - currently dont seem to be used?
		- ask about jumpy points?
		- count good samples?


*************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define HEADERMAX 1000


void hux_error(char message[]);
int hux_getfileheader(FILE *fpin, char *header,	char *header_stop, int headermax);
int hux_getword(char *line, char *trigger, int word, char *target);
typedef union {char ch[2];short i;} swap_i16; /* create special structure for bit-swap function */
short i16_swap_(short input){short output;swap_i16 inp,out;inp.i=input;out.ch[1]=inp.ch[0];out.ch[0]=inp.ch[1];return out.i;} /*swap function */

int hux_readpos_axona (
   char *posfile,		/* pointer to input file name */
   double *pos_time, 	/* array to hold time values */
   float *pos_x, 		/* array to hold x-values */
   float *pos_y, 		/* array to hold y-values*/
   float vidres, 		/* video resolution (pixels/cm) */
   float *result		/* aray to hold output */
   )	
{
char line[1000],temp_str[256],header[HEADERMAX],header_stop[1000];
int i,x,y,rectot=-1,badrangetot=0,posvalid=0,posinvalid=-1,vidxmax=640,vidymax=480,headerlength=0,winxmin=0,winymin=0,winxmax=999999,winymax=999999;
int recsize=0,bytes_per_sample=0, bytes_per_timestamp=0, num_colours=0, bytes_per_coord=0, ppm=0;
float posinterval=-1.0, sample_rate=-1.0, duration=-1.0, ividres=1.0/vidres;
double time=0.0,time0=0.0,timeprev=0.0,interval=0.0;
FILE *fpin;
short i16_swap_(short input);

fpin=fopen(posfile,"r+b");
if(fpin==NULL) {sprintf(temp_str,"hux_readpos_axona: can't open position file %s\n",posfile); hux_error(temp_str); }
sprintf(header_stop,"data_start\0");
headerlength=hux_getfileheader(fpin,header,header_stop,HEADERMAX)*sizeof(char);
if(headerlength==-1) {sprintf(temp_str,"hux_readposaxona: no file header ending in %s",header_stop); hux_error(temp_str);}

hux_getword(header,"duration",1,line); duration=atof(line);
hux_getword(header,"num_pos_samples",1,line); rectot=atoi(line); /* this will have been confirmed using hux_checkaxona */
hux_getword(header,"bytes_per_timestamp",1,line); bytes_per_timestamp=atoi(line);
hux_getword(header,"sample_rate",1,line);sample_rate=atof(line);
hux_getword(header,"num_colours",1,line); num_colours=atoi(line);
hux_getword(header,"bytes_per_coord",1,line); bytes_per_sample=atoi(line);
hux_getword(header,"bytes_per_timestamp",1,line); bytes_per_timestamp=atoi(line);
hux_getword(header,"pixels_per_metre",1,line); ppm=atoi(line); /* not used - may be unreliable */
hux_getword(header,"max_x",1,line); vidxmax = atoi(line);
hux_getword(header,"max_y",1,line); vidymax = atoi(line);
hux_getword(header,"window_min_x",1,line); winxmin = atoi(line);
hux_getword(header,"window_min_y",1,line); winymin = atoi(line);
hux_getword(header,"window_max_x",1,line); winxmax = atoi(line);
hux_getword(header,"window_max_y",1,line); winymax = atoi(line);
recsize = bytes_per_timestamp + num_colours*2*bytes_per_sample;
interval=1.0/sample_rate;
/***************************************************************************************************/
struct AxonaVidProto	{ // this should correspond with recsize! 
		unsigned long ptime; short x1; short y1; short x2; short y2; short x3; short y3; short x4; short y4;
	} AxonaVid;

if(recsize!=sizeof(AxonaVid)) hux_error("hux_readpos_axona: position input structure size does not match header info");

/* Store video data: invert y-coordinates and scale to cm */
posvalid=0; time=0.00;
fseek(fpin,headerlength,SEEK_SET);
for(i=0;i<rectot;i++) {
	pos_time[i] = time;
	fread(&AxonaVid,recsize,1,fpin);
	x=i16_swap_(AxonaVid.x1); /* swap bytes */
	y=(vidymax - i16_swap_(AxonaVid.y1)); /* swap bytes & invert */
	if(x==1023||x<winxmin||x>winxmax||x<winymin||y>winymax) { /* if x is invalid or x|y fall outside the user-defined bounding box... */
	badrangetot++;
	pos_x[i] = pos_y[i] = (float)posinvalid;
	} 
	else {posvalid++; pos_x[i] = ividres * (float)x; pos_y[i] = ividres * (float)y;}
	time+=interval; /* assume samples are taken at precise regular intervals */
}
fclose(fpin);

/***************************************************************************************************/
result[0]= (float) rectot;
result[1]= (float) vidxmax;
result[2]= (float) vidymax;
result[3]= (float) sample_rate;
result[4]= (float) posinterval;
result[5]= (float) badrangetot;
return(posvalid);
}
