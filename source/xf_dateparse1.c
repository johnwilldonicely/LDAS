/*
DESCRIPTION:
	Convert a date-string into integer year month and day values
	- can convert different date-formats into the three digits for constructing the actual date
	- all input values must be numeric (e.g. December= 12, not DEC or similar)
	- years represented as two-digits will not be expanded (eg. 99 does not become 1999)

DEPENDENCIES:
	None

ARGUMENTS:
	char *date1   : date1, date-string in one of the following numeric formats...
	int format    : date1, format (1-4) 1: dd/mm/yyyy 2: mm/dd/yyyy 3: yyyy/mm/dd 4: yyyymmdd
	int *year     : output (pass to function as address): year
	int *month    : output (pass to function as address): month
	int *day      : output (pass to function as address): day
	char *message : pre-allocated array to hold error message

RETURN VALUE:
	0 on success, -1 on error
	year, month and day will be assigned
	char array will hold message (if any)

SAMPLE CALL:
	char *date1="12/31/2000\0"
	int year,month,day;
	z= xf_dateparse1(date1,2,year,month,day,message);
	if(z==-1) { fprintf(stderr,"\n\t--- %s/%s\n\n",thisprog,message); exit(1); }

<TAGS> string time </TAGS>
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int xf_dateparse1(char *date1, int format, int *year, int *month, int *day, char *message) {

	char *thisfunc="xf_dateparse1\0";
	char c0,c1,c2;
	int y1,m1,d1;
	long ii,nn,i1,i2,lendate1;

	if(format<1||format>4) { sprintf(message,"%s [ERROR]: invalid format (%d)",thisfunc,format); return(-1); }
	lendate1= strlen(date1);

	/* OPTION1: NON-DELIMITED FORMAY YYYYMMDD */
	if(format==4) {
		if(lendate1!=8) { sprintf(message,"%s [ERROR]: (format:%d) input \"%s\" is not 8 digits long",thisfunc,format,date1); return(-1); }
		for(ii=0;ii<lendate1;ii++) if(date1[ii]<'0'||date1[ii]>'9') { sprintf(message,"%s [ERROR]: input \"%s\" contains non-numeric characters)",thisfunc,date1); return(-1); }
		nn=2; i1=4; i2=6; c0=0;
		d1= atoi(date1+i2); c2= date1[i2] ; date1[i2]= '\0';
		m1= atoi(date1+i1); c1= date1[i1] ; date1[i1]= '\0';
		y1= atoi(date1);
	}
	/* OPTION2: EVERYTHING ELSE, DEELIMITED BY / - OR . */
	else {
		nn=0; i1=i2=-1; c0=c1=c2=0; y1=m1=d1=0;
		/* count and substitute delimiters */
		for(ii=0;ii<lendate1;ii++ ) {
			c0= date1[ii]; /* temporarily store the current character */
			if(c0=='/'||c0=='-'||c0=='.') { /* if the current character is a delimiter... */
				date1[ii]= '\0';  /* reassign the delimiter to NULL, so an end-of-string is detected here */
				nn++; /* increment the delimiter-counter */
				if(nn==1) { i1=ii; c1=c0; } /* record the character and index for the first delimiter */
				else { i2=ii; c2=c0; break; } /* record the character and index for the second delimiter, and break */
			}
		}
		if(format==1)      { d1= atoi(date1); m1= atoi(date1+i1+1); y1= atoi(date1+i2+1); }
		else if(format==2) { m1= atoi(date1); d1= atoi(date1+i1+1); y1= atoi(date1+i2+1); }
		else               { y1= atoi(date1); m1= atoi(date1+i1+1); d1= atoi(date1+i2+1); }
	}
	//*TEST;printf("y1=%d m1=%d d1=%d\n",y1,m1,d1);

	/* RESTORE ORIGINAL VALUES TO DATE1, IF DELIMITERS WERE FOUND */
	if(i1!=-1) date1[i1]= c1;
	if(i1!=-1) date1[i2]= c2;

	/* CHECK THAT DATE1 FORMAT WAS CORRECT */
	if(nn!=2) {sprintf(message,"%s [ERROR]: (format:%d) insufficient delimiters ( / - or . ) in input \"%s\"",thisfunc,format,date1); return(-1);}
	if(y1<1) {sprintf(message,"%s [ERROR]: (format:%d) invalid year %d in input \"%s\"",thisfunc,format,y1,date1); return(-1);}
	if(m1<1||m1>12) {sprintf(message,"%s [ERROR]: (format:%d) invalid month %d in input \"%s\"",thisfunc,format,m1,date1); return(-1);}
	if(d1<1||d1>31) {sprintf(message,"%s [ERROR]: (format:%d) invalid day %d in input \"%s\"",thisfunc,format,d1,date1); return(-1);}

	/* COPY VALUES TO RESULTS ADDRESSES AND RETURN  */
	(*year)= y1;
	(*month)= m1;
	(*day)= d1;
	return (0);
}
