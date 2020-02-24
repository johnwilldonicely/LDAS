/*
<TAGS>dt.matrix</TAGS>
DESCRIPTION:
	Calculate the spatial coherence of a matrix
	- how well does one bin rate predict the 8 neighbouring bins
	- coherence = z-transform of correlation between each matrix element and the mean of surrounding elements
	- non-finite elements will be ignored

USES:

DEPENDENCIES:
	double  xf_correlate_simple_d(double *x, double *y, long nn, double *result_in);

ARGUMENTS:
	double *rate       : the input matrix array
	long width         : matrix width (number of elements)
	long height        : matrix height (number of elements)
	double *result_out : pre-allocated array to hold results (16-elements)
	char *message      : pre-allocated array to hold error message, if any

RETURN VALUE:
	- the spatial coherence in the matrix, or NAN on error
	- result array will hold additional statistics
		result_out[0]=

SAMPLE CALL:

*/

#include<stdio.h>
#include<stdlib.h>
#include<math.h>

/* external functions start */
double xf_correlate_simple_d(double *x, double *y, long nn, double *result_in);
/* external functions end */

double xf_matrixcoh1_d(double *rate,long width,long height, char *message) {

	char *thisfunc="xf_matrixcoh1_d\0";
	long ii,jj,kk,w,x,y,z,n1,n2,n3,p1,p2,xbin,ybin;
	double *avg,r,temprate,result_in[32];
	double coherence;

	/* CHECK ARGUMENTS */
	if(width<=0||height<=0) {sprintf(message,"%s [ERROR]: inappropriate width (%ld) and/or height (%ld)",thisfunc,width,height);return(NAN);}

	/* SIZE OF DWELL AND RATE ARRAYS */
	n1= width*height;

	/* CHECK WHETHER THERE ARE ANY VALID VALUES */
	n2=0; for(ii=0;ii<n1;ii++) if(isfinite(rate[ii])) n2++;
	if(n2<=0) return(NAN);

	/* ASSIGN MEMORY FOR TEMPORARY AVERAGE ARRAY */
	avg= malloc(n1*sizeof(*avg));
	if(avg==NULL) {sprintf(message,"%s [ERROR]: insufficient memory",thisfunc);return(NAN);}

	/********************************************************************************/
	/* CALCULATE THE SPATIAL COHERENCE  */
	/********************************************************************************/
	for(ybin=0;ybin<height;ybin++) {
		for(xbin=0;xbin<width;xbin++)	{
			p1= ybin*width+xbin; /* set pointer to central (current) bin */
			temprate= rate[p1];
			if(!isfinite(temprate)) continue;

			/* create the avg array - average rate of up to 8 bins (p2) around central bin (p1) : omit unvisited bins */
			n3= 0; avg[p1]= 0.00; w= xbin-1; z= xbin+1;
			/* do top row */
			y= ybin-1; if(y>=0) {for(x=w;x<=z;x++) {p2= y*width+x; avg[p1]+= rate[p2]; n3++;}}
			/* do bottom row */
			y= ybin+1; if(y<width) {for(x=w;x<=z;x++) {p2= y*width+x; avg[p1]+= rate[p2]; n3++;}}
			/* do left side (xbin-1 = w)*/
			if(w>=0) {p2= ybin*width+w; avg[p1]+= rate[p2]; n3++;}
			/* do right side (xbin+1 = z)*/
			if(z<width) {p2= ybin*width+z; avg[p1]+= rate[p2]; n3++;}
			/* for this central bin, calculate the average of the surrounding bins */
			if(n3>0) avg[p1]= avg[p1]/n3;
			else avg[p1]= NAN;
			}
		}

	/* get the correlation with the avg of surrounding bins - non-finite values will be ignored  */
	r= xf_correlate_simple_d(rate,avg,n1,result_in);
	/* perform the Fisher z-transform */
	coherence = atanhf(r);


	/* FREE MEMORY AND RETURN RESULTS  */
	free(avg);
	return(coherence);
}
