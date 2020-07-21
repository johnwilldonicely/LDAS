/*
<TAGS>signal_processing filter</TAGS>

DESCRIPTION:
	Remove jumpy points from an x/y position series
	- unlike filtering, will not alter the posx outside the "jumpy" events
	- points are invalidated (assigned to NAN) until one is found that doesn't look jumpy relative to the last good point
	- note that once enough time has elapsed, a series of displaced points can be considered non-jumpy again.

USES:
	Remove rapid changes in apparent position due to transient video tracking problems (reflection-tracking etc)
	Can help to remove noise artefacts

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	float *posx   : pointer to array holding x-position data
	float *posy   : pointer to array holding y-position data
	long nn       : number of elements in the arrays
	double sfreq  : sample-frequency of input (samples per second)
	double thresh : the threshold for rate-of-change, beyond which posx & posy are reassigned to NAN

RETURN VALUE:
	Number of jumpy points invalidated
	No error return for this function

SAMPLE CALL:
	# in a video data record sampled t 25Hz, invalidate movement > 200cm/s
	njumps= xf_dejump2_f(posx, posy, nn, 25.0, 200.0)
*/

#include <math.h>

long xf_dejump2_f(float *posx, float *posy, long nn, double sfreq, double thresh) {

	long ii,njumps;
	float prevx,prevy,thisx,thisy;
	double aa,bb,cc,dd;
	double interval,prevtime,thistime,velocity;

	/* calculate number of samples corresponding to interval */
	interval=1.0/(double)sfreq;

	/* seek to first valid datum */
	ii=0; while(!isfinite(posx[ii]) || !isfinite(posy[ii]) ) ii++;

	/* assign the first reference values for calculating rate-of-change */
	prevx= posx[ii];
	prevy= posy[ii];
	prevtime= (double)ii*interval;

	/* scan through the array starting at the next point */
	njumps=0;
	for(ii=(ii+1);ii<nn;ii++) {

		thistime=(double)ii*interval;
		thisx= posx[ii];
		thisy= posy[ii];

		if(isfinite(thisx) && isfinite(thisy)) {

			/* calculate velocity */
			aa= thisx-prevx; /* change in x */
			bb= thisy-prevy; /* change in y */
			cc= sqrt(aa*aa + bb*bb); /* path segment length */
			dd= thistime-prevtime; /* change in time */
			velocity= cc/dd;

			if(velocity>thresh){
				posx[ii]=NAN; /* invalidate current sample */
				posy[ii]=NAN; /* invalidate current sample */
				njumps++; /* increment the number of jumpy points found */
			}
			else {
				/* define new reference sample if current point is not jumpy */
				prevx=thisx;
				prevy=thisy;
				prevtime=thistime;
			}
		}
	}
	return (njumps);
}
