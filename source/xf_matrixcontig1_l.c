/*
<TAGS>matrix filter</TAGS>
DESCRIPTION:
	- invalidate (set to zero) matrix bins not adjacent to other non-zero bins
	- designed for long-integer matrix arrays recording counts of events

USES:
	- allow programs to ignore isolated bins in a map

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	long *matrix1   : input, preallocated array matrix (counts)
	long setxbintot : input, width of array
	long setybintot : input, height of array
	long setcontig  : input, minimum adjacent non-zero bins required to keep a bin (must be 0-8)
	char *message   : output pre-allocated array to hold error message


RETURN VALUE:
	status flag (0=OK, -1=ERROR)
	matrix1 array will be overwritten
	message array will hold message (if any)

SAMPLE CALL:
	long matrix[15];
	width=3; height=5;
	z= xf_matrixcontig1_l(matrix,width,height,1,message);
	if(z=-1)  { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int xf_matrixcontig1_l(long *matrix1, long setxbintot, long setybintot, long setcontig, char *message) {

	char *thisfunc="xf_matrixcontig1_l\0";
	long ii,jj,kk,ncontig,index1,index2,x1,y1,x2,y2,matrixsize,*matrix2=NULL;;

	/* CHECK FOR INVALID ARGUMENTS */
	if(setxbintot<1) {sprintf(message,"%s [ERROR]: matrix width <1 (setxbintot=%ld)",thisfunc,setxbintot); return(-1); }
	if(setxbintot<1) {sprintf(message,"%s [ERROR]: matrix width <1 (setxbintot=%ld)",thisfunc,setxbintot); return(-1); }
	if(setcontig<0 || setcontig>8) {sprintf(message,"%s [ERROR]: setcontig (%ld) must be >=0 and <=8",thisfunc,setcontig); return(-1); }


	/* TOTAL NUMBER OF BINS IN MATRIX */
	matrixsize=setxbintot*setybintot;

	/* ADJUST SETCONTIG, BECAUSE THE REFERENCE BIN WILL ALSO BE TESTED IN THE LOOP BELOW */
	setcontig++;

	/* ALLOCATE MEMORY FOR TEMPORAY MATRIX ARRAY TO HOLD MODIFIED VALUES */
	if((matrix2=(long *)realloc(matrix2,matrixsize*sizeof(long)))==NULL) {sprintf(message,"%s [ERROR]: memory allocation error",thisfunc); return(-1); }


	/* PROCESS THE MATRIX */
	for(index1=0;index1<matrixsize;index1++) {

		/* don't bother with reference bins which are zero */
		if(matrix1[index1]<=0) { matrix2[index1]=0; continue; }

		/* convert index to x/y bins */
		if(index1==0) x1=y1=0;
		else {
			y1=(long)((double)index1/(double)setxbintot);
			x1=index1%setxbintot;
		}

		/* reset counter for number of contiguous >0 bins */
		ncontig=0;

		/* scan the adjacent bins for finite values, making sure row and column boundaries are respected */
		for(y2=(y1-1);y2<(y1+2);y2++) {
			if(y2>=0 && y2<setybintot) {
				for(x2=x1-1;x2<(x1+2);x2++) {
					if(x2>=0 && x2<setxbintot) {
						index2=y2*setxbintot+x2;
						if(matrix1[index2]>0) ncontig++;
		}}}}

		/* if contiguity criterion is met, matrix value is stored (otherwise, set to NAN) */
		if(ncontig>=setcontig) matrix2[index1]=matrix1[index1];
		else matrix2[index1]=0;
	}


	/* COPY NEW MATRIX VALUES BACK TO MATRIX1 */
	for(index1=0;index1<matrixsize;index1++) matrix1[index1]=matrix2[index1];

	/* TIDY UP AND FINISH  */
	free(matrix2);
	return (0);
}
