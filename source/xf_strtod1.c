/**********************************************************************************
<TAGS>string</TAGS>

DESCRIPTION:
	Convert a string to a double float number
	Returns "NAN" if string is non-numeric
	Defines NAN if necessary (for Windows)

USES:
	Testing if input is numeric

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	char *str : input character array (eg. 123, NAN, or abc)

RETURN VALUE:
	A double-precision floating-point number

		str 	returns
		---------------
		123 	123
		NAN 	NAN
		INF 	INF
		abc 	NAN

SAMPLE CALL:

	char input[64]="123.456";
	double aa

	aa= xf_strtod1(input);
**********************************************************************************/

#include <math.h>
#include <stdio.h>

double xf_strtod1(char *str) {

	int z;
	double aa;

	z=sscanf(str,"%lf",&aa);

	if(z!=1) {

		#ifndef NAN
			unsigned long nan1[2]={0xffffffff, 0x7fffffff};
			aa=*( double* )nan1;
		#else
			aa=NAN;
		#endif
	}

	return(aa);
}
