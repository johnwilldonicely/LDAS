/*
<TAGS>math</TAGS>

DESCRIPTION:
	Calculate an array of velocity values using Bob Muller's "cord" method
	- define size of window (seconds) : recommend 0.4 seconds = 10 samples at 25Hz
	- velocity is integrated across the cord joining the start & end of the window
	- one velocity estimate is generated for every position, aligned to the middle of each cord
	- the very beginning and end of the velocity array is filled with the nearest calculated velocity value
	- based on old CRUNCH function hux_posvel
	- NOTE: if there are NANs in the input, interpolation should be applied to velocity result

USES:

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	float *posx     : input array of x-position values (cm)
	float *posy     : input array of y-position values (cm)
	float *velocity : output array of velocity values (cm/second) - must be preallocated by calling function (nn elements)
	long nn         : total number of position samples (and velocity output values)
	double winsecs  : size (seconds) of window for velocity integration (0.4 recommended)
	double samprate : sample rate of video record (samples/second)
	char *message   : pre-allocated array to hold error message

RETURN VALUE:
	0 on success, -1 on error
	message array will describe error, if any

SAMPLE CALL:
	x= xf_velocity1(posx,posy,velocity,10000,0.4,25,message);
	if(x==-1) { fprintf(stderr,"\n\t--- %s/%s\n\n",thisprog,message); exit(1); }

*/

#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<string.h>

int xf_velocity1(float *posx, float *posy, float *velocity, long nn, double winsecs, double samprate, char *message) {

	char *thisfunc="xf_velocity1\0";
	long ii,jj,winsize,winstart,winmid,first,last,veltot=0;
	double aa,bb,cc,dd;

	/* CALCULATE WINDOW SIZE IN SAMPLES */
	winsize=(long)(winsecs*samprate);

	/* INTEGRITY CHECK */
	if(winsize<2)   { sprintf(message,"%s [ERROR]: window (%g seconds) spans less than 2 samples",thisfunc,winsecs); return(-1); }
	if(winsize>=nn) { sprintf(message,"%s [ERROR]: window (%g seconds) exeeds no. of position samples",thisfunc,winsecs); return(-1); }

	/* INITIALIZE POSITION OF WINDOW START AND MIDDLE */
	/* NOTE that these will be incremented at the top of the main loop */
	winstart= -1;
	winmid= (long)((double)winsize/2.0)-1;
	/* save a record of the first sample that will be assigned a velocity in the main loop */
	first= winmid+1;

	/* MAIN LOOP: start at winsize samples into the position record */
	/* velocity values are applied to the mid-point of the sliding window */
	for(ii=winsize;ii<nn;ii++) {
		winstart++; /* update index to start of window */
		winmid++;   /* update index to middle of window */
		if(isfinite(posx[ii]) && isfinite(posx[winstart])) {
			veltot++; /* total number of valid path segments calculated */
			aa= posx[ii] - posx[winstart]; /* change in x */
			bb= posy[ii] - posy[winstart]; /* change in y */
			cc= sqrt(aa*aa + bb*bb); /* path segment length */
			velocity[winmid]= cc/winsecs; /* current sample is velocity based on preceding winsize samples */
		}
		else velocity[winmid]= NAN;
	}

	/* FILL THE BEGINNING AND END OF THE VELOCITY ARRAY WITH THE FIRST AND LAST CALCULATED VELOCITY VALUE, RESPECTIVELY */
	for(ii=0;ii<first;ii++)     velocity[ii]= velocity[first];
	for(ii=winmid;ii<nn;ii++) velocity[ii]= velocity[winmid];

	return(0);
}
