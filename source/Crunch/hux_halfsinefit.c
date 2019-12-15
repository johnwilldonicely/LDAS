/*******************************************************************************
- fit half sinusoid to a series of data
- as above, but assumes only negative portion of cycle is fit
********************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

float hux_halfsinefit(float *data, int total) {
	int z;
	float a,ymin,ymax,yrange,zoffset,xadjust,meansqdif;

	ymin = ymax = data[0];
	for(z=0;z<total;z++) {if(data[z]<ymin) ymin = data[z]; if(data[z]>ymax) ymax = data[z];}

	xadjust = 3.14159265358979323846/(total - 1.0);
	yrange = ymax - ymin;
	zoffset = 0.00 - ymax;	

	meansqdif=0;
	for(z=0;z<total;z++) {
		a = ((data[z]+zoffset)/ yrange) + (sin(z*xadjust)); /* normalized original data */
		meansqdif += a*a;
	}
	meansqdif = sqrt(meansqdif/total); 
	return(meansqdif);
}

