/*
<TAGS>dt.matrix</TAGS>
DESCRIPTION:
	Transpose a 1-dimensional array of numbers meant to be interpreted as a 2-dimensional matrix
	Note that this differs from rotation, because low columns become low rows ( & vice versa)
	This version deals with floating-point numbers

	Example: this matrix...

	0	1	2	3
	4	5	6	7
	8	9	10	11

	... will be transposed to...

	0	4	8
	1	5	9
	2	6	10
	3	7	11


USES:
	- table transposition - interchange rows and columns
	- matrix algebra

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:

	float *data1	: pointer to array of numbers representing the original matrix
				- sufficient memory must be allocated by calling program
				- this original memory will be freed
	long width	: width of the matrix
	long height 	: height of the matrix
				- width and height must accurately reflect the total size of the data1 array

	NOTE: width and height are not swapped by this function, so if the calling function uses these
	variables after transposition, it is the responsibility of the calling function to do the swap

RETURN VALUE:

	A pointer to a transposed version of the original numbers
	Returns NULL if a memory allocation error was encountered


SAMPLE CALL :

		matrix= xf_matrixtrans1_f(matrix, &width, &height);

*/

#include <stdio.h>
#include <stdlib.h>

float *xf_matrixtrans1_f(float *data1, long *width, long *height) {


	long h,i,j,k,nx,ny;
	float *data2=NULL;

	nx = (*width);
	ny = (*height);

	/* MAKE SURE ARRAY CONTAINS ELEMENTS */
	if(ny<1||nx<1) return(data1);

	/* ALLOCATE MEMORY FOR THE NEW ROTATED MATRIX */
	data2= realloc(data2,nx*ny*sizeof(*data2));
	if(data2==NULL) return(NULL);

	/* TRANSFER VALUES FROM ORIGINAL MATRIX TO NEW TRANSPOSED */
	for(i=0;i<nx;i++) {
		k=i*ny;
		h=i;
		for(j=0;j<ny;j++) {
			data2[k+j] = data1[h];
			h+=nx;
		}
	}

	/* FREE ORIGINAL MEMORY */
	free(data1);

	/* UPDATE WIDTH AND HEIGHT VALUES TO REFLECT TRANSPOSITION */
	(*width) = ny;
	(*height) = nx;

	/* RETURN A POINTER TO THE NEW BLOCK OF MEMORY FOR THE TRANSPOSED MATRIX */
	return(data2);
}
