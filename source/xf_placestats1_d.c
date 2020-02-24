/*
<TAGS>stats dt.matrix</TAGS>
DESCRIPTION:
	Calculate statistics on cell firing rate map pattern...

	Pi = probability of being n bin i, = dwell time in bin i divided by trial length
	Ri = firing rate in bin i
	R = mean firing rate over entire trial

	- cohererence of place cell firing (how well does one bin rate predict the neighbouring bin rate)
		coherence = z-transform of correlation between binrates and the mean of surrounding binrates,

	- sparsity is roughly equivalent to the proportion of the environment the cell is likely to be active in
		= (SUM{Pi*Ri})^2 * SUM(Pi*Ri^2)

	- information content (how well does cell firing predict location - high info = small fields)
		= SUM {Pi*(Ri/R) log2*(Ri/R)}
		NOTE: log2(x) = log(x)/log(2)

USES:

DEPENDENCIES:
	long    xf_stats3_d(double *data, long n, int varcalc, double *result_in);
	double  xf_correlate_simple_d(double *x, double *y, long nn, double *result_in);
	int     xf_percentile1_d(double *data, long n, double *result);

ARGUMENTS:
	double *dwell       : dwelltime array (make sure unvisited pixels are zero, not NAN)
	double *rate        : firing rate array
	long width          : width of array
	long height         : height of array
	double dwelltot     : total duration of time in all "valid" bins
	double *result_out  : array to hold outputs - should point to 32-element array
	char *message       : pre-allocated array to hold error message

RETURN VALUE:
	- total good dwell & rate values, or -1 on error
	- char array will hold message (if any)
	- result array will hold statistics
		result_out[0]= maxrate;
		result_out[1]= meanrate;
		result_out[2]= baserate;   // 10th percentile
		result_out[3]= medianrate; // 50th percentile
		result_out[4]= peakrate;   // 97.5th percentile = midpoint of 95th percentile
		result_out[5]= sdrate;
		result_out[6]= info_content;
		result_out[7]= sparsity;
		result_out[8]= coherence;

SAMPLE CALL:

*/

#include<stdio.h>
#include<stdlib.h>
#include<math.h>

/* external functions start */
long xf_stats3_d(double *data, long n, int varcalc, double *result_in);
double xf_correlate_simple_d(double *x, double *y, long nn, double *result_in);
int xf_percentile1_d(double *data, long n, double *result);
/* external functions end */

long xf_placestats1_d(double *dwell,double *rate,long width,long height,double dwelltot,double *result_out, char *message) {

	char *thisfunc="xf_placestats1_d\0";
	long ii,jj,kk,w,x,y,z,n1,n2,n3,p1,p2,xbin,ybin;
	double *avg,r,info,temprate,result_in[32];
	double baserate,meanrate,sdrate,medianrate,peakrate,maxrate;
	double a=0.0,SUMx=0.0,SUMy=0.0,SUMx2=0.0,SUMy2=0.0,SUMxy=0.0,SSx=0.0,SSy=0.0,SPxy=0.0;
	double dwellratio,rateratio,s1=0.0,s2=0.0,info_content,coherence,sparsity;

	if(width<=0||height<=0) {sprintf(message,"%s [ERROR]: inappropriate width (%ld) and/or height (%ld)",thisfunc,width,height);return(-1);}

	avg= malloc(width*height*sizeof(*avg)); /* assign memory for temporary average array */
	if(avg==NULL) {sprintf(message,"%s [ERROR]: insufficient memory",thisfunc);return(-1);}

	/* size of dwell and rate arrays */
	n1= width*height;

	/* calculate basic rate statistics */
	for(ii=0;ii<32;ii++) result_in[ii]=0;
	n2= xf_stats3_d(rate,n1,1,result_in);
	meanrate= result_in[0];
	sdrate= result_in[2];
	maxrate= result_in[5];

	/* get percentile breakdown  */
	for(ii=0;ii<32;ii++) result_in[ii]=0;
	z= xf_percentile1_d(rate,n1,result_in);
	baserate= result_in[3];   // 10th percentile
	medianrate= result_in[5]; // 50th percentile
	peakrate= result_in[9];   // 97.5th percentile = midpoint of 95th percentile


	/********************************************************************************/
	/* CALCULATE THE STATS */
	/********************************************************************************/
	info_content = coherence = sparsity = 0.0;
	if(meanrate<=0) info_content = sparsity = coherence = -1.0;
	else {
		for(ybin=0;ybin<height;ybin++) {
			for(xbin=0;xbin<width;xbin++)	{

				p1= ybin*width+xbin; /* set pointer to central (current) bin */

				if(dwell[p1]<=0) {avg[p1]= NAN; continue;}
				temprate= rate[p1];

				rateratio= temprate/meanrate;
				dwellratio= dwell[p1]/dwelltot;
				if(rateratio>0) info_content+= (dwellratio)*(rateratio)*log2(rateratio); /* add up information content */
				s1+= dwellratio*temprate;	/* numerator for sparsity calculation */
				s2+= dwellratio*temprate*temprate; /* denominator for sparsity calculation */

				/* create the avg array - average rate of up to 8 bins (p2) around central bin (p1) : omit unvisited bins */
				n3= 0; avg[p1]= 0.00; w= xbin-1; z= xbin+1;
				/* do top row */
				y= ybin-1; if(y>=0) {for(x=w;x<=z;x++) {p2= y*width+x; if(dwell[p2]>0) {avg[p1]+= rate[p2]; n3++;}}}
				/* do bottom row */
				y= ybin+1; if(y<width) {for(x=w;x<=z;x++) {p2= y*width+x; if(dwell[p2]>0) {avg[p1]+= rate[p2]; n3++;}}}
				/* do left side (xbin-1 = w)*/
				if(w>=0) {p2= ybin*width+w; if(dwell[p2]>0) {avg[p1]+= rate[p2]; n3++;}}
				/* do right side (xbin+1 = z)*/
				if(z<width) {p2= ybin*width+z; if(dwell[p2]>0) {avg[p1]+= rate[p2]; n3++;}}
				/* for this central bin, calculate the average of tehe surrounding bins */
				if(n3>0) avg[p1]= avg[p1]/n3;
				else avg[p1]= NAN;
			}
		}

		/* CALCULATE SPARSITY */
		sparsity = (s1*s1)/s2;

		/* CALCULATE SPATIAL-COHERENCE (PEARSON'S R FOR CORRELATION BETWEEN BINRATE AND AVG RATE) */
		r= xf_correlate_simple_d(rate,avg,n1,result_in);
		/* perform the Fisher z' transform */
		coherence = atanhf(r);
	}


	/* FREE MEMORY FOR TEMPORAARY AVERAGES  */
	free(avg);

	/* FILL THE RESULTS ARRAY */
	result_out[0]= maxrate;
	result_out[1]= meanrate;
	result_out[2]= baserate;   // 10th percentile
	result_out[3]= medianrate; // 50th percentile
	result_out[4]= peakrate;   // 97.5th percentile = midpoint of 95th percentile
	result_out[5]= sdrate;
	result_out[6]= info_content;
	result_out[7]= sparsity;
	result_out[8]= coherence;

	/* RETURN THE NUMBER OF GOOD BINS */
	return(n2);
}
