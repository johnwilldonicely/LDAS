/*
DESCRIPTION:
	Create a set of jittered x-values to apply to an input set of y-values

USES:
	Apply jitter to x-values in grouped data so points don't overlap in plot

DEPENDENCIES:
	xf_rand1_d

ARGUMENTS:
	double *yval  : input array, mean used for defining scaling for jitter
	long nn       : number of input/output items
	double centre : centre-value for result
	double limit  : limit for deviation from centre-value (absolute value)
	char *message : pre-allocated array to hold error message


RETURN VALUE:
	pointer to array on success, NULL on error
	message array will hold explanatory text (if any)

SAMPLE CALL:
	long ii,nn=100;
	double *xval=NULL, yval[100];
	char message[1000],*thisprog="test\0";

	for(ii=0;ii<nn;ii++) yval[ii]= (double)ii;
	xval= xf_jitter1_d(yval,nn,5,0.25,message);
	if(xval==NULL) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }
	for(ii=0;ii<nn;ii++) printf("%g\t%g\n",xval[ii],yval[ii]);

<TAGS> math </TAGS>
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>    /* needed for time() function */
#include <unistd.h>  /* needed for getpid() function */

double xf_rand1_d(double setmax);

double *xf_jitter1_d(double *yval, long nn, double centre, double limit, char *message) {

	char *thisfunc="xf_jitter1_d\0";
	int sizeofdouble=sizeof(double);
	unsigned long seed;
	long ii,jj,kk,sign;
	double aa,bb,cc,rr;
	double *result=NULL,ymin,ymax,xrange,sum,mean,maxdev,dev,scaling;

	/* CHECK VALIDITY OF ARGUMENTS */
	if(nn<1) { sprintf(message,"%s [ERROR]: invalid number of elements (%ld)",thisfunc,nn); return(NULL); }

	/* ALLOCATE MEMORY */
	result= realloc(result,(nn*sizeof(*result)));
	if(result==NULL) { sprintf(message,"%s [ERROR]: insufficient memory",thisfunc); return(NULL); }

	/* SET THE SEED VALUE FOR THE RAND FUNCTION */
	seed= (unsigned long) (time(NULL) + getpid());
	srand( seed );

	/* FIND THE DATA SUM AND MEAN */
	sum= 0.0;
	for(ii=0;ii<nn;ii++) { sum+= yval[ii]; }
	mean= sum/nn;

	/* FIND THE MAX DEVIATION FROM THE MEAN */
	/* jitter scaling will range from 0 (at maxdev) to 1 (at the mean) */
	maxdev= 0.0;
	for(ii=0;ii<nn;ii++) {
		aa= fabs((yval[ii]-mean));
		if(aa>maxdev) maxdev= aa;
	}

	/* INITIALIZE THE X-VALUES */
	sign=1;
	for(ii=0;ii<nn;ii++) {
		sign= 0-sign;
		/* store the current value */
		aa= yval[ii];
		/* calculate the deviation from the mean */
		dev= fabs(aa-mean);
		/* calculate the scaling factor, based on deviation from the mean */
		if(dev==0) scaling=1.0 ;
		else scaling= sign * (1.0-(dev/maxdev));
		/* build resulting x-value from properly ranged random number, with scaling applied */
		result[ii]= centre + (scaling * xf_rand1_d(limit));
	}

	return (result);
}
