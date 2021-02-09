/*
<TAGS>string</TAGS>

DESCRIPTION:
	- convert a date to the day- or week-in-the-year
	- weeks are presumed to begin with Monday
USES:
	- convert a date to a number useable for plotting results

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	int sety      ; year (e.g. 2000)
	int setm      : month-in-year (e.g. 12)
	int setd      : day-in-month (e.g. 28)
	int setconv   : convert to week (1) or day (2)
	char *message : pre-allocated array to hold error message

RETURN VALUE:
	on success: week-in-year (0-52) or day-in-year (1-365)
	on error: -1
	message array will be filled if there was an error

SAMPLE CALL:
		char message[256];
		int z,year=2000,month=12,day=31;
		z= xf_dateconv1(year,month,day,1,message);
		if(z>0) printf("week-in-year=%d\n",z);
		else { fprintf(stderr,"\n\t--- %s/%s\n\n",thisprog,message); exit(1); }

*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

int xf_dateconv1(int sety, int setm, int setd, int setconv, char *message) {

	char *thisfunc="xf_dateconv1\0";
	char *buffer=NULL;
	int result=-1;
	long ii,jj,kk;
	struct tm date1,*ptm;
	time_t rawtime;

	/* check validity of arguments */
	if(setm > 12) { sprintf(message,"%s [ERROR]: invalid month (%d)",thisfunc,setm); return(-1); }
	if(setd > 31) { sprintf(message,"%s [ERROR]: invalid day (%d)",thisfunc,setd); return(-1); }
	if(setconv<1||setconv>2) { sprintf(message,"%s [ERROR]: invalid conv (%d) - must be 1 or 2",thisfunc,setconv); return(-1); }

	/* build the time structure - hours minutes and seconds are set to zero */
	memset(&date1,0,sizeof(date1));
	date1.tm_year = sety;
	date1.tm_mon  = setm-1;
	date1.tm_mday = setd;

	/* OPTION-1: calculate the week-in-year (1-52) */
	if(setconv==1) {
		date1.tm_sec  = -1; /* adjust to the second-before midnight - because for some reason the weeks roll over one day before they should! */
		rawtime= mktime(&date1); /* fill the time structure */
		ptm= gmtime(&rawtime); /* make a pointer referenced to GMT */
		buffer= malloc(8*sizeof(*buffer)); /* create a buffer to hold the text */
		strftime(buffer,8,"%W",ptm); /* print the week-number to the buffer */
		result= atoi(buffer); /* convert to a number */
		free(buffer); /* free the memory */

	}
	/* OPTION-2: calculate the day-in-year (1-365) */
	else {
		rawtime= mktime(&date1); /* fill the time structure */
		result= (date1.tm_yday)+1;
	}

  	return(result);
}
