/*
<TAGS>stats</TAGS>

DESCRIPTION:John Huxter: 17 February 2015

	- based on xf_wavewidth.c, which was designed to detect compound waveform width for action potentials

	A function to find the peak and width in a curve
	Peak can be positive or negative
	Width is the width at the point at which a percentage of the peak is crossed
	For compound curves (multiple channels in sequence) the width of the channel containing the peak is used


USES:

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	float *curve    : input array of values representing a curve
	int nn         : number of samples in the curve
	int nchan      : if a compound curve, the number of channels (eg. 4 for a tetrode waveform)
	float thresh   : threshold as a percentage of the max or min value (see sign, below)
	int sign       : whether thresholdhing should be set to maximum (1) or minimum (-1)
	float *results : pre-allocated array to hold the results of the analysis
	char *message  : pre-allocated array to hold error message

RETURN VALUE:
	index to the detected peak
	-1 on error

	result array will hold statistics
		result[0]=peak value
		result[1]=width at threshold
		result[2]=threshold used

	char array will hold message (if any)

SAMPLE CALL:




*/

#include <stdlib.h>
#include <stdio.h>

long xf_curvestats3_f(float *curve,long nn,int nchan,float thresh,int sign,float *result,char *message) {

	char *thisfunc="xf_curvestats3_f\0";
	long ii,jj,kk,x1,x2,x3;
	long peakindex,chanstart,chanend,partlen;
	int chan;
	float a,b,y1,y2,y3,peakval,width;

	if(nn==0) { sprintf(message,"%s: no data",thisfunc); return(-1); }
	if(nchan<1||nchan>nn) { sprintf(message,"%s: invalid number of channels (%d)",thisfunc,nchan); return(-1); }
	if(sign!=-1 && sign!=1) { sprintf(message,"%s: invalid sign (%d) - must be -1 or 1",thisfunc,nchan); return(-1); }
	if(thresh<=0 || thresh>=1) { sprintf(message,"%s: invalid precent threshold (%g) - must be between 0 and 1",thisfunc,thresh); return(-1); }

	/* find the negative or positive peak */
	peakval=curve[0];
	if(sign==1)
		for(ii=0;ii<nn;ii++) {if(curve[ii]>peakval) {peakval=curve[ii];peakindex=ii;}}
	else
		for(ii=0;ii<nn;ii++) {if(curve[ii]<peakval) {peakval=curve[ii];peakindex=ii;}}

	/* convert threshold from a percentage to an actual vcalue based on peakval */
	thresh *= peakval;

	/* if curve is compound, establish length of curve on each channel */
	partlen=nn/nchan;

	/* determine channel and samples bounding data from that channel */
	/* if curve is not compound, chanstart=0 and chanend=nn */
	chan=(int)((float)peakindex/(float)partlen);
	chanstart=chan*partlen;
	chanend=chanstart+partlen-1;


	/* look back for point at which thresh is crossed */
	if(sign==-1)
		for(ii=peakindex;ii>=chanstart;ii--) {if(curve[ii]>thresh) break; else x1=ii;}
	else
		for(ii=peakindex;ii>=chanstart;ii--) {if(curve[ii]<thresh) break; else x1=ii;}
	/* calculate interpolated (fractional) sample-number at which threshold was crossed */
	x2=ii;
	y1=curve[x1];
	y2=curve[x2];
	a=x1-(thresh-y1)/(y2-y1);

	/* look forward for point at which thresh is crossed */
	if(sign==-1)
		for(ii=peakindex;ii<=chanend;ii++) {if(curve[ii]>thresh) break; else x1=ii;}
	else
		for(ii=peakindex;ii<=chanend;ii++) {if(curve[ii]<thresh) break; else x1=ii;}
	/* calculate interpolated (fractional) sample-number at which threshold was crossed */
	x2=ii;
	y1=curve[x1];
	y2=curve[x2];
	b=x1+(thresh-y1)/(y2-y1);

	/* calculate width in fractional number of samples */
	width=(b-a);

	result[0]=peakval;
	result[1]=width;
	result[2]=thresh;

	return(peakindex); /* status good */
}
