/*
<TAGS>dt.matrix</TAGS>

DESCRIPTION:
	Transpose a 1-dimensional array meant to be interpreted as a 2-dimensional matrix
	- makes an internal copy of data, modifies it, and copies it back to the original
	- therefore, safer for memory management and nested functions, but has a heavier computational load

	- Example
		1 2 3                1 4 7
		4 5 6  --becomes-->  2 5 8
		7 8 9                3 6 9
		USES:

	- image or map transposition
	- matrix algebra

DEPENDENCIES:
	None

ARGUMENTS:
	long *data1   : input/output - pointer to array of numbers representing the original matrix
	long *nx1     : input/output - width of the matrix, will become the height
	long *ny1     : input/output - height of the matrix, will become the width

RETURN VALUE:
	0 on success
	-1 for invalid size of input
	-2 for invalid arguments
	-3 for memory allocation error
	NOTE: nx1 and ny1 will be swapped
*/

#include <stdio.h>
#include <stdlib.h>

int xf_matrixtrans2_l(long *data1, long *width, long *height) {

	long x1,y1,index1;
	long ii,jj,kk,mm,nn,nx1=*width,ny1=*height;
	long *data2=NULL;

	/* MAKE SURE ARRAY CONTAINS ELEMENTS */
	if(nx1<1||ny1<1) return(-1);

	/* ALLOCATE MEMORY FOR THE NEW TRANSPOSED MATRIX */
	nn= nx1*ny1;
	data2= realloc(data2,nn*sizeof(*data2));
	if(data2==NULL) return(-2);

	/* TRANSFER VALUES FROM ORIGINAL MATRIX TO NEW TRANSPOSED MATIX */
	for(ii=0;ii<nx1;ii++) {
		kk= ii*ny1;
		mm= ii;
		for(jj=0;jj<ny1;jj++) {
			data2[kk+jj] = data1[mm];
			mm+= nx1;
	}}

	/* COPY BACK TO ORIGINAL */
	for(ii=0;ii<nn;ii++) data1[ii]= data2[ii];

	/* UPDATE VARIABLES FOR WIDTH & HEIGHT OF MATRIX */
	*width=ny1; *height=nx1;

	/* CLEANUP AND RETURN */
	free(data2);
	return(0);
}
