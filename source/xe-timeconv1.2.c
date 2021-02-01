#define thisprog "xe-timeconv1"
#define TITLE_STRING thisprog" v 2: 21.May.2016 [JRH]"
#define MAXLINELEN 1000

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/*  define and include required (in this order!) for time functions */
#define __USE_XOPEN // required specifically for strptime()
#include <time.h>


/*
<TAGS>string</TAGS>

v 2: 21.May.2016 [JRH]
	- switch to new long-int version of strstr1

v 2: 1.December.2014 [JRH]
	- add conversion option for seconds to hh:mm:ss
*/

/* external functions start */
long *xf_lineparse2(char *line,char *delimiters, long *nwords);
long *xf_lineparse1(char *line,long *nwords);
long xf_strstr1(char *haystack,char *needle,int setcase, int firstlast);
/* external functions end */

int main (int argc, char *argv[]) {


	/* time-related  variables & structures */
	char timestring[256];
	time_t t1,t2;
	struct tm *tstruct1;

	/* general variables */
	char infile[256],line[MAXLINELEN];
	long int ii,jj,kk,nn;
	int v,w,x,y,z,col,colmatch;
	float a,b,c,d;
	double aa,bb,cc,dd,ee, result_d[64];
	FILE *fpin,*fpout;
	size_t sizeofchar=sizeof(char),sizeofshort=sizeof(short),sizeoflong=sizeof(long),sizeofint=sizeof(int),sizeoffloat=sizeof(float),sizeofdouble=sizeof(double);

	/* program-specific variables */
	char *pday,*pmonth,*pyear;
	int hours,minutes,seconds;
	long nwords=0,*start=NULL;
	long colhour,colmin,colsec;
	/* arguments */
	int setformat=1;

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Convert hh:mm:ss time format to seconds or vice-versa\n");
		fprintf(stderr,"Assumes one time per input line\n");
		fprintf(stderr,"Assumes colon is the delimiter if converting to seconds\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [input] [options]\n",thisprog);
		fprintf(stderr,"	[input]: file name or \"stdin\"\n");
		fprintf(stderr,"VALID OPTIONS:\n");
		fprintf(stderr,"	-f input format (1= hh:mm:ss, 2= seconds) [%d]:\n",setformat);
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s times.txt\n",thisprog);
		fprintf(stderr,"	echo 12:59:59.123 | %s stdin\n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	one modified time per valid input line\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	sprintf(infile,"%s",argv[1]);
	for(ii=2;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-f")==0)    setformat= atoi(argv[++ii]);
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
	}}
	if(setformat!=1&&setformat!=2) {fprintf(stderr,"\n--- Error[%s]: invalid format (-f %d) - must be 1 or 2\n\n",thisprog,setformat);exit(1);}


	// /* INTIALISE T1 AND TSTRUCT1 - THIS AVOIDS USING MALLOC OR MEMSET */
	// t1 = time(NULL);
	// tstruct1 = localtime(&t1);
	// // make a new tstruct1 and t1, perhaps from a string read from a file
	// snprintf(timestring,32,"2021/01/19 20:50:00");
	// strptime(timestring,"%Y/%m/%d %H:%M:%S", tstruct1); // convert string to broken-down-time (Y/M/D etc)
	// t1 = mktime(tstruct1);  // convert broken-down-time to seconds
	// fprintf(stderr,"\tstring: %s	time: %ld\n",timestring,t1); // output
	// t1+= 301; // add 5 minutes and 1 second
	// tstruct1= localtime(&t1); // convert seconds to broken-down-time (opposite of mktime)
	// strftime(timestring,sizeof(timestring),"%Y/%m/%d %H:%M:%S",tstruct1); // convert broken-down-time to string (opposite of strptime)
	// fprintf(stderr,"\tstring: %s	time: %ld\n",timestring,t1); // output

	/* READ & CONVERT THE DATA */
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}


	if(setformat==1) {
		nn=0;
		while(fgets(line,MAXLINELEN,fpin)!=NULL) {

			/* increment line counter first */
			nn++;

			/* trim trailing newline off line so that it cannot be detected as an extra field after a delimiter */
			ii= xf_strstr1(line,"\n",1,2);
			if(ii>0)line[ii]='\0';

			/* replace delimiters in line with NULLS and create array of indices to each word (day,month or year) */
			start= xf_lineparse2(line,":",&nwords);

			/* there should be between 1 and 3 fields */
			if(nwords<1) {fprintf(stderr,"--- Warning[%s]: no fields on line %ld\n",thisprog,nn);continue;}
			else if(nwords==1) {
				aa= atof(line+start[0]);
			}
			else if(nwords==2) {
				aa= 60.0*atof(line+start[0]) + atof(line+start[1]) ;
			}
			else if(nwords==3) {
				aa= 3600.0*atof(line+start[0]) + 60.0*atof(line+start[1]) + atof(line+start[2]) ;
			}
			else if (nwords>3) {fprintf(stderr,"--- Warning[%s]: too many fields (>3) on line %ld\n",thisprog,nn);continue;}

			/* output */
			printf("%f\n",aa);

		}
	}

	if(setformat==2) {
		nn=0;
		while(fgets(line,MAXLINELEN,fpin)!=NULL) {

			/* increment line counter first */
			nn++;

			/* trim trailing newline off line so that it cannot be detected as an extra field after a delimiter */
			ii= xf_strstr1(line,"\n",1,2);
			if(ii>0)line[ii]='\0';

			/* replace whitespace delimiters in line with NULLS and create array of indices to each word  */
			start= xf_lineparse1(line,&nwords);

			/* there should be only one field */
			if(nwords!=1) {fprintf(stderr,"--- Warning[%s]: %ld fields on line %ld - should be only one\n",thisprog,nwords,nn);continue;}

			aa=atof((line+start[0]));
			seconds=(int)aa;
			bb=(aa-seconds);

			hours=(int)(seconds/3600.0);
			seconds-= (hours*3600);
			minutes= (int)(seconds/60.0);
			seconds-= (minutes*60);
			x=(int)seconds;

			/* output */
			if(bb==0) printf("%02d:%02d:%02d\n",hours,minutes,x);
			else {
				sprintf(line,"%g",bb);
				printf("%02d:%02d:%02d.%s\n",hours,minutes,x,(line+2));
			}
		}
	}


	if(strcmp(infile,"stdin")!=0) fclose(fpin);
	if(start==NULL) free(start);
	exit(0);

}
