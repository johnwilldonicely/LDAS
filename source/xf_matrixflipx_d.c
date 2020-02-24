/*
<TAGS>dt.matrix</TAGS>

DESCRIPTION:
	Flip a matrix in the y-dimaension

	Example: this matrix...

	0	1	2	3
	4	5	6	7
	8	9	10	11

	... will be flipped to ...

	3	2	1	0
	7	6	5	4
	11	10	9	8


USES:

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:

	double *data1 : pointer to array of numbers representing the original matrix
				- sufficient memory must be allocated by calling program
				- this original memory will be freed
	long nx       : width of the matrix
	long ny       : height of the matrix
				- width and height must accurately reflect the total size of the data1 array

	NOTE: width and height are not swapped by this function, so if the calling function uses these
	variables after transposition, it is the responsibility of the calling function to do the swap

RETURN VALUE:

	A pointer to a FLIPPED version of the original numbers
	Returns NULL if a memory allocation error was encountered


SAMPLE CALL :
		matrix= xf_matrixflipx_d(matrix, width, height);

*/

#include <stdio.h>
#include <stdlib.h>

double *xf_matrixflipx_d(double *data1, long nx, long ny) {


	long ii,jj,kk,in1,in2;
	double *data2=NULL;


	/* MAKE SURE ARRAY CONTAINS ELEMENTS */
	if(ny<1||nx<1) return(data1);

	/* ALLOCATE MEMORY FOR THE NEW ROTATED MATRIX */
	data2=(double *)realloc(data2,nx*ny*sizeof(double));
	if(data2==NULL) return(NULL);

	/* TRANSFER VALUES FROM ORIGINAL MATRIX TO NEW FLIPPED VERSION */
	kk=0;
	ii=nx-1;
	for(ii=ii;ii>=0;ii--) {
		for(jj=0;jj<ny;jj++) {
			in1=jj*nx+ii;
			in2=jj*nx+kk;
			data2[in2]=data1[in1];
		}
		kk++;
	}

	/* FREE ORIGINAL MEMORY */
	free(data1);

	/* RETURN A POINTER TO THE NEW BLOCK OF MEMORY FOR THE TRANSPOSED MATRIX */
	return(data2);
}
