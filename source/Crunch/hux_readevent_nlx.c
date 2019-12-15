/************************************************************************
- read Neuralynx event records
- NOTE!!: assumes sufficient memory has been allocated by calling function
*************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include "neuralynx.h"

int hux_readevent_nlx (
					   char *infile, 
					   double *event_time, /* array for event timestamps */
					   char *event_label, /* array for event labels (max 128 characters each */
					   float *result  /* array to hold results */
					   )
{
struct EventRec	Event; /* define size of event record structure */
int i,eventtot=-1,EventSize=sizeof(struct EventRec), timedivisor=1000000;
long headerlength=16384; /* size of ascii header on binary Neuralynx files */
double time0=0.0,timeprev=0.0,interval=0.0;
FILE *fpin;

fpin=fopen(infile,"rb");
if(fpin==NULL) {fprintf(stderr,"\n** ERROR in hux_readevent_nlx: can't open \"%s\"\n\n",infile);return(-1);}

fseek(fpin,headerlength,SEEK_SET);
while (fread(&Event,EventSize,1,fpin)!=0) {
	eventtot++;
	event_time[eventtot] = (double) Event.qwTimeStamp/timedivisor; 
	for(i=0;i<128;i++) event_label[eventtot*128+i] = Event.EventString[i];
}

eventtot++;
fclose(fpin);
result[0]=(float) eventtot; /* number of event records */
return (eventtot);
}
