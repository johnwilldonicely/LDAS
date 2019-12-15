/*
<TAGS>signal_processing detect</TAGS>

DESCRIPTION:
	Detect start peak & stop times for cycles (oscillations) in data
	Detection is on the trough (inflection)
	Peak-point is taken as the highest value
	Differences from xf_detectcycles1_f:
		- no sine-fit test of fit
		- above is presumed to have been handled by a preceeding filtering step
		- addition of peak(peak) output


USES:

DEPstopENCY TREE:
	No depstopencies

ARGUMENTS:
	float *data     : pointer to input (time series)
	size_t ndat     : number of elements in the data array
	size_t cmin 	: minimum duration of cycle to be included
	size_t cmax 	: maximum duration of cycle to be included
	size_t **cstart : address to array to hold results (cycle start-times) - calling function must free()
	size_t **cpeak  : address to array to hold results (cycle peak(peak)-times) - calling function must free()
	size_t **cstop  : address to array to hold results (cycle stop-times) - calling function must free()
	size_t *cycletot: address to variable (not an array) holding the total cycles detected

RETURN VALUE:
	0 on success, -1 on failure

SAMPLE CALL:
*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

int xf_detectcycles2_f(float *data, size_t ndat, size_t cmin, size_t cmax, size_t **cstart, size_t **cpeak, size_t **cstop, size_t *ctot, char *message) {

	char *thisfunc="xf_detectcycles2_f\0";
	size_t ii,jj,kk,start,stop,prevstart=0,ncycles=0;
	size_t *tempstart=NULL,*temppeak=NULL,*tempstop=NULL;
	int tempsize=sizeof(size_t);
	float peak;

	//TEST: fprintf(stderr,"cmin:%ld\ncmax:%ld\nndat:%ld\n",cmin,cmax,ndat);

	/* pass first cycle before fitting begins */
	jj=ndat+1;
	for(ii=1;ii<jj;ii++) {
		if(data[ii]<data[ii-1] && data[ii]<data[ii+1]) {
			prevstart = ii;
			break;
	}}

	for(ii=prevstart+1;ii<ndat;ii++) {

		/* if a negative inflection is detected... */
		if(data[ii]<data[ii-1] && data[ii]<data[ii+1]) {

			jj= ii-prevstart;
			/* if cycle is too short, keep looking for next inflection */
			if(jj<cmin) { prevstart= ii; continue; }
			/* if cycle is too long, reject current cycle and start a new one */
			if(jj>cmax) { prevstart= ii; continue; }

			/* dynamically allocate memory for cycle start/stop times */
			if((tempstart=(size_t *)realloc(tempstart,(ncycles+1)*tempsize))==NULL) {
				sprintf(message,"%s [ERROR]: insufficient memory",thisfunc);
				return(-1);
			}
			if((temppeak=(size_t *)realloc(temppeak,(ncycles+1)*tempsize))==NULL) {
				sprintf(message,"%s [ERROR]: insufficient memory",thisfunc);
				return(-1);
			}
			if((tempstop=(size_t *)realloc(tempstop,(ncycles+1)*tempsize))==NULL) {
				sprintf(message,"%s [ERROR]: insufficient memory",thisfunc);
				return(-1);
			}

			/* find the cycle-peak */
			kk=prevstart+1;
			peak=data[kk]; /* because peak can't possibly be an inflection point */
			for(jj=kk;jj<ii;jj++) { if(data[jj]>peak) { peak=data[jj]; kk=jj; } }
			/* store start peak and stop times */
			tempstart[ncycles]= prevstart;
			temppeak[ncycles]= kk;
			tempstop[ncycles]= ii;
			prevstart= ii; 	/* current crossing also becomes previous cycle start */
			ncycles++;

		}
	}

	sprintf(message,"%s : detected %ld cycles",thisfunc,ncycles);

	/* set pointers to input arguments to results */
	(*ctot)=ncycles;
	(*cstart)=tempstart;
	(*cpeak)=temppeak;
	(*cstop)=tempstop;

	return(0);
}
