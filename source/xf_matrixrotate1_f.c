/*
<TAGS>dt.matrix</TAGS>

DESCRIPTION:
	Rotate a 1-dimensional array of numbers meant to be interpreted as a 2-dimensional matrix

USES:
	- image or map rotation
	- matrix algebra

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	float *data1 (input)- pointer to array of numbers representing the original matrix
				- sufficient memory must be allocated by calling program
				- this original memory will be freed
	long *nx1 (input/output)- width of the matrix, to be modified depending on rotation
	long *ny1 (input/output)- height of the matrix, to be modified depending on rotation
				- nx1 and ny1 MUST reflect the total size of the data1 array
	int r (input) - rotation (90,180,270,-90,-180,-270)
				- 0,360, or any other value simply results in no changes

RETURN VALUE:

	A pointer to a rotated version of the original numbers
	Returns NULL if a memory allocation error was encountered
	NOTE: nx1 and ny1 will be also modified according to the rotation
	NOTE: if a valid value for "r" is set, the original memory for the matrix is freed


TEST PROGRAM :

	#include <stdio.h>
	#include <stdlib.h>

	int main (int argc, char *argv[]) {

		float *xf_matrixrotate1_d(float *data1, long *nx1, long *ny1, int r);
		float *matrix=NULL;
		long width=3,height=5,n=width*height,x,y;

		matrix=(float *)realloc(matrix,n*sizeof(float));
		for(x=0;x<n);x++) matrix[x]=(float)x;

		for(y=0;y<height;y++) { for(x=0;x<width;x++) printf("%g\t",matrix[y*width+x]);	printf("\n"); }
		printf("\n");

		matrix= xf_matrixrotate1_d( matrix, &width, &height, 90 );

		for(y=0;y<height;y++) { for(x=0;x<width;x++) printf("%g\t",matrix[y*width+x]);	printf("\n"); }
	}

*/

#include <stdio.h>
#include <stdlib.h>

float *xf_matrixrotate1_f(float *data1, long *width, long *height, int r) {

	long x1,y1,xmax1,ymax1,index1;
	long x2,y2,nx1=*width,ny1=*height,nx2,ny2,index2;
	long i,j,k;
	float *data2=NULL;

	/* MAKE SURE ARRAY CONTAINS ELEMENTS */
	if(nx1<1||nx1<1) return(data1);

	/* DETERMINE THE HEIGHT (ny2) AND WIDTH (nx2) OF THE ROTATED MATRIX */
	if(r==90 || r==270 || r==-90 || r==-270) { nx2=ny1; ny2=nx1; }
	else if(r==180||r==-180) { nx2=nx1; ny2=ny1; }
	/* IF ROTATION IS NOT A MULTIPLE OF 90, CHANGE NOTHING */
	else return(data1);

	/* ALLOCATE MEMORY FOR THE NEW ROTATED MATRIX */
	data2=(float *)realloc(data2,nx2*ny2*sizeof(float));
	if(data2==NULL) return(NULL);

	/* CALCULATE HIGHEST VALUE OF ROW & COLUMN - FOR SPEED IN THE NEXT STEP */
	xmax1=nx1-1; ymax1=ny1-1;

	/* TRANSFER VALUES FROM ORIGINAL MATRIX TO NEW ROTATED MATRIX */
	if(r==90||r==-270)  for(y1=0;y1<ny1;y1++) { j=y1*nx1; k=ymax1-y1; for(x1=0;x1<nx1;x1++) { data2[x1*nx2+k]= data1[j+x1]; } }
	if(r==180||r==-180) for(y1=0;y1<ny1;y1++) { j=y1*nx1; k=ymax1-y1; for(x1=0;x1<nx1;x1++) { data2[k*nx2+(xmax1-x1)]= data1[j+x1]; } }
	if(r==-90||r==270)  for(y1=0;y1<ny1;y1++) { j=y1*nx1;             for(x1=0;x1<nx1;x1++) { data2[(xmax1-x1)*nx2+y1]= data1[j+x1]; } }

	/* FREE ORIGINAL MEMORY */
	free(data1);

	/* UPDATE VARIABLES FOR WIDTH & HEIGHT OF MATRIX */
	*width=nx2; *height=ny2;

	/* RETURN A POINTER TO THE NEW BLOCK OF MEMORY FOR THE ROTATED MATRIX */
	return(data2);
}
