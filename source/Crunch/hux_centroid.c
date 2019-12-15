/*
Calculate cetroid of 2-dimensional array, only for bins within predefined firing field
*/

#include<stdio.h>
#include<stdlib.h>

void hux_error(char message[]);

void hux_centroid (
					 float *maprate,	/* firing rate array */
					 int *mapfield,	/* field array (0=out of field, 1=in, 2=peak) */
					 int bintot,	/* width of array (total elements = bintot*bintot) */
					 float mapbinsize,	/* size of each ratemap bin, in cm (e.g. "2" = 2x2 cm) */
					 float *result	/* array of float to hold outputs - should point to 32-element array */
					 )

{
	int p1,xbin,ybin;
	double centroid_x=0.0, centroid_y=0.0,tot=0.0;

	if(bintot<=0) hux_error("hux_centroid: inappropriate specification of number of bins");

	for(ybin=0;ybin<bintot;ybin++) {
		for(xbin=0;xbin<bintot;xbin++)	{
			p1=ybin*bintot+xbin;
			if(mapfield[p1]<1)continue;

			centroid_x += mapbinsize*(xbin+0.5)*maprate[p1];
			centroid_y += mapbinsize*(ybin+0.5)*maprate[p1];
			tot += maprate[p1];
		}
	}
	if(tot<=0) hux_error("hux_centroid: in-field firing rates all appear to be zero!");
	result[1] = (float) (centroid_x/tot);
	result[2] = (float) (centroid_y/tot);
	return;
}
