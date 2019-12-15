/*
DESCRIPTION:
	Remove noise from a spectral-matrix (time x freq)
 	- noise=  single-timepoint decrease/increase that span many adjacent frequencies
 	- timeseries for each freq. converted to Z-scores for thresholding
 	- outputs a modified matrix with noise-timepoints set to NAN
	- percentages for positive and negative threhold crossings are calculated separately

USES:
	cleaning up spectral matrices

DEPENDENCIES:
	xf_matrixrotate2_d

ARGUMENTS:
	double *matrix1 : input holding matrix
	long width      : number of columns (time)
	long height     : number of rows (frequencies)
	double setclip  : max permissable value for z-score calculation (-1=noclip) - avoids skewing of the z-scores by extreme outliers
	double setz     : Z-score threshold for noise at each freq (typically 1)
	int setsign     : detect negative (-1) or positive (+1) threshold crossings, or both (0)
	double setper   : % of adjacent freq > sd needed to invalidate timepoint (typically 25)
	int setrotate   : specify if input is rotated 90-degrees so row=time (0=NO 1=YES)
	char *message   : pre-allocated array to hold error message

RETURN VALUE:
	number of invalidated frequencies (columns) on success, -1 on error
	char array will hold message (if any)

SAMPLE CALL:
	noisecount= xf_spectdenoise1_d(matrix,nn,width,height,-1,3,25,0,0,message);
	if(noisecount==-1) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }

<TAGS>matrix,signal_processing,spectra</TAGS>
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>

int xf_matrixrotate2_d(double *data1, long *width, long *height, int r);

long xf_spectdenoise1_d(double *matrix1,long width,long height,double setclip,double setz,int setsign,double setper,int setrotate,char *message) {

	char *thisfunc="xf_spectdenoise1_d\0",*matrix2=NULL;
	long ii,jj,kk,nn,mm,noisecount=0;
	double aa,bb,sum=0.0,mean=NAN,ss=0.0,sd=0.0,zpos,zneg;


	nn= width*height;
	/* CHECK VALIDITY OF DATA */
	if(nn==0) { sprintf(message,"%s [ERROR]: empty matrix",thisfunc); return(-1); }
	for(ii=0;ii<nn;ii++) if(isfinite(matrix1[ii])) break;
	if(ii>=nn) { sprintf(message,"%s [ERROR]: no valid data in matrix",thisfunc); return(-1); }

	/* DEFINE CLIP */
	if(setclip<0.0) setclip= FLT_MAX;

	/* SET NEGATIVE AND POSITIVE THRESHOLDS */
	zpos= fabs(setz);
	zneg= zpos*-1.0;

	/* CHECK VALIDITY OF ARGUMENTS */
	if(setsign<-1||setsign>1) {sprintf(message,"%s [ERROR]: invalid setsign [%d] must be -1, 0 or 1\n\n",thisfunc,setsign);return(-1);}
	if(setrotate!=0&&setrotate!=1) {sprintf(message,"%s [ERROR]: invalid setrotate [%d] must be 0 or 1\n\n",thisfunc,setrotate);return(-1);}
	if(setz==0.0) {sprintf(message,"%s [ERROR]: invalid setz [%g], cannot be zero\n\n",thisfunc,setz);return(-1);}
	if(setper<=0.0) {sprintf(message,"%s [ERROR]: invalid setper [%g], must be >0\n\n",thisfunc,setper);return(-1);}

	/* ROTATE IF REQUIRED - this will be reversed later */
	if(setrotate!=0) ii= xf_matrixrotate2_d(matrix1,&width,&height,-90);
	if(ii==-1) {sprintf(message,"%s/xf_matrixrotate2_d [ERROR]: insufficient memory\n\n",thisfunc);return(-1);}

	/* ALLOCATE MEMORY FOR THE TEMPORARY DATA AND THE FREQUENCY-COUNTER*/
	matrix2= malloc(nn*sizeof(*matrix2));
	if(matrix2==NULL) {sprintf(message,"%s [ERROR]: insufficient memory\n\n",thisfunc);return(-1);}

	/* NORMALIZE EACH ROW (TIME-SERIES FOR A SINGLE FREQUENCY) */
	for(ii=0;ii<height;ii++) {
		mm= 0;
		sum= ss= 0.0;
		kk= ii*width;
		/* calculate the mean and standard deviation - apply clipping */
		for(jj=mm=0;jj<width;jj++) {
			aa= matrix1[kk+jj];
			if(aa>setclip) aa=setclip;
			if(isfinite(aa)) {sum+=aa; ss+=aa*aa; mm++;}
		}
		if(mm==0) continue;
		else if(mm>1) sd= sqrt((double)((mm*ss-(sum*sum))/(mm*(mm-1))));
		else sd= 1.0;
		mean= sum/(double)mm;
		/* test if z-score passes threshold - if so, increment freqcount for this column */
		for(jj=0;jj<width;jj++) {
			aa= matrix1[kk+jj];
			matrix2[kk+jj]= 0;
			if(isfinite(aa)) {
				bb= (aa-mean)/sd;
				if(bb>=0) { if(bb>=zpos) matrix2[kk+jj]=  1; }
				else      { if(bb<=zneg) matrix2[kk+jj]= -1; }
	}}}

	/* INVALIDATE COLUMNS (TIMES) WHEN THE PERCENTAGE-THRESHOLD IS EXCEEDED */
	if(setsign==0||setsign==1) {
		for(mm=jj=0;jj<width;jj++) {
			mm=0; for(ii=0;ii<height;ii++) if(matrix2[ii*width+jj]==1) mm++;
			aa= 100.0*(double)mm/(double)height;
			if(aa>setper) {	noisecount++; for(ii=0;ii<height;ii++) matrix1[ii*width+jj]=NAN;	}
	}}
	if(setsign==0||setsign==-1) {
		for(mm=jj=0;jj<width;jj++) {
			for(ii=mm=0;ii<height;ii++) if(matrix2[ii*width+jj]==-1) mm++;
			aa= 100.0*(double)mm/(double)height;
			if(aa>setper) {	noisecount++; for(ii=0;ii<height;ii++) matrix1[ii*width+jj]=NAN;	}
	}}

	/* ROTATE IF REQUIRED - restores original orientation */
	if(setrotate!=0) ii= xf_matrixrotate2_d(matrix1,&width,&height,90);
	if(ii==-1) {sprintf(message,"%s/xf_matrixrotate2_d [ERROR]: insufficient memory\n\n",thisfunc);return(-1);}

	/* CLEANUP AND RETURN */
	if(matrix2!=NULL) free(matrix2);
	return(noisecount);
}
