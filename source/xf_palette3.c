/*
DESCRIPTION:
	Generate a 3-anchor RGB colour-palette (0-1 range)

DEPENDENCIES:
	xf_interp3_f

ARGUMENTS:
	float *red    : pre-allocated array for red values (0-1)
	float *green  : as above for green
	float *blue   : as above for blue
	long nn       : size of the array to fill (number of colours)
	int *anchors  : array of 3 RGB triplets = 9 numbers, 0-1 (beginning, middle, end of palette)
	int rev       : reverse the order of palette colours (1=YES,0-NO)

RETURN VALUE:
	0 on success, -1 on error (invalid palette)
	result array will hold statistics
	message array will hold explanatory text (if any)

SAMPLE CALL:
	#define N 100
	int ii; float red[N],green[N],blue[N],anchors[9]= {1,0,0, 0,1,0, 0,0,1};
	xf_palette3(red,green,blue,N,"rainbow");
	for(ii=0;ii<N;ii++)printf("%g\t%g\t%g\n",red[ii]),green[ii],blue[ii],0);

<TAGS> programming LDAS </TAGS>
*/

#include <stdlib.h>
#include <math.h>
#include <string.h>

long xf_interp3_f(float *data, long ndata);

int xf_palette3(float *red, float *green, float *blue, long nn, int *anchors, int rev) {

	long ii,jj,kk,mm,rgbstart[3];
	float ct[21];

	/* DEFINE THE START-POINTS FOR EACH COLOUR */
	rgbstart[0]= (long)(nn*.00);
	rgbstart[1]= (long)(nn*.51); // 50%
	rgbstart[2]= (long)(nn-1);

	/* INITIALIZE THE COLOUR ARRAYS TO NAN, SO THE INTERPOLATOR KNOWS WHERE TO INTERPOLATE */
	for(ii=0;ii<nn;ii++) red[ii]= green[ii]= blue[ii]= NAN;

	/* INSERT THE ANCHOR-COLOUR-TRIPLETS INTO THE RED GREEN AND BLUE ARRAYS */
	for(ii=0;ii<3;ii++) {
		kk= ii*3;
		red[rgbstart[ii]]   = anchors[kk];
		green[rgbstart[ii]] = anchors[kk+1];
		blue[rgbstart[ii]]  = anchors[kk+2];
	}

	/* APPLY INTERPOLATION */
	xf_interp3_f(red,nn);
	xf_interp3_f(green,nn);
	xf_interp3_f(blue,nn);

	/* REVERSE THE PALLET IF REQUESTED */
	if(rev==1) {
		mm= nn-1; // max value to speed up calculations
		float *swap= malloc(nn*sizeof(*swap));
		if(swap==NULL) return(-1);
		for(ii=0;ii<nn;ii++) swap[ii]= red[mm-ii];
		for(ii=0;ii<nn;ii++) red[ii]= swap[ii];
		for(ii=0;ii<nn;ii++) swap[ii]= green[mm-ii];
		for(ii=0;ii<nn;ii++) green[ii]= swap[ii];
		for(ii=0;ii<nn;ii++) swap[ii]= blue[mm-ii];
		for(ii=0;ii<nn;ii++) blue[ii]= swap[ii];
		free(swap);
	}
	/* RETURN ALL CLEAR */
	return(0);
}
