/***********************************************************************
 - Create a smoothed (box-car averaged) 2-dimensional firing rate array
 - only modifies array rate[xbin][ybin]
 - will assign smoothed value to unvisited bins, 
		but won't use them for calculating averages
 ************************************************************************/
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

void hux_error(char message[]);

void hux_smooth_map (
	int *spikes,	/* spike-count array (spikes[xbin][ybin]) */
	float *dwell,	/* dwell-time array (dwelltime[xbin][ybin]) */
	float *rate,	/* firing rate array to be adjusted (rate[xbin][ybin]) */
	int bintot,		/* width or height of array - total bins actually = bintot*bintot */
	int smooth,		/* smoothing factor to be applied (bin-radius not including central bin) */
	int smoothtype, /* averages rates from adjacent bins [1] or add adjacent bin spikes and dwell times first [2] */ 
	int invalid)	/* invalid rate value to be ignored (indicating unvisited bins) */
{ 
int x,y,p,xbin,ybin,xstart,xend,ystart,yend;
int winwidth=smooth+smooth+1, Nmap=bintot*bintot, Nwin=winwidth*winwidth;
float temp1,temp2,temp3,std=(float)smooth,*conv,*srate;

if(smoothtype==1) { /* averages rates from adjacent bins */
	for(ybin=0;ybin<bintot;ybin++) {
		ystart=ybin-smooth; yend=ybin+smooth;
		for(xbin=0;xbin<bintot;xbin++) {
			xstart=xbin-smooth; xend=xbin+smooth; temp1=temp2=0.00;
			for(y=ystart;y<=yend;y++) for(x=xstart;x<=xend;x++) {
					if(y<0||y>=bintot||x<0||x>=bintot) continue; /* do not use unvisited bins to smooth */
					p = y*bintot+x;
					if(dwell[p]>0.0) {temp1 += spikes[p]/dwell[p]; temp2 ++;}
			}
			if(temp2>0) rate[ybin*bintot+xbin]=(temp1/temp2);
}}}

if(smoothtype==2) { /* add adjacent bin spikes and dwell times first, then take average */ 
	for(ybin=0;ybin<bintot;ybin++) {
		ystart=ybin-smooth; yend=ybin+smooth;
		for(xbin=0;xbin<bintot;xbin++) {
			xstart=xbin-smooth; xend=xbin+smooth; temp1=temp2=0.00;
			for(y=ystart;y<=yend;y++) for(x=xstart;x<=xend;x++) {
					if(y<0||y>=bintot||x<0||x>=bintot) continue; /* do not use unvisited bins to smooth */
					p = y*bintot+x;
					if(dwell[p]>0.0) {temp1 += spikes[p]; temp2 += dwell[p];}
			}
			if(temp2>0) rate[ybin*bintot+xbin]=(temp1/temp2);
}}}

if(smoothtype==3) {
	/* initialise temporary arrays */
	conv = (float *) malloc((Nwin+1)*sizeof(float)); if(conv==NULL) hux_error("smooth_map_gaussian: insufficient memory");
	srate = (float *) malloc((Nmap+1)*sizeof(float)); if(srate==NULL) hux_error("smooth_map: insufficient memory");
	// create gaussian kernal
	temp3=0.0;
	for(y=-smooth;y<=smooth;y++) {	
		for(x=-smooth;x<=smooth;x++) {
			p=((y+smooth)*winwidth)+(x+smooth);
            conv[p] = (float) exp(-y*y/std/2.)*exp(-x*x/std/2.);
			temp3+=conv[p];
	}}
	for(p=0;p<Nwin;p++) conv[p]/=temp3; /* normalise so sum of weights = 1.0 */

	for(ybin=0;ybin<bintot;ybin++) {
		ystart= ybin-smooth; yend= ybin+smooth;
		for(xbin=0;xbin<bintot;xbin++) {
			xstart= xbin-smooth; xend = xbin+smooth; 
			temp2 = temp3 = 0.0;
			for(y=ystart;y<=yend;y++) for(x=xstart;x<=xend;x++) {
					if(y<0||y>=bintot||x<0||x>=bintot) continue; /* do not use unvisited bins to smooth */
					p = y*bintot+x;
					if(rate[p]!=invalid) {
						temp1 = conv[ ((y-ybin)+smooth)*winwidth + ((x-xbin)+smooth) ];
						temp2 += temp1 * rate[p]; /* add up weighted rates */
						temp3 += temp1;	/* sum weights from visited bins - this should add to 1, ideally*/
			}}
			if(temp3>0.0) srate[ybin*bintot+xbin] = temp2/temp3; /* divide weighted mean by sum of weights - corrects for missing bins */
			else srate[ybin*bintot+xbin] = rate[ybin*bintot+xbin]; /* otherwise, rate is  */
	}}
	for(p=0;p<Nmap;p++) rate[p]=srate[p]; /* copy smoothed array to original rate map */
	free(conv); free(srate);
}
return;
}

