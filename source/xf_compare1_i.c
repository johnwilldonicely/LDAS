/*
<TAGS>math</TAGS>
DESCRIPTION:
	Compare funtion to be used in conjunction with qsort
	This version compares long integers

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

	qsort(array,n,sizeof(long),xf_compare1_l);

*/

#include <stdlib.h>

int xf_compare1_i(const void *a, const void *b) {

	int aa= *(int *)a, bb= *(int *)b;

	if(aa<bb) return (-1);
	else if(aa>bb) return (1);
	else return (0);
}
