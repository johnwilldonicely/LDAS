/*
<TAGS>math dt.matrix</TAGS>

DESCRIPTION:
	Calculate the average matrix from a multi-matrix array
	John Huxter - 4.March.2012
USES:
	Creating averages of 2-dimensional arrays - eg. density maps

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	float *multimatrix
		- input array already holding data for multiple matrices to be averaged
		- NOTE: values for each matrix should be normalized first
	size_t n_matrices
		- number of matrices
	size_t bintot
		- number of cells in each matrix
	char message[]
		- holds error message or other report
		- recommended pre-allocation of memory by calling function (eg. char message[256])

RETURN VALUE:
	A pointer to an array (newmatrix) of type double with xbins*ybins elements
	This variable must be initialized by the calling function and assigned value NULL
	This function will handle assigning sufficient memory
	If a memory allocation error occurs, a NULL is returned

SAMPLE CALL:

	new= xf_matrixavg1_f(multimatrix,nmatrices,bintot,message);

	if(new==NULL) {fprintf(stderr,"--- Error: %s\n",message) ; exit(1);}

*/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

float *xf_matrixavg1_f(float *multimatrix, size_t n_matrices, size_t bintot, char message[]) {

	size_t i,n,bin,*n_bincount=NULL;
	float *newmatrix;

	/* CALCULATE THE TOTAL DATA */
	n=n_matrices*bintot;

	/* ALLOCATE MEMORY FOR THE AVERAGE MATRIX */
	newmatrix=(float *)calloc(bintot,sizeof(float));
	if(newmatrix==NULL) { sprintf(message," xf_matrixavg1_d memory allocation error");return(NULL); }

	/* ALLOCATE MEMORY FOR VARIABLE TO RECORD THE NUMBER OF MATRICES CONTRIBUTING VALID VALUES TO EACH BIN */
	n_bincount=(long *)calloc(bintot,sizeof(long));
	if(n_bincount==NULL) { sprintf(message," xf_matrixavg1_d memory allocation error"); free(newmatrix) ; return(NULL); }

	/* SCAN THE MULTIMATRIX AND BUILD SUMMED VALUES FOR THE NEW MATRIX */
	for(bin=0,i=0;i<n;i++) {
		if(isfinite(multimatrix[i])) {
			newmatrix[bin]+=multimatrix[i]; /* running sum for each matrix bin */
			n_bincount[bin]++; 		/* keep track of the number of elements contributing to the sum */
		}
		/* check if one complete matrix has been read */
		if(++bin>=bintot) { bin=0; n_matrices++; }
	}

	/* CONVERT SUMMED VALUES IN EACH BIN TO AVERAGES */
	for(i=0;i<bintot;i++) {
		if(n_bincount[i]>0) newmatrix[i]/=n_bincount[i];
		else newmatrix[i]=NAN;
	}

	/* RECORD THE NUMBER OF MATRICES WHICH WERE IN THE MULTIMATRIX */
	sprintf(message,"success");
	free(n_bincount);
	return (newmatrix);
}
