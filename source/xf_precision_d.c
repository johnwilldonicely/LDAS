/************************************************************************************
<TAGS>math</TAGS>
DESCRIPTION:
	Return the decimal precision of a number

	- bear in mind some numbers cannot be accurately represented in float/double
	- hence accurately calculating precision of some numbers will fail
	- for this reason the "max" argument allows the user to limit the search for zeros

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	double number : [input] double-precision floating-point number to be analyzed
	int max       : [1-12] the maximum number of decimal places to consider
				- 8 recommended
				- might need to be lower for large numbers
				- 12 is the absolute maximum

RETURN VALUE:
	The number of significant digits after the decimal, for example....

	number		precision

	 100		0
	 1.123		3
	 1.12340	4

SAMPLE CALL:

	int precision;
	double data=100.1234500;

	precision= xf_precision_d(data,8);

*/

#include <stdlib.h>
#include <stdio.h>

int xf_precision_d(double number,int max) {

	int c,start=0,precision=0;
	char line[15];
	if(max<1) max=1;
	if(max>12) max=12;

	snprintf(line,15,"%0.*lf",max,(number-(int)number));

	for(c=max+1;c>1;c--) {
		if(line[c]!='0') start=1;
		if(start==1) precision++;
	}
	return(precision);
}
