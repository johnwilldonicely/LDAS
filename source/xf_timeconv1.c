/*
DESCRIPTION:
	Convert seconds to days:hours:minutes:seconds

DEPENDENCIES:
	None

ARGUMENTS:
	double *seconds : input number of seconds
	int *days       : output (pass as address)
	int *hours      : output (pass as address)
	int *minutes    : output (pass as address)
	double *sec2    : output: remaining seconds (pass as address)

RETURN VALUE:
	0 on success, -1 on error

SAMPLE CALL:
	x= xf_timeconv1(seconds,&days,&hours,&minutes,&sec2);
	if(x!=0) { fprintf(stderr,"\n\t%s/%s\n\n",thisprog,message); exit(1); }

<TAGS> time string </TAGS>
*/

#include <stdio.h>
#include <stdlib.h>
int xf_timeconv1(double seconds, int *days, int *hours, int *minutes, double *sec2) {

	char *thisfunc="xf_timeconv1\0";

	*sec2= seconds;

	*days= (int)(*sec2/86400.0);
	*sec2-= (*days*86400);

	*hours= (int)(*sec2/3600.0);
	*sec2-= (*hours*3600);

	*minutes= (int)(*sec2/60.0);
	*sec2-= (*minutes*60);

	return (0);
}
