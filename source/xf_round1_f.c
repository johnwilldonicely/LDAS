/*
<TAGS>math</TAGS>
DESCRIPTION:
	Round a floating-point number to the nearest desired base

	* If input is not finite (INF or NAN) no rounding is attempted
	* Result may in fact have zeros added for a few decimal places after desired precision
		Example: 49.974998 rounded to 3 decimal places may become 49.975002
	* This is due to the use of nextafterf() to increment the numbers slightly
	* This ensures that when using a formatted print to display the numbers, rounding is in the right direction
	* It also means that a precise input (50.000000) may become slightly less precise (50.000004)

REVISIONS:
	14 April 2014: fix rounding for negative numbers (subtract 0.5, don't add)
	4 November 2015: fix rounding for negative numbers (prevent negative numbers from rounding to zero)

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	float input : input number
	float base : nearest number to round to  (eg. 0.1, 1, 100, 9 etc)
	int setdown : use standard rounding (0) or force to round down (1)

RETURN VALUE:
	The rounded number
	NOTE: if input is not finite (INF or NAN) no rounding is attempted

SAMPLE CALLS:
	x= round1_f( 105, 10,0); # rounds 105 to the nearest ten, so x=110
	x= round1_f( 105, 10,1); # rounds 105 down to the nearest ten, so x=100

*/

#include <math.h>
#include <float.h>

float xf_round1_f(float input, float setbase, int setdown) {

	if(!isfinite(input)) return(input);

	if(input>=0.0) {
		input= nextafterf(input,FLT_MAX)/setbase;
		if(setdown==0) input = setbase*(long)(input+0.5);
		else input = setbase*(long)(input);
	}
	else {
		input= nextafterf(input,FLT_MAX)/setbase;
		if(setdown==0) input = setbase*(long)(input-0.5);
		else input = setbase*((long)input-1.0);
	}

	return(input);

}
