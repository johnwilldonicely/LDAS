/*
DESCRIPTION:
	Scale a number to restricted range, wrapping when the range is exceeded
	- e.g. in a 3 colour scale (1-3) , the numbers 0-9 are wrapped as follows...
		original: 0 1 2 3 4 5 6 7 8 9
		wrapped:  3 1 2 3 1 2 3 1 2 3

USES:
	- assigning colours to values when there are only a limited number of colours which should repeat

DEPENDENCIES:
	None

ARGUMENTS:
	long data    : input data
	long min     : minimum value permitted
	long max     : maximum value permitted

RETURN VALUE:
	wrapped value

SAMPLE CALL:
	for(ii=-10;ii<20;ii++) { jj= xf_scale1_l(ii,1,5); printf("%ld\t%ld\n",ii,jj); }

<TAGS> transform </TAGS>
*/

#include <stdlib.h>
long xf_scale1_l(long old, long min, long max) {

	if(old>=min) {
		return( (old-min)%(1+max-min) + min ) ;
	}
	else {
		return( (1+old-min)%(1+max-min) + max );
	}
}
