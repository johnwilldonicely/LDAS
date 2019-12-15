/*
<TAGS>math</TAGS>
DESCRIPTION:
	Round an array of double-precision numbers to the nearest desired base

	* If input is not finite (INF or NAN) no rounding is attempted
	* Result may in fact have zeros added for a few decimal places after desired precision
		Example: 49.974998 rounded to 3 decimal places may become 49.975002
	* This is due to the use of nextafter() to increment the numbers slightly
	* This ensures that when using a formatted print to display the numbers, rounding is in the right direction
	* It also means that a precise input (50.000000) may become slightly less precise (50.000004)

REVISIONS:
	14 April 2014: fix rounding for negative numbers (subtract 0.5, don't add)
	4 November 2015: fix rounding for negative numbers (prevent negative numbers from rounding to zero)

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	double input : input array
	size_t nn : size of array
	double base : nearest number to round to  (eg. 0.1, 1, 100, 9 etc)
	int setdown : use standard rounding (0) or force to round down (1)

RETURN VALUE:
	zero (no errors possible)
	NOTE: input array is modified

SAMPLE CALLS:
	x= round2_d( array, 10000, 10,0); # if encountered, rounds 105 to the nearest ten, so x=110
	x= round2_d( array, 10000, 10,1); # if encountered, rounds 105 down to the nearest ten, so x=100

*/

#include <stdlib.h>
#include <math.h>
#include <float.h>

int xf_round2_d(double* input, size_t nn, double setbase, int setdown) {

	size_t ii;
	double aa;

	if(setdown==0) {
		for(ii=0;ii<nn;ii++) {
			aa=input[ii];
			if(isfinite(aa)) {
				if(aa>=0.0) {
					aa= nextafter(aa,DBL_MAX)/setbase;
					input[ii] = setbase*(long)(aa+0.5);
				}
				else {
					aa= nextafter(aa,-DBL_MAX)/setbase;
					input[ii] = setbase*(long)(aa-0.5);
				}
	}}}

	if(setdown==1) {
		for(ii=0;ii<nn;ii++) {
			aa=input[ii];
			if(isfinite(aa)) {
				if(aa>=0.0) {
					aa= nextafter(aa,DBL_MAX)/setbase;
					input[ii]= setbase*(long)(aa);
				}
				else {
					aa= nextafter(aa,DBL_MAX)/setbase;
					input[ii] = setbase*((long)aa-1);
					}
	}}}

	return(0);
}
