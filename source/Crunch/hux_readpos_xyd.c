/************************************************************************
This function (read_pos_xyd) reads ascii position files with an .xyd extention
- .xyd file format...
	(long int)time	(int)xpos (int)ypos (int)headdirection
	- time is the sample number in the accompanying binary electrophysiology record at which a video sync signal was detected
	- xpos is the x camera pixel value (integer)
	- ypos is the y camera pixel value (integer)
	- headdirection is the orientation of the subject's head based on use of a 2- or 3-LED array (integer)
- position sampling frequency = 25Hz, though the "time" variable itself has much higher resolution
- if either x or y value is invalid (<0), then both are set to -1
- float is used for storage to facilitate data transformations
- assumptions...
	- memory has been allocated for time,x,y,head-direction,led-distance and result
- NOTE:
	- the value of posfreq returned by this function may be inaccurate if video sync signals were lost in the original recording
*************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

void hux_error(char message[]);
int hux_substring(char *source, char *target, int casesensitive);

int hux_readpos_xyd (
		char *infile, /* filename to read*/
		double *pos_time, /* storage for timestamps (seconds)*/
		float *pos_x, /* storage for position (cm) */
		float *pos_y, /* storage for position (cm) */
		float *pos_headdir, /* storage for head direction (degrees) */
		float *led_dist, /* storage for distance between red & blue LEDs (cm) */
		float vidres, /* pixels/cm resolution  - defined in calling function*/
		int vidxmax, /* maximum possible x-coordinate (typically 768) - passed back to calling function */
		int vidymax, /* maximum possible y-coordinate (typically 576) - passed back to calling function */
		int ephysfreq, /* sampling freq. of electrophysiology recording, from which .xyd file derives timestamps (sample numbers) */
		float *result /* 0=postot,1=vidxmax,2=vidymax,3=samplefreq*/
		) 
{
	char line[1000],command[256];
	int i,tempx,tempy,tempdir,postot=-1,posvalid=0,badrangetot=0;
	long int temptime;
	double timebase; 
	FILE *fpin;

	timebase=(double) ephysfreq;

	fpin=fopen(infile,"r");
	if(fpin==NULL) {sprintf(command,"hux_readpos_csicsvari_asc: can't open \"%s\"\n\n",infile);	hux_error(command);}
	
	/* read .xyd file */
	while(fgets(line,1000,fpin)!=NULL) {
		if(line[0]!='#' && sscanf(line,"%ld %d %d %d",&temptime,&tempx,&tempy,&tempdir)==4) {
			postot++; // note that postot starts from -1
			pos_time[postot] = temptime/timebase;
			led_dist[postot] = -1.0; /* in this implimentation, no LED distance */
			if(tempx<0||tempy<0) {pos_x[postot]=pos_y[postot]=pos_headdir[postot]=-1;continue;}
			else if(tempx>vidxmax||tempy>vidymax) {badrangetot++;pos_x[postot]=pos_y[postot]=pos_headdir[postot]=-1;continue;}
			else {
				posvalid++;
				pos_x[postot] = tempx/vidres;		// convert to cm
				pos_y[postot] = (vidymax-tempy)/vidres; // also invert y-coordinates using max camera y-pixels
				pos_headdir[postot] = tempdir;

	}}}
	fclose(fpin);

	result[0]= (float) postot+1;
	result[5]= (float) badrangetot;
	return(posvalid);
}
