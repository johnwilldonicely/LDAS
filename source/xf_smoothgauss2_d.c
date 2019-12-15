/***********************************************************************
<TAGS>signal_processing filter</TAGS>
DESCRIPTION:
	Uses a Gaussian kernal to smooth a 2-dimensional array of data
	Modifies array data[xbin][ybin]

8 March 2013:
	new name  xf_smoothgauss2_d
	no longer accepts an "invalid" argument
	assumes NAN or INF are invalid instead

6 November 2012: fix so that xsmooth or ysmooth can now be zero

************************************************************************/
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

int xf_smoothgauss2_d(double *data,int xbintot,int ybintot,int xsmooth,int ysmooth) {

	int x,y,p1,p2,xbin,ybin,xstart,xend,ystart,yend;
	int xwinwidth=xsmooth+xsmooth+1;
	int ywinwidth=ysmooth+ysmooth+1;
	int Nmap=xbintot*ybintot, Nwin=xwinwidth*ywinwidth;
	double temp1,temp2,temp3,xstd,ystd,*conv,*sdata;

	xstd=(double)xsmooth*2.0;
	ystd=(double)ysmooth*2.0;

	/* INITIALISE TEMPORARY ARRAYS */
	conv= (double *) malloc((Nwin+1)*sizeof(double)); if(conv==NULL) return(-1);
	sdata= (double *) malloc((Nmap+1)*sizeof(double)); if(sdata==NULL) return(-1);

	/* CREATE GAUSSIAN KERNAL */
	temp3=0.0;
	for(y=-ysmooth;y<=ysmooth;y++) {
		for(x=-xsmooth;x<=xsmooth;x++) {
			p2=((y+ysmooth)*xwinwidth)+(x+xsmooth);
			if(ysmooth<1) temp1=1; else temp1=exp(-y*y/ystd);
			if(xsmooth<1) temp2=1; else temp1=exp(-x*x/xstd);
			conv[p2] = temp1*temp2;
			temp3+=conv[p2];
	}}

	/* NORMALISE THE KERNAL SO SUM OF WEIGHTS = 1.0 */
	for(p2=0;p2<Nwin;p2++) conv[p2]/=temp3;

	/* SMOOTH THE 2-DIMENSIONAL ARRAY */
	for(ybin=0;ybin<ybintot;ybin++) {
		ystart= ybin-ysmooth; if(ystart<0) ystart=0;
		yend= ybin+ysmooth; if(yend>=ybintot) yend=(ybintot-1);

		for(xbin=0;xbin<xbintot;xbin++) {
			xstart= xbin-xsmooth; if(xstart<0) xstart=0;
			xend = xbin+xsmooth; if(xend>=xbintot) xend=(xbintot-1);
			temp2 = temp3 = 0.0;

			/* calculate the weighted average for the block of bins surrounding [ybin,xbin]*/
			for(y=ystart;y<=yend;y++) { /* the y-range of surrounding bins */
				for(x=xstart;x<=xend;x++) { /* the x-range of surrounding bins */
					p1 = y*xbintot+x; /* pointer to the datum at this x-y coordinate*/
					p2= ((y-ybin)+ysmooth) * xwinwidth + ((x-xbin)+xsmooth); /* pointer to the correct kernel element */
					if(isfinite(data[p1])) {
						temp1 = conv[p2];
						temp2 += temp1 * data[p1]; /* add up weighted data points in the window */
						temp3 += temp1;	/* sum weights from visited bins - this should add to 1, ideally*/
			}}}
			if(temp3>0.0) sdata[ybin*xbintot+xbin] = temp2/temp3; /* divide weighted mean by sum of weights - corrects for missing bins */
			else          sdata[ybin*xbintot+xbin] = data[ybin*xbintot+xbin]; /* otherwise, data is same as original */
	}}

	/* copy smoothed array to original data map */
	for(x=0;x<Nmap;x++) data[x]=sdata[x];

	free(conv);
	free(sdata);
	return(0);
}
