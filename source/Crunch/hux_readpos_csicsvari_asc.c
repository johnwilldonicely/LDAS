/************************************************************************
read_pos_csicsvari_asc reads Csicsvari lab position records (.whl files)
- .whl/.whd file format...
	- ASCII
	- whl: 2 float values per line (x y)
	- whd: 3 float values per line (x y head-direction)
	- -1, -2 = invalid position
	- in pre-prosecssing, original camera resolution in pixels (768 x 576) is scaled down by a factor of two.
	- this scaling is for backward compatability - no resolution is lost, as values are stored as float
	- note that values are also interpolated, because position is resampled for best match with EEG sampling records
	- position sampling frequency = 20KHz/(32*16) = 20000/512 = 39.0625 - this may change if base sample rate changes
	- unlike the .spot file from which it is derived, the beginning of the .whl file is filled with "-1 -1"
	- the idea is that each position sample corresponds with 512 electrophysiology samples (20KHz)
	- however, electrophysiology sampling starts before position sampling, so to to align the two, you need this filler
	- then, the first record in the .whl file represents the pretend video sample that would have occurred at electrophys time/sample zero
	- if either x or y value is invalid (<0), then both are set to -1
	- first call hux_checkcsicsvari and allocate required memory
	- float is used for storage to facilitate data transformations
	- assumptions...
		- memory has been allocated for time,x,y,head-direction,led-distance and result
	- NOTE:
		- function must be passed accurate "posfreq" value specifying .whl file sample frequency (Hz)
		- the value of posfreq returned by this function is simply what it is given in the first place
*************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

void hux_error(char message[]);
int hux_substring(char *source, char *target, int casesensitive);

int hux_readpos_csicsvari_asc (
		char *infile, /* filename to read*/
		double *pos_time, /* storage for timestamps (seconds)*/
		float *pos_x, /* storage for position (cm) */
		float *pos_y, /* storage for position (cm) */
		float *pos_headdir, /* storage for head direction (degrees) */
		float *led_dist, /* storage for distance between red & blue LEDs (cm) */
		float vidres, /* pixels/cm resolution */
		int vidxmax, /* maximum possible x-coordinate (typically 768/2 = 384) - passed back to calling function */
		int vidymax, /* maximum possible y-coordinate (typically 576/2 = 288) - passed back to calling function */
		float posfreq, /* sampling frequency of .whl file, old default = 39.0625 Hz */
		float *result /* 0=postot,1=vidxmax,2=vidymax,3=samplefreq*/
		) 
{
	int i,col,wordsperline=0,postot=-1,posvalid=0,badrangetot=0;
	char *pline,*pcol,line[1000],command[256];
	float posinterval,halfinterval,tempx,tempy,tempdir;
	FILE *fpin;

	posinterval= 1.00/posfreq; halfinterval= posinterval/2.00;
	if(hux_substring(infile,".whl",0)>=0) wordsperline=2; // position records: x,y
	else if(hux_substring(infile,".whd",0)>=0) wordsperline=3; // position+direction records, x,y,dir

	fpin=fopen(infile,"r");
	if(fpin==NULL) {sprintf(command,"hux_readpos_csicsvari_asc: can't open \"%s\"\n\n",infile);	hux_error(command);}
	
	/* read .whl or .whd file */
	while(fgets(line,1000,fpin)!=NULL) {
			pline=line;
			for(col=0;(pcol=strtok(pline," ,\t\n"))!=NULL;col++){
                pline=NULL;
                if(col==0) tempx=atof(pcol); 
				if(col==1) tempy=atof(pcol); 
				if(col==2) tempdir=atof(pcol);
			}
			/* only accept lines so long as there are the right number of "words" per line (x y [dir])*/
			if(col!=wordsperline) sprintf(command,"hux_readpos_csicsvari_asc: incorrect number of words (%d) on line %d",col,postot+2); 
			else {
				postot++;
				led_dist[postot] = -1; /* in this implimentation, no LED distance */
				pos_time[postot] = (postot*posinterval)+halfinterval;
				if(tempx<0||tempy<0) {pos_x[postot]=pos_y[postot]=pos_headdir[postot]=-1;continue;}
				else if(tempx>vidxmax||tempy>vidymax) {badrangetot++;pos_x[postot]=pos_y[postot]=pos_headdir[postot]=-1;continue;}
				else {
					posvalid++;
					pos_x[postot] = tempx/vidres;		// convert to cm
					pos_y[postot] = (vidymax-tempy)/vidres; // also invert y-coordinates using max camera y-pixels
					if(wordsperline==3) pos_headdir[postot] = tempdir;
	}}}
	fclose(fpin);

	result[0]= (float) postot+1;
	result[1]= (float) vidxmax;
	result[2]= (float) vidymax;
	result[3]= (float) posfreq;
	result[4]= (float) posinterval;
	result[5]= (float) badrangetot;
	return(posvalid);
}
