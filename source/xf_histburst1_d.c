/*
<TAGS>stats signal_processing</TAGS>

DESCRIPTION:
	Calculate burstiness in a histogram
	Adapted from xf_histbursty1d, John Huxter: 7 February 2011

	1. histogram is downsampled to 50bins focussed on the +-50ms zone
	2. this histogram is smoothed with a 10ms Gaussian kernel (central bin +-2 bins)
	3. cubic spline interpolation is used to up-sample to 200 bins
	4. the first positive inflection at time greater-than-zero is identified
	5. the time at which this peak drops to 75% amplitude is identified
		- this is a measure of the rate of falloff in the histogram
	6. this dropoff time is normalized to the potential time at which it could have occurred


	Important assumptions regarding the input histogram:
		- should range from -0.05 to +0.05 s (+-50ms)
		- must have at least 0.5 bins per ms - so a +-50ms window should have at least 50 bins
		- y-values in the histogram can be counts or probabilities - it shouldn't matter - but the array must be a double-float

		NOTE: scores based on fewer than 50 spikes in the histogram tend to be unreliable
		NOTE: a "bursty" cell should ideally have a peak-time less than 0.012s and a burstiness score > 0.75?

USES:

DEPENDENCIES:
	xf_bin1a_d
	xf_round1_d
	xf_smoothgauss1_d
	xf_spline1_d

ARGUMENTS:
	double *histx1 : input histogram x-values (typically time in seconds)
	double *histy1 : input histogram y-values (typically spike-counts)
	long nbins1    : number of x/y pairs in the histogram
	double *result_d : pre-allocated array to hold results - must allow at least 3 elements
	char *message : pre-allocated array to hold error message

RETURN VALUE:
	0 on success
	-1 on warning (eg. no peak in the histogram)
	-2 error related to input histogram or parameters
	-3 fatal memory allocation error

	result array will hold statistics
	char array will hold message (if any)

SAMPLE CALL:

*/

#include <stdlib.h>
#include <stdio.h>

double xf_bin1a_d(double *data, size_t *setn, size_t *setz, size_t setbins, char *message);
double xf_round1_d(double input, double setbase, int setdown);
int xf_smoothgauss1_d(double *original,size_t arraysize,int smooth);
void xf_spline1_d(double xin[],double yin[],long nin,double xout[],double yout[],long nout);

int xf_histburst1_d(double *histx1, double *histy1, long nbins1, double *result_d, char *message) {

	char *thisfunc="xf_histburst1_d\0", *message2;
	long ii,jj,kk,nbins2=0,nbinsmin,nbins3,setz,startbin;
	double aa,bb,cc,*histx2=NULL,*histx3=NULL,*histy2=NULL,*histy3=NULL;
	double width,binwidth3,x1,x2,x3,y1,y2,y3;
	double peak_time=-1.0,peak_value=-1.0,drop_time=-1.0,drop_value;

	nbinsmin=50; // number of bins in downsampled version of input histogram - also the minimum bins permitted in the input
	nbins3=200; // number of bins in spline-interpolated histogram used for peak & valley detection

	/* initialize results values */
	for(ii=0;ii<5;ii++) result_d[ii]=-1.0;
	/* determine the half-bin-width */
	aa= (histx1[1]-histx1[0])/2.0;
	/* determine the actual theoretical edges of the data encompassed by the histogram  */
	/* assumes the histogram was generated so x-values represent the mid-point of each bin */
	bb= histx1[0]-aa;
	cc= histx1[(nbins1-1)]+aa;
	/* round the edges to nearest milisecond */
	bb= xf_round1_d(bb,0.001,0);
	cc= xf_round1_d(cc,0.001,0);
	/* make sure histogram covers +-50ms */
	if(bb>-0.05) {sprintf(message,"%s [ERROR]: histogram data-range starts at time %g (must be <= -0.05)",thisfunc,bb);return(-2);}
	if(cc<0.05)  {sprintf(message,"%s [ERROR]: histogram data-range ends at time %g (must be >= +0.05)",thisfunc,cc);return(-2);}

	/********************************************************************************
	ALLOCATE MEMORY FOR TEMPORARY HISTOGRAMS
	********************************************************************************/
	histx2= realloc(histx2,(nbins1)*sizeof(*histx2));
	histy2= realloc(histy2,(nbins1)*sizeof(*histy2));
	histx3= realloc(histx3,(nbins3)*sizeof(*histx3));
	histy3= realloc(histy3,(nbins3)*sizeof(*histy3));
	if(histx2==NULL||histx3==NULL||histy2==NULL||histy3==NULL) {sprintf(message,"%s [ERROR]: insufficient memory",thisfunc);return(-3);}

	/********************************************************************************
	BUILD A WORKING COPY OF THE INPUT HISTOGRAM FOCUSSING ON THE +-50MS ZONE
	********************************************************************************/
	for(ii=nbins2=0;ii<nbins1;ii++) {
		aa=histx1[ii];
		if(aa>=-0.05 && aa<=0.05) { histx2[nbins2]=aa; histy2[nbins2]=histy1[ii]; nbins2++; }
	}
	//TEST:	for(ii=0;ii<nbins2;ii++) printf("%g\t%g\n",histx2[ii],histy2[ii]); exit(1);

	/* flag problems with input */
	if(nbins2<nbinsmin) {sprintf(message,"%s [ERROR]: input histogram has only %ld bins in the critical +-50ms zone (must have >= %ld)",thisfunc,nbins2,nbinsmin);return(-2);}

	/********************************************************************************
	RESAMPLE THE HISTOGRAM IF NECESSARY - SHOULD BE 50 BINS AT 2MS PER BIN
	- the only other option is that it is already the right number of bins, so no change is needed
	********************************************************************************/
	if(nbins2>nbinsmin) {
		/* find zero */
		setz=-1; for(ii=0;ii<nbins2;ii++) if(histx2[ii]>=0.00) {setz=ii;break;}
		if(setz<0) {sprintf(message,"%s [ERROR]: no time >= zero identified in input histogram",thisfunc);return(-2);}
		/* bin the y-values */
		aa= xf_bin1a_d(histy2,&nbins2,&setz,nbinsmin,message2);
		if(aa==0) {sprintf(message,"%s/%s",thisfunc,message2);return(-1);}
		/* reasign x-values for new histogram (aa=binsize) */
		jj=1;  for(ii=setz;ii<nbins2;ii++) { histx2[ii]= aa*jj; jj++; }
		jj=-1; for(ii=(setz-1);ii>=0;ii--) { histx2[ii]= aa*jj; jj--; }
	}
	//TEST: fprintf(stderr,"nbins1=%ld	nbins2=%ld\n",nbins1,nbins2);
	//TEST: for(ii=0;ii<nbins2;ii++) printf("%g\t%g\n",histx2[ii],histy2[ii]); exit(1);

	/********************************************************************************
	SMOOTH THE HISTOGRAM Y-VALUES (2-BINS HALF-WINDOW, SO FULL GAUSSIAN WINDOW= 3BINS = 6MS
	********************************************************************************/
	kk= xf_smoothgauss1_d(histy2,(size_t)(nbins2),2);
	if(kk!=0) {sprintf(message,"%s [ERROR]: insufficient memory for smoothing",thisfunc);return(-3);}
	//TEST:	for(ii=0;ii<nbins2;ii++) printf("%g\t%g\n",histx2[ii],histy2[ii]); exit(1);

	/********************************************************************************
	APPLY SPLINE INTERPOLATION TO UPSAMPLE HISTOGRAM TO 200 POINTS
	- after this, each bin represents 0.5ms, and bin100= 0-0.5ms
	- note also that histx3[] values represent the mid-point of each bin (so add 0.25 to the bin-bunmber...)
	- hence the peak should not occur before bin 104 (= 0 + (4*0.5ms) + 0.25 )
	- similarly, the 25%drop cannot really occur before bin 105 (=
	********************************************************************************/
	xf_spline1_d(histx2,histy2,nbins2,histx3,histy3,nbins3);
	//TEST:	for(ii=0;ii<nbins3;ii++) printf("%g\t%g\n",histx3[ii],histy3[ii]); exit(1);

	/* FREE MEMORY */
	free(histx2);
	free(histy2);

	/* DETERMINE THE SIZE OF THE WINDOW IN WHICH THE PEAK COULD REALISTICALLY BE FOUND ( USUALLY 2MS-50MS) */
	/* - this is half the histogram, minus the 2ms refractory period, minus the bin-width */
	aa= (histx3[(nbins3-1)] - histx3[0])/2.0; // histogram half-width
	binwidth3= (histx3[1]-histx3[0])/2.0;     // width of each bin
	width= aa - 0.002 - binwidth3 ;           // size of the zone (seconds) where the 25% drop could be found

	/* INITIALIZE VARIABLES HOLDING SEQUENTIAL TIME AND VALUES FROM THE HISTOGRAM */
	/* basically fill first three values before starting peak-valley search at 3rd bin after the original startbin */
	/* assume mid-point of the new histogram is zero */
	startbin= (long)(nbins3/2.0);
	x1=histx3[startbin]; y1=histy3[startbin]; startbin++;
	x2=histx3[startbin]; y2=histy3[startbin]; startbin++;
	x3=histx3[startbin]; y3=histy3[startbin]; startbin++;

	/* FIND FIRST-PEAK-TIME AND SUBSEQUENT-DROP-TIME */
	for(ii=startbin;ii<nbins3;ii++) {
		if(histy3[ii]!=-999) { /* if histy3 value has changed... (ignore flat sections)  */
			x1=x2; x2=x3; x3=histx3[ii]; /* update 3-in-a-row times */
			y1=y2; y2=y3; y3=histy3[ii]; /* update 3-in-a-row counts */
			if(peak_time<0 && y2>y1 && y2>y3) {peak_time=x2; peak_value=y2; drop_value=0.8*peak_value; } /* time & value of the first peak */
			if(peak_time>0 && y2<=drop_value) {drop_time=x2; break; } /* if a peak has been found, record when value drops by 25% or more */
		}
	}
	if(peak_time<0) { sprintf(message,"%s [WARNING]: no peak detected in histogram",thisfunc); return(-1);}
	if(drop_time<0) { drop_time= x3; }


	result_d[0]= peak_time;
	result_d[1]= drop_time;
	result_d[2]= (width-(drop_time-0.002-binwidth3)) / width;

	/* FREE MEMORY AND RETURN */
	free(histx3);
	free(histy3);
	return(0);
}
