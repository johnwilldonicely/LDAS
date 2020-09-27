/*
<TAGS>dt.matrix</TAGS>

DESCRIPTION:
	Flip (x or y) a 1-dimensional array meant to be interpreted as a 2-dimensional matrix
	- makes an internal copy of data, modifies it, and copies it back to the original
	- therefore, safer for memory management and nested functions, but has a heavier computational load

	- Example - x-flip
		1 2 3                3 2 1
		4 5 6  --becomes-->  6 5 4
		7 8 9                9 8 7

USES:
	- image or map rotation
	- matrix algebra

DEPENDENCIES:
	None

ARGUMENTS:
	long *data1   : input/output - pointer to array of numbers representing the original matrix
	long *nx1     : input        - width of the matrix
	long *ny1     : input        - height of the matrix
	int setflip   : input        - flip axis - 1=x, 2=y

RETURN VALUE:
	0 on success
	-1 for invalid size of input
	-2 for invalid arguments
	-3 for memory allocation error
*/

#include <stdio.h>
#include <stdlib.h>

int xf_matrixflip2_l(long *data1, long *width, long *height, int setflip) {

	long in1,in2,index1;
	long ii,jj,kk,nn,nx1=*width,ny1=*height;
	long *data2=NULL;

	/* MAKE SURE ARRAY CONTAINS ELEMENTS */
	if(nx1<1||ny1<1) return(-1);
	/* DETERMINE THE HEIGHT (ny2) AND WIDTH (nx2) OF THE ROTATED MATRIX */
	if(setflip!=1 && setflip!=2) return(-2);

	/* ALLOCATE MEMORY FOR THE NEW ROTATED MATRIX */
	nn= nx1*ny1;
	data2= realloc(data2,nn*sizeof(*data2));
	if(data2==NULL) return(-3);

	/* TRANSFER VALUES FROM ORIGINAL MATRIX TO NEW FLIPPED VERSION */
	if(setflip==1) { // x-flip
		kk=0;
		ii= nx1-1;
		for(ii=ii;ii>=0;ii--) {
			for(jj=0;jj<ny1;jj++) {
				in1= jj*nx1+ii;
				in2= jj*nx1+kk;
				data2[in2]= data1[in1];
			}
		kk++;
		}
	}
	if(setflip==2) { // y-flip
		kk=0;
		ii= ny1-1;
		for(ii=ii;ii>=0;ii--) {
			for(jj=0;jj<nx1;jj++) {
				in1= ii*nx1+jj;
				in2= kk*nx1+jj;
				data2[in2]= data1[in1];
			}
			kk++;
		}
	}

	/* COPY BACK TO ORIGINAL */
	for(ii=0;ii<nn;ii++) data1[ii]= data2[ii];

	/* CLEANUP AND RETURN */
	free(data2);
	return(0);
}
