#include <stdlib.h>
#include <stdio.h>
#include <math.h>
/*
<TAGS>stats signal_processing</TAGS>
DESCRIPTION:
	John Huxter: 20 January 2011
	Calculate theta-modulation in a histogram
		- looks for negative and positive inflections
		- histogram should be smoothed and spline-interpolated first
		- histogram should be normalized to range from 0-1
*/

int xf_histtheta1d (double *histx, double *histy, int nbins, double *result_d, char *message) {

	char *thisfunc="xf_histtheta1d\0";
	long int i,j,k;
	int startbin;
	double thetamin=4.0, thetamax=12.0;
	double width, halfbins, y1,y2,y3,v1,v2,v3,x1,x2,x3,vt,pt,vp,pp;
	double aa=0.0,bb=0.0;
	double secsperbin,binspersec;
 	double my_pi= 3.141592653589;


	width=histx[(nbins-1)]-histx[0];
	halfbins=(float)nbins/2.0; // histogram-midpoint
	secsperbin=width/(double)nbins; // seconds per bin
	binspersec=(double)nbins/width; // bins per second
	startbin= (int)(halfbins+(0.5/thetamax)*binspersec); // starting point = halfway to the period of the maximum rate

	vt=vp=pt=pp=-1.0;	// initialize holders for valley and peak times and probabilities

	// initialize variables holding sequential time and probability values from the histogram
	// basically fill first three values before starting peak-valley search at 3rd bin after the original startbin
	x1=histx[startbin]; y1=histy[startbin]; startbin++; if(startbin>=nbins) {sprintf(message,"%s: start-bin has exceeded total bins in histogram",thisfunc);return(-1);}
	x2=histx[startbin]; y2=histy[startbin]; startbin++; if(startbin>=nbins) {sprintf(message,"%s: start-bin has exceeded total bins in histogram",thisfunc);return(-1);}
	x3=histx[startbin]; y3=histy[startbin]; startbin++; if(startbin>=nbins) {sprintf(message,"%s: start-bin has exceeded total bins in histogram",thisfunc);return(-1);}

	for(i=startbin;i<nbins;i++) {

		if(histy[i]!=y3) { // if histy value has changed... (ignore flat sections)
			x1=x2; x2=x3; x3=histx[i]; // update 3-in-a-row times
			y1=y2; y2=y3; y3=histy[i]; // update 3-in-a-row probabilities
			if(y2<y1 && y2<y3) {vt=x2; vp=y2;} // store probability and time for valley
			if(y2>y1 && y2>y3) {pt=x2; pp=y2;} // store probability and time for peak
			if(vt>0 && pt>0) break; // if a peak and a valley has been found, stop looking
	}}

	if(pt<vt || vt<0 || pt<0) {
		result_d[0]=result_d[1]=result_d[2]=-1.0;
		if(pt<vt) sprintf(message,"%s: histogram peak occurred before valley",thisfunc);
		if(vt<0) sprintf(message,"%s: found no suitable trough in histogram",thisfunc);
		if(pt<0) sprintf(message,"%s: found no suitable peak in histogram",thisfunc);
		return(-1);
	}

	aa=(pt-vt)/pt; 		// quality-measure - range 0-1, 0.5 being ideal (valley is halfway to peak if this is an oscillatory valley-peak pair)

	result_d[0]= sin(M_PI*aa); // this transforms aa to a half-sine with max-value 1 at 0.5 and minima at 0 and 1 (below 0.9 is poor)
	result_d[1]=1.0/pt; 	// frequency (time of peak)
	result_d[2]=(pp-vp)/pp;	// depth of modulation - range 0-1
	sprintf(message,"%s: histogram valley at %g (value %g), peak at %g (value %g)",thisfunc,vt,vp,pt,pp);
	return(0);
}
