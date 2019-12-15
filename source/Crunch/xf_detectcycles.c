/***********************************************************************************
Detect cycles in a time series of data
Arguments: 
	int ndat,		// total number of samples
	double *time,	// array of ndat times (seconds)
	float *dat,		// array of ndat data values (typically voltages)
	float *phase,	// result-array to hold ndat phase-values
	int *cstart,	// result-array to hold cycle start-time indices 
	int *cend,		// result-array to hold cycle end-time indices
	float fmin,		// minimum frequency to be detected
	float fmax,		// maximum frequency to be detected
	float fitmax,	// maximum mean-square difference to accept sine-fit
	int invalid,	// invalid data values to be ignored
	int method		// detect cycle-starts at zero-crossing (1) peak-detect (2) or trough (3)
************************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
float xf_fitsine(float *data, int total, int alignment);

int xf_detectcycles(int ndat,double *time,float *dat,float *phase,int *cstart,int *cend,float fmin,float fmax,float fitmax,int invalid,int method) {

	long i,j,k;
	int start,end,found,cycletot=0,cycle_prev=0;
	int sizeoflong=sizeof(long),sizeofint=sizeof(int);;
	float a,fit,intmin,intmax,minhalf,maxhalf;

	/* convert frequency boundaries to cycle-duration (NOTE: min frequency = max interval) */
	intmin = 1.0/fmax; 
	intmax = 1.0/fmin; 

	/* pass first cycle (positive inflection) before fitting begins */
	j=ndat+1;	
	if(method==1) for(i=1;i<j;i++) if(dat[i]<0 && dat[i-1]>0) {cycle_prev = i;break;}
	if(method==2) for(i=1;i<j;i++) if(dat[i]>dat[i-1] && dat[i]>dat[i+1]) {cycle_prev = i;break;}
	if(method==3) for(i=1;i<j;i++) if(dat[i]<dat[i-1] && dat[i]<dat[i+1]) {cycle_prev = i;break;}

	for(i=cycle_prev+1;i<ndat;i++) {

		found=0;
		if(method==1) if(dat[i]<0 && dat[i-1]>0) found=1; 
		if(method==2) if(dat[i]>dat[i-1] && dat[i]>dat[i+1]) found=1; 
		if(method==3) if(dat[i]<dat[i-1] && dat[i]<dat[i+1]) found=1; 

		if(found==1) {
			start= cycle_prev; 			/* temp holder for previous cycle start */
			cycle_prev= i; 				/* current crossing becomes previous cycle start */
			a= time[i]-time[start]; 	/* calculate time elapsed since last zero xing */

			if(a>=intmin && a<=intmax) { 	/* if interval fits parameters, proceed */

				fit = xf_fitsine(dat+start,i-start+1,method);

				if(fit <= fitmax){
					/* store start and end times */
					cstart[cycletot]=start; cend[cycletot]=i; cycletot++;
					/* assign phase values to eeg data */ 
					a = 359.0 / (i-start); /* "a" is the phase-incriment of each sample */
					for(j=start;j<i;j++) phase[j] = (double) (j-start)*a;
				}
	}}}

	//for(i=0;i<cycletot;i++) {x=cstart[i]; y=cend[i];printf("%d	%d\n",x,y);}
	return(cycletot);
}
