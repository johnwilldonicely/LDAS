/************************************************************************************
<TAGS>math</TAGS>
DESCRIPTION:
	Return the decimal precision of a number as written in characters

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	char *number : [input] character string representing the number

RETURN VALUE:
	The number of significant digits after the decimal, for example....

	number		precision

	 100		0
	 1.123		3
	 1.12340	4

SAMPLE CALL:

	int precision;
	double data=100.1234500;

	precision= xf_precision_d(data);

*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int xf_precision_c(char *number) {

	int start,end,precision=0;
	char line[15];

	start=end=-1;
	start= strlen(number)-1;
	while (start>=0) {

		if(number[start]=='.') break;
		else if(end==-1 && number[start]!='0') end=start;
		else start--;
	}

	if(end<0 || start<0) return(0);
	else return(end-start);
}
