/*
DESCRIPTION:
	Generate an RGB colour-palette (0-1 range)

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
	xf_palette7(red,green,blue,N,"rainbow");
	for(ii=0;ii<N;ii++)printf("%g\t%g\t%g\n",red[ii]),green[ii],blue[ii]);

TO BUILD EXTRA PALLETES USING R
	1. in R, print a list of 7 hex values spanning the range
		library(viridis)
		viridis(7,option="inferno")
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

int xf_palette7(float *red, float *green, float *blue, long nn, char *palette) {

	long ii,jj,kk,rgbstart[7];
	float ct[21];

	/* DEFINE THE TRIPLETS FOR THE CHOSEN PALETTE */
	if(strcmp(palette,"grey")==0) {
		ct[0]=  0.2; ct[1]=  0.2; ct[2]=  0.2; // black
		ct[3]=  0.3; ct[4]=  0.3; ct[5]=  0.3;
		ct[6]=  0.4; ct[7]=  0.4; ct[8]=  0.4;
		ct[9]=  0.5; ct[10]= 0.5; ct[11]= 0.5; // medium
		ct[12]= 0.6; ct[13]= 0.6; ct[14]= 0.6;
		ct[15]= 0.7; ct[16]= 0.7; ct[17]= 0.7;
		ct[18]= 0.8; ct[19]= 0.8; ct[20]= 0.8; // very light grey
	}
	else if(strcmp(palette,"rainbow")==0) {
		ct[0]=  0.0; ct[1]=  0.0; ct[2]=  0.3; // deep blue
		ct[3]=  0.0; ct[4]=  0.3; ct[5]=  1.0; // blue
		ct[6]=  0.2; ct[7]=  0.8; ct[8]=  0.8; // cyan
		ct[9]=  0.5; ct[10]= 1.0; ct[11]= 0.0; // green
		ct[12]= 1.0; ct[13]= 1.0; ct[14]= 0.0; // yellow
		ct[15]= 1.0; ct[16]= 0.3; ct[17]= 0.0; // orange
		ct[18]= 0.7; ct[19]= 0.0; ct[20]= 0.0; // red
	}
	else if(strcmp(palette,"viridis")==0) {
		ct[0]= 0.27; ct[1]= 0.00; ct[2]= 0.33; // blue
		ct[3]= 0.27; ct[4]= 0.23; ct[5]= 0.51;
		ct[6]= 0.19; ct[7]= 0.41; ct[8]= 0.56;
		ct[9]= 0.13; ct[10]=0.56; ct[11]=0.55;  // bluish-green
		ct[12]=0.21; ct[13]=0.72; ct[14]=0.47;
		ct[15]=0.56; ct[16]=0.84; ct[17]=0.27;
		ct[18]=0.99; ct[19]=0.91; ct[20]=0.15; // yellow
	}
	else if(strcmp(palette,"plasma")==0) {
		ct[0]=  0.05; ct[1]=  0.03; ct[2]=  0.53; // blue
		ct[3]=  0.36; ct[4]=  0.00; ct[5]=  0.65; //
		ct[6]=  0.61; ct[7]=  0.09; ct[8]=  0.62; //
		ct[9]=  0.80; ct[10]= 0.27; ct[11]= 0.47; // purple
		ct[12]= 0.93; ct[13]= 0.47; ct[14]= 0.33; //
		ct[15]= 0.99; ct[16]= 0.70; ct[17]= 0.18; //
		ct[18]= 0.94; ct[19]= 0.98; ct[20]= 0.13; // yellow
	}
	else if(strcmp(palette,"magma")==0) {
		ct[0]=0.00; ct[1]=0.00; ct[2]=0.02;
		ct[3]=0.18; ct[4]=0.07; ct[5]=0.38;
		ct[6]=0.45; ct[7]=0.12; ct[8]=0.51;
		ct[9]=0.71; ct[10]=0.21; ct[11]=0.47;
		ct[12]=0.95; ct[13]=0.38; ct[14]=0.36;
		ct[15]=1.00; ct[16]=0.69; ct[17]=0.47;
		ct[18]=0.99; ct[19]=0.99; ct[20]=0.75;
	}
	else if(strcmp(palette,"inferno")==0) {
		ct[0]=0.00; ct[1]=0.00; ct[2]=0.02;
		ct[3]=0.20; ct[4]=0.04; ct[5]=0.37;
		ct[6]=0.47; ct[7]=0.11; ct[8]=0.43;
		ct[9]=0.73; ct[10]=0.22; ct[11]=0.33;
		ct[12]=0.93; ct[13]=0.41; ct[14]=0.15;
		ct[15]=0.99; ct[16]=0.71; ct[17]=0.10;
		ct[18]=0.99; ct[19]=1.00; ct[20]=0.64;
	}

	else return(-1);

	/* DEFINE THE START-POINTS FOR EACH COLOUR */
	rgbstart[0]= (long)(nn*.00);
	rgbstart[1]= (long)(nn*.17);
	rgbstart[2]= (long)(nn*.34);
	rgbstart[3]= (long)(nn*.51); // 50%
	rgbstart[4]= (long)(nn*.68);
	rgbstart[5]= (long)(nn*.85);
	rgbstart[6]= (long)(nn-1);

	/* INITIALIZE THE COLOUR ARRAYS TO NAN, SO THE INTERPOLATOR KNOWS WHERE TO INTERPOLATE */
	for(ii=0;ii<nn;ii++) red[ii]= green[ii]= blue[ii]= NAN;

	/* INSERT THE ANCHOR-COLOUR-TRIPLETS INTO THE RED GREEN AND BLUE ARRAYS */
	for(ii=0;ii<7;ii++) {
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
