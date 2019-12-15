/*
<TAGS>math</TAGS>
DESCRIPTION:
	Calculate the index to a list-element given the min, max and number of elements
	Assumes elements in list are evenly spaced (eg. a series of timestamps)

USES:

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	double min     : smallest value in series
	double max     : largest value in series
	long n         : number of elements in the series
	double value   : value to return the index to (will be rounded down to nearest integer)
	char *message  : pointer to pre-allocated array to hold error message if any

RETURN VALUE:
	index, or -1 on error

SAMPLE CALL:


*/

#include <stdio.h>
#include <stdlib.h>

long xf_getindex1_d(double min, double max, long n, double value, char *message) {

	char *thisfunc="xf_getindex1_d\0";

	/* check for invalid n */
	if(n<1) { sprintf(message,"%s [ERROR]: n < 1",thisfunc); return(-1); }
	/* check for invalid range */
	else if(max<min) { sprintf(message,"%s [ERROR]: max (%g) < min (%g)",thisfunc,max,min); return(-1); }
	/* check for invalid value */
	else if(value<min||value>max) { sprintf(message,"%s [ERROR]: value (%g) out of min-max range (%g to %g)",thisfunc,value,min,max); return(-1); }
	/* check for cases where index must be zero - avoid possible division of/by zero */
	else if(n==1 || max==min || value==min) return(0);
	/* otherwise return calculated index */
	else return( (long) ((value-min) / ((max-min)/(n-1))) );

}
