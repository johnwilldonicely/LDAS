/*
<TAGS>math</TAGS>
DESCRIPTION:
	Compare funtion to be used in conjunction with qsort
	This version compares double-precision floating point numbers

USES:
	When calling qsort, include this (or similar) function as the 4th argument

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	const void *a : pointer to first element in comparison
	const void *b : pointer to second element in comparison

RETURN VALUE:
	an integer determining if "a" if less than (-1) equal to (0) or greater than (1) "b"

SAMPLE CALL:

	qsort(array,n,sizeof(double),xf_compare1_d);

*/

#include <stdlib.h>

int xf_compare1_d(const void *a, const void *b) {

	double aa= *(double *)a, bb= *(double *)b;

	if(aa<bb) return (-1);
	else if(aa>bb) return (1);
	else return (0);
}
