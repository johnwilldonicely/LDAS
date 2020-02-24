/*
<TAGS>stats dt.matrix</TAGS>
DESCRIPTION:
	Calculate spatial-information-content for place-fields
	- uses the adaptive binning methods described in Langston et al. 2010 (adapted from Skaggs & McNaughton,1998)
		- expand radius of binning until:  radius * dwell * sqrt(events) >= alpha
		- oddly, in the literture this is expressed as:  radius >= alpha / ( dwell * sqrt(events) )
		- alpha is a constant, which varies from publication (10^3 for Skaggs, 10^5 for Langston & O'Keefe)

	Pi = probability of being n bin i, = dwell (samples) in bin i divided by total dwell samples
	Ri = firing rate in bin i, = event/dwell
	R = mean firing rate = (event total) / (dwell total)

	- information content (how well does cell firing predict location - high info = small fields)
		= SUM {Pi*(Ri/R) log2*(Ri/R)}
		NOTE: log2(x) = log(x)/log(2)

USES:

DEPENDENCIES: none

ARGUMENTS:
	double *dwell       : position-sample-count array
	double *event       : event-count array
	long width          : width of array
	long height         : height of array
	long radmax         : maximum radius for adaptive binning (-1= auto, 0= central bin only)
	long alpha          : constant used to limit adaptive binning (typically 10^3 to 10^6, depending on the original binning or smoothing of the data)
	long *result_l      : array to hold outputs - should point to 8-element array
	char *message       : pre-allocated array to hold error message

RETURN VALUE:
	- spatial information content, or NAN on error
	- char array will hold message (if any)
	- result array will hold statistics
		result_l[0]= total dwell samples
		result_l[1]= total events
		result_l[2]= total non-zero dwell samples
		result_l[3]= radius limit (radmax)
		result_l[4]= largest radius used in adaptive binning (radbig)

SAMPLE CALL:

*/

#include<stdio.h>
#include<stdlib.h>
#include<math.h>

double xf_placeinfo1_l(long *dwell1,long *event1,long width,long height,long radmax,long alpha,long *result_l, char *message) {

	char *thisfunc="xf_placeinfo1_l\0";
	long ii,jj,kk,ww,xx,yy,zz,n1,n2,xbin,ybin,radbig=0,dwelltot,eventtot,dwell2,event2;
	double aa,bb,ratemean,spatial_info=0.0;

	/* CHECK PARAMETERS */
	if(width<=0||height<=0) {sprintf(message,"%s [ERROR]: inappropriate width (%ld) and/or height (%ld)",thisfunc,width,height);return(NAN);}
	if(radmax>=width||radmax>=height) {sprintf(message,"%s [ERROR]: radmax (%ld) must be < width (%ld) and height (%ld)",thisfunc,radmax,width,height);return(NAN);}
	/* AUTO-DEFINE RADMAX */
	if(radmax<0) { if(width<=height) radmax= width-1; else radmax= height-1; }
	/* SIZE OF DWELL AND RATE ARRAYS */
	n1= width*height;
	/* SUM THE DWELL-SAMPLES AND EVENTS, CLACULATE MEAN EVENT-RATE */
	n2= dwelltot= eventtot= 0;
	for(ii=0;ii<n1;ii++) { if(dwell1[ii]>0) { dwelltot+= dwell1[ii]; eventtot+= event1[ii];n2++;} }
	ratemean= (double)eventtot/(double)dwelltot;

	/********************************************************************************/
	/* CALCULATE THE INFORMATION CONTENT */
	/********************************************************************************/
	for(ybin=0;ybin<height;ybin++) {
	for(xbin=0;xbin<width;xbin++)  {
		/* apply adaptive binning to calculating dwell- and event-sums for central bin */
		if(radmax>0) {
			for(radius=1;radius<=radmax;radius++) {
				/* set bounds for current radius */
				startx= xbin-radius; if(startx<0) startx=0;
				starty= ybin-radius; if(starty<0) starty=0;
				stopx=  xbin+radius; if(stopx>=width)  stopx= width-1;
				stopy=  ybin+radius; if(stopy>=height) stopy= height-1;
				/* sum the dwell-samples and events */
				dwell2= event2= 0;
				for(xx=startx;xx<=stopx;xx++) {
				for(yy=starty;yy<=stopy;yy++) {
					p2= yy*width+xx;
					dwell2+= dwell1[p2];
					event2+= event1[p2];
				}}
				if( radius*dwell2*sqrtl(event2) > alpha ) break;
			}
			/* determine the largest radius used */
			if(radius>radmax) radius--;
			if(radius>radbig) radbig=radius;
		}
		/* otherwise, for no binning, just use the central (current) bin */
		else {
			p1= ybin*width+xbin;
			dwell2= dwell1[p1];
			event2= event1[p];
		}
		/* accumulate spatial information content */
		aa= (double)dwell2/(double(dwelltot); /* occupancy probability for this bin */
		bb= ((double)event2/(double)dwell2) / ratemean; /* ratio of binrate to mean rate */
		spatial_info+= aa * bb * log2(bb);
	}}

	/* FILL THE RESULTS ARRAY */
	result_l[0]= dwelltot;
	result_l[1]= eventtot;
	result_l[2]= n2;
	result_l[3]= radmax;
	result_l[4]= radbig;

	/* RETURN THE SPATIAL INFORMATION CONTENT */
	return(spatial_info);
}
