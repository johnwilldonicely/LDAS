#define thisprog "xe-timeconv2"
#define TITLE_STRING thisprog" 1.Feb.2021 [JRH]"
#define MAXLINELEN 1000
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
/*  define and include required (in this order!) for time functions */
#define __USE_XOPEN // required specifically for strptime()
#include <time.h>


/*
<TAGS>string time date</TAGS>

 1.Feb.2021 [JRH]
	- new time conversion program to handle date+time
 /

/* external functions start */
char *xf_lineread1(char *line, long *maxlinelen, FILE *fpin);
long *xf_lineparse2(char *line,char *delimiters, long *nwords);
/* external functions end */

int main (int argc, char *argv[]) {

	/* time-related  variables & structures */
	char timestring[256];
	time_t t0=0,t1=0;
	struct tm *tstruct1;
	/* general variables */
	char *line=NULL, *pline=NULL;
	long ii,jj,kk,mm,nn,maxlinelen=0;
	double aa,bb,cc,dd,ee,result_d[64];
	FILE *fpin,*fpout;
	/* program-specific variables */
	char *pword;
	int sizeofdata;
	long *iword=NULL,nwords;
	float *data1=NULL;
	/* arguments */
	char *infile=NULL;
	int setformat=1;

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Convert \"%%d/%%m/%%Y %%H:%%M:%%S\" times to seconds-since-1970\n");
		fprintf(stderr,"Assumes one time per input line, in the first column\n");
		fprintf(stderr,"Blank lines and comments (#) will be passed unaltered\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"    %s [in] [options]\n",thisprog);
		fprintf(stderr,"    [in]: file name or \"stdin\"\n");
		fprintf(stderr,"VALID OPTIONS:\n");
		fprintf(stderr,"    -f : output format options [%d]:\n",setformat);
		fprintf(stderr,"        0: seconds elapsed since the first value read (zero)\n");
		fprintf(stderr,"        1: seconds since 1 January 1970\n");
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"    %s times.txt\n",thisprog);
		fprintf(stderr,"    echo \"31/12/2020 12:59:59.123\" | %s stdin\n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"    one modified time per valid input line\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	infile= argv[1];
	for(ii=2;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-f")==0) setformat= atoi(argv[++ii]);
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
	}}
	if(setformat!=0&&setformat!=1) {fprintf(stderr,"\n--- Error[%s]: invalid format (-f %d) - must be 0 or 1 \n\n",thisprog,setformat);exit(1);}

	/* INTIALISE T1 AND TSTRUCT1 - THIS AVOIDS USING MALLOC OR MEMSET */
	t1 = time(NULL);
	tstruct1 = localtime(&t1);

	/********************************************************************************/
	/* READ & CONVERT THE DATA */
	/********************************************************************************/
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
	sizeofdata= sizeof(*data1);
	mm= nn= 0;
	while((line=xf_lineread1(line,&maxlinelen,fpin))!=NULL) {
		mm++;
		if(line[0]=='#') {printf("%s",line) ;continue;}
		if(strlen(line)<2) {printf("\n");continue;}
		if(maxlinelen==-1)  {fprintf(stderr,"\n--- Error[%s]: readline function encountered insufficient memory\n\n",thisprog);exit(1);}
		iword= xf_lineparse2(line,"\t",&nwords);
		if(nwords<0) {fprintf(stderr,"\n--- Error[%s]: lineparse function encountered insufficient memory\n\n",thisprog);exit(1);};
		// for now assume date-time string is in first column (0)
		pword= line+iword[0];
		// convert string to broken-down-time structure (Y/M/D etc)
		// assumes input format is %d/%m/%Y %H:%M:%S
		pline= strptime(pword,"%d/%m/%Y %H:%M:%S", tstruct1);
		if(pline==NULL) {fprintf(stderr,"\n--- Error[%s]: nvalid date format on line %ld\n\n",thisprog,mm);exit(1);};
		// convert broken-down-time to seconds-since-1970
		t1 = mktime(tstruct1);
		// if this is the first timestamp and elapsed-time is required, make t1 the reference time
		if(nn==0) { if(setformat==0) t0= t1; }
		// output
		printf("%ld\n",(t1-t0));
		nn++;
	}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);

	/********************************************************************************/
	/* CLEANUP AND EXIT */
	/********************************************************************************/
END:
	if(line!=NULL) free(line);
	exit(0);

}
