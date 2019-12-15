/*
<TAGS>string</TAGS>

DESCRIPTION:
	Adjust a date/time array by adding or subtracting a number of days, hours, minutes or seconds
	- should accurately reflect lengths of months, leap years, etc
USES:

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	int *input    : pre-allocated 6-item array input array
				input[0]: year
				input[1]: month
				input[2]: day
				input[3]: hour
				input[4]: minute
				input[5]: second
	int adjust    : adjustment to apply to one of the values
	char *field   : the field of to adjust (day,hour,min, or sec)
	char *message : pre-allocated array to hold error message

RETURN VALUE:
	0 on success, -1 on error
	input array will be adjusted

SAMPLE CALL:
	To get the date/time 90 minutes before 1 January 2000 10:30:15 AM

		char message[256];
		int i,x, mydate[6];
		mydate[0]=2000; mydate[1]=1; mydate[2]=1; mydate[3]=10; mydate[4]=30; mydate[5]=15;

		x= xf_datemod1(mydate,-90,"min",message)

*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

int xf_datemod1(int *input, int adjust, char *field, char *message) {

	char *thisfunc="xf_datemod1\0";
	long ii,jj,kk;
	struct tm date1 = { 0, 0, 0, 0, 0, 0 } ;
	time_t date_seconds;

	/* check integrity of input month,day,hour,min, & seconds (year is free to vary) */
	if(input[1] > 12) { sprintf(message,"%s [ERROR]: invalid month (%d)",thisfunc,input[1]); return(-1); }
	if(input[2] > 31) { sprintf(message,"%s [ERROR]: invalid day (%d)",thisfunc,input[2]); return(-1); }
	if(input[3] > 23) { sprintf(message,"%s [ERROR]: invalid hour (%d)",thisfunc,input[3]); return(-1); }
	if(input[4] > 59) { sprintf(message,"%s [ERROR]: invalid minutes (%d)",thisfunc,input[4]); return(-1); }
	if(input[5] > 59) { sprintf(message,"%s [ERROR]: invalid seconds (%d)",thisfunc,input[5]); return(-1); }

	/* build the time structure, correcting year and month */
	date1.tm_year = input[0] - 1900;
	date1.tm_mon  = input[1]- 1;
	date1.tm_mday = input[2];
	date1.tm_hour = input[3];
	date1.tm_min  = input[4];
	date1.tm_sec  = input[5];

	/* apply the adjustment */
	if(strcmp(field,"sec")==0)  adjust*=1;
	else if(strcmp(field,"min")==0)  adjust*=60;
	else if(strcmp(field,"hour")==0) adjust*=3600;
	else if(strcmp(field,"day")==0)  adjust*=86400;
	else { sprintf(message,"%s [ERROR]: invalid field \"%s\" (must be sec,min,hour or day)",thisfunc,field); return(-1); }

	/* seconds since start of epoch */
	date_seconds = mktime( &date1 ) + adjust ;

	/* update time structure: use localtime because mktime converts to UTC so may change date */
	date1 = *localtime( &date_seconds ) ; ;

    	/* copy values back to input date/time array, reversing the year & month correction applied earlier */
 	input[0]= date1.tm_year + 1900;
 	input[1]= date1.tm_mon + 1;
 	input[2]= date1.tm_mday;
 	input[3]= date1.tm_hour;
 	input[4]= date1.tm_min;
 	input[5]= date1.tm_sec;

  	return 0;
}
