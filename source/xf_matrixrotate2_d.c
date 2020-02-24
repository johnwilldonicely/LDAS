/*
<TAGS>dt.matrix</TAGS>

DESCRIPTION:
	Rotate a 1-dimensional array of numbers  meant to be interpreted as a 2-dimentional matrix
	- unlike xf_matrixrotate1_d, makes an internal copy of data and modifies the original
	- therefore, safer for memory management and nested functions, but has a heavier computational load
USES:
	- image or map rotation
	- matrix algebra

DEPENDENCIES:
	None

ARGUMENTS:
	double *data1 : input, pointer to array of numbers representing the original matrix
	long *nx1     : input/output - width of the matrix, to be modified depending on rotation
	long *ny1     : input/output- height of the matrix, to be modified depending on rotation
	int r         : input - rotation (90,180,270,-90,-180,-270)

RETURN VALUE:
	0 on success
	-1 for invalid arguments
	-2 for memory allocation error
	NOTE: nx1 and ny1 will be also modified according to the rotation
*/

#include <stdio.h>
#include <stdlib.h>

int xf_matrixrotate2_d(double *data1, long *width, long *height, int r) {

	long x1,y1,xmax1,ymax1,index1;
	long ii,jj,kk,nn,x2,y2,nx1=*width,ny1=*height,nx2,ny2,index2;
	double *data2=NULL;

	/* MAKE SURE ARRAY CONTAINS ELEMENTS */
	if(nx1<1||nx1<1) return(-1);

	/* DETERMINE THE HEIGHT (ny2) AND WIDTH (nx2) OF THE ROTATED MATRIX */
	if(r==90 || r==270 || r==-90 || r==-270) { nx2=ny1; ny2=nx1; }
	else if(r==180||r==-180) { nx2=nx1; ny2=ny1; }
	/* IF ROTATION IS NOT A MULTIPLE OF 90, CHANGE NOTHING */
	else return(-1);

	/* ALLOCATE MEMORY FOR THE NEW ROTATED MATRIX */
	nn= nx2*ny2;
	data2= realloc(data2,nn*sizeof(*data2));
	if(data2==NULL) return(-2);

	/* CALCULATE HIGHEST VALUE OF ROW & COLUMN - FOR SPEED IN THE NEXT STEP */
	xmax1=nx1-1; ymax1=ny1-1;

	/* TRANSFER VALUES FROM ORIGINAL MATRIX TO NEW ROTATED MATRIX */
	if(r==90||r==-270) {
		for(y1=0;y1<ny1;y1++) {
			jj=y1*nx1; kk=ymax1-y1;
			for(x1=0;x1<nx1;x1++) { data2[x1*nx2+kk]= data1[jj+x1]; }
	}}
	if(r==180||r==-180) {
		for(y1=0;y1<ny1;y1++) {
			jj=y1*nx1; kk=ymax1-y1;
			for(x1=0;x1<nx1;x1++) { data2[kk*nx2+(xmax1-x1)]= data1[jj+x1]; }
	}}
	if(r==-90||r==270) {
		for(y1=0;y1<ny1;y1++) {
			jj=y1*nx1;
			for(x1=0;x1<nx1;x1++) { data2[(xmax1-x1)*nx2+y1]= data1[jj+x1]; }
	}}

	/* COPY BACK TO ORIGINAL */
	for(ii=0;ii<nn;ii++) data1[ii]= data2[ii];

	/* UPDATE VARIABLES FOR WIDTH & HEIGHT OF MATRIX */
	*width=nx2; *height=ny2;

	/* CLEANUP AND RETURN */
	free(data2);
	return(0);
}
