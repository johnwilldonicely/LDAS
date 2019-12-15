/*******************************************************************************
Fit a series of data to a sinusoid
- use ALIGNMENT argument to set which part of the sinusoid is aligned to zero
- returns the mean squared difference between data and the ideal sinusoid
- returns -1 if invalid alignment argument is passed
- theoretical result can range from zero (perfect fit) to 0.635739 (horizontal line at zero)
- actual results range from about 0.3 to 0.8, with a mean of 0.611266
********************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

float xf_fitsine(float *data, int total, int alignment) {
	int x; float a,ymin,ymax,yrange,yoffset,xadjust,meansqdif=0.0;

	// determine x- and y-adjustment factors
	ymin=ymax=data[0];
	for(x=0;x<total;x++) {if(data[x]<ymin)ymin=data[x]; if(data[x]>ymax)ymax=data[x];}
	yoffset= 0.0-ymin; yrange=(ymax-ymin)/2.0; // used to scale yrange to -1 to 1
	xadjust= (-2.0 * M_PI)/total; // used to scale x-range to 0 to PI

	if(alignment==1) {	// align zero to positive-to-negative zero-crossing
		for(x=0;x<total;x++) {
			a=sin(x*xadjust) - ( ((data[x]+yoffset)/yrange) -1 ); 
			meansqdif += a*a;
	}}
	else if(alignment==2) {	// align zero to peak
		for(x=0;x<total;x++) {
			a=cos(x*xadjust) - ( ((data[x]+yoffset)/yrange) -1 ); 
			meansqdif += a*a;
	}}
	else if(alignment==3) {	// align zero to trough
		for(x=0;x<total;x++) {
			a=cos(x*xadjust - M_PI) - ( ((data[x]+yoffset)/yrange) -1 ); 
			meansqdif += a*a;
	}}
	else return(-1);

	meansqdif = sqrt(meansqdif/total); 
	return(meansqdif);
}
