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
	char *palette : name of the palette (grey,rainbow,viridis,plasma,magma,inferno)

RETURN VALUE:
	0 on success, -1 on error (invalid palette)
	result array will hold statistics
	message array will hold explanatory text (if any)

SAMPLE CALL:
	#define N 100
	int ii; float red[N],green[N],blue[N];
	xf_palette3(red,green,blue,N,"rainbow");
	for(ii=0;ii<N;ii++)printf("%g\t%g\t%g\n",red[ii]),green[ii],blue[ii]);

TO BUILD EXTRA PALLETES USING R
	1. in R, print a list of 3 hex values spanning the range
		library(viridis)
		viridis(3,option="inferno")
	2. copy this into a list, removing internal quotes.. eg.
		list='#440154FF #443A83FF #31688EFF #21908CFF #35B779FF #8FD744FF'
	3. build a code snippet
		for i in $list ; do xs-hex2rgb $i ; done | awk 'BEGIN{n=-1} {for(i=1;i<=NF;i++) { printf("ct[%d]=%s; ",++n,$i); } printf("\n")}'

<TAGS> programming LDAS </TAGS>
*/

#include <stdlib.h>
#include <math.h>
#include <string.h>

long xf_interp3_f(float *data, long ndata);

int xf_palette3(float *red, float *green, float *blue, long nn, char *palette) {

	long ii,jj,kk,rgbstart[3];
	float ct[9];

	/* DEFINE THE TRIPLETS FOR THE CHOSEN PALETTE */
	if(strcmp(palette,"grey")==0) {
		ct[0]= 0.2; ct[1]= 0.2; ct[2]= 0.2; // very dark grey
		ct[3]= 0.5; ct[4]= 0.5; ct[5]= 0.5; // medium
		ct[6]= 0.8; ct[7]= 0.8; ct[8]= 0.8; // very light grey
	}
	if(strcmp(palette,"black2grey")==0) {
		ct[0]= 0.00; ct[1]= 0.00; ct[2]= 0.00; // black
		ct[3]= 0.40; ct[4]= 0.40; ct[5]= 0.40; // medium
		ct[6]= 0.80; ct[7]= 0.80; ct[8]= 0.80; // very light grey
	}
	else if(strcmp(palette,"rainbow")==0) {
		ct[0]= 0.0; ct[1]= 0.0; ct[2]= 1.0; // deep blue
		ct[3]= 0.0; ct[4]= 1.0; ct[5]= 0.0; // cyan
		ct[6]= 1.0; ct[7]= 1.0; ct[8]= 0.0; // red
	}
	else if(strcmp(palette,"viridis")==0) {
		ct[0]= 0.27; ct[1]= 0.00; ct[2]= 0.33; // blue
		ct[3]= 0.13; ct[4]= 0.56; ct[5]= 0.55;  // bluish-green
		ct[6]= 0.99; ct[7]= 0.91; ct[8]= 0.15; // yellow
	}
	else if(strcmp(palette,"plasma")==0) {
		ct[0]= 0.05; ct[1]= 0.03; ct[2]= 0.53; // blue
		ct[3]= 0.80; ct[4]= 0.27; ct[5]= 0.47; // purple
		ct[6]= 0.94; ct[7]= 0.98; ct[8]= 0.13; // yellow
	}
	else if(strcmp(palette,"magma")==0) {
		ct[0]= 0.00; ct[1]= 0.00; ct[2]= 0.02;
		ct[3]= 0.71; ct[4]= 0.21; ct[5]= 0.47;
		ct[6]= 0.99; ct[7]= 0.99; ct[8]= 0.75;
	}
	else if(strcmp(palette,"inferno")==0) {
		ct[0]= 0.00; ct[1]= 0.00; ct[2]= 0.02;
		ct[3]= 0.73; ct[4]= 0.22; ct[5]= 0.33;
		ct[6]= 0.99; ct[7]= 1.00; ct[8]= 0.64;
	}

	else return(-1);

	/* DEFINE THE START-POINTS FOR EACH COLOUR */
	rgbstart[0]= (long)(0.0);
	rgbstart[1]= (long)(nn*.50); // 50%
	rgbstart[2]= (long)(nn-1.0);

	/* INITIALIZE THE COLOUR ARRAYS TO NAN, SO THE INTERPOLATOR KNOWS WHERE TO INTERPOLATE */
	for(ii=0;ii<nn;ii++) red[ii]= green[ii]= blue[ii]= NAN;

	/* INSERT THE ANCHOR-COLOUR-TRIPLETS INTO THE RED GREEN AND BLUE ARRAYS */
	for(ii=0;ii<3;ii++) {
		kk= ii*3;
		red[rgbstart[ii]]   = ct[kk];
		green[rgbstart[ii]] = ct[kk+1];
		blue[rgbstart[ii]]  = ct[kk+2];
	}

	/* APPLY INTERPOLATION */
	xf_interp3_f(red,nn);
	xf_interp3_f(green,nn);
	xf_interp3_f(blue,nn);

	/* RETURN ALL CLEAR */
	return(0);
}
