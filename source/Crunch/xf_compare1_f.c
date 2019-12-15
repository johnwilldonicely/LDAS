/*

DESCRIPTION: 
	Compare funtion to be used in conjunction with qsort
	This version compares floating point numbers

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

	qsort(array,n,sizeof(float),xf_compare1_f);

*/

#include <stdlib.h>

int xf_compare1_f(const void *a, const void *b) { 

	float aa= *(float *)a, bb= *(float *)b; 

	if(aa<bb) return (-1); 
	if(aa==bb) return (0); 
	if(aa>bb) return (1);

}

