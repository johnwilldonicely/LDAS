#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define thisprog "xe-dateconv1"
#define TITLE_STRING thisprog" v 6: 27.Jan.2018 [JRH]"
#define MAXlen 1000

/*
<TAGS>string time</TAGS>

v 6: 27.Jan.2018 [JRH]
	- significant rewrite to move date-parsing to function xf_dateparse1
		- this function preserves the input string
	- add column-specific processing
	- add option to suppress formatting warning-errors (-verb 0)

v 5: 27.Jan.2018 [JRH]
	- bugfix -o 2 - previously omitted forward-slashes in date output"
	- add new -o options: week (7) or day (8) of year

v 5: 15.Nov.2018 [JRH]
	- expand output formats and make more consistent with input codes

v 5: 21.May.2016 [JRH]
	- switch to new long-int version of strstr1

v 5: 21.September.2015 [JRH]
	- add conversion from yyyymmdd to something else

v 4: 23.August.2015 [JRH]
	- now alows conversion to 8-digit dates eg. 20151231
	- this is less ambiguous about the fact that it is, in fact, a date!

 v 3: 30.March.20145 [JRH]
 	- allows specification of output as well as input
*/

/* external functions start */
char *xf_lineread1(char *line, long *maxlen, FILE *fpin);
long *xf_lineparse2(char *line,char *delimiters, long *nwords);
long xf_strstr1(char *haystack,char *needle,int setcase, int firstlast);
int xf_dateparse1(char *date1, int format, int *year, int *month, int *day, char *message);
int xf_dateconv1(int sety, int setm, int setd, int setconv, char *message);
/* external functions end */

int main (int argc, char *argv[]) {
	/* general variables */
	char *infile=NULL,*line1=NULL,*line2=NULL,message[MAXlen];
	long ii,jj,kk,nn,mm,maxlen=0,prevlen=0,nwords=0,word,*iword=NULL;
	int v,w,x,y,z,col,colmatch;
	float a,b,c,d;
	double aa,bb,cc,dd,ee;
	size_t sizeofline2=sizeof(*line2);
	FILE *fpin,*fpout;
	/* program-specific variables */
	int day,month,year,year2,adj1=0;
	long scdm1;
	/* arguments */
	char *setzero=NULL;
	int setin=1,setout=4,sethead=0,setverb=0;
	long setcoldate=1;

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Convert date to 6- or 8-digit format (yymmdd or yyyymmdd)\n");
		fprintf(stderr,"Assumes one date per input line\n");
		fprintf(stderr,"Accepts three possible field delimiters: / . -\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [input] [options]\n",thisprog);
		fprintf(stderr,"		[input]: file name or \"stdin\"\n");
		fprintf(stderr,"VALID OPTIONS:\n");
		fprintf(stderr,"	-h: input has header-line (0=NO 1=YES) [%d]\n",sethead);
		fprintf(stderr,"	-c: column holding date [%ld]\n",setcoldate);
		fprintf(stderr,"	-i: input format [%d]\n",setin);
		fprintf(stderr,"		1: UK   (dd/mm/yyyy)\n");
		fprintf(stderr,"		2: USA  (mm/dd/yyyy)\n");
		fprintf(stderr,"		3: JP   (yyyy/mm/dd)\n");
		fprintf(stderr,"		4: LDAS (yyyymmdd)\n");
		fprintf(stderr,"	-o: output format [%d]\n",setout);
		fprintf(stderr,"		1: UK   (dd/mm/yyyy)\n");
		fprintf(stderr,"		2: USA  (mm/dd/yyyy)\n");
		fprintf(stderr,"		3: JP   (yyyy/mm/dd)\n");
		fprintf(stderr,"		4: LDAS (yyyymmdd)\n");
		fprintf(stderr,"		...2-digit-year formats...\n");
		fprintf(stderr,"		5: (yymmdd)\n");
		fprintf(stderr,"		6: (yy:mm:d)\n");
		fprintf(stderr,"		...other formats...\n");
		fprintf(stderr,"		7: week 0-52, Monday=1st day of week\n");
		fprintf(stderr,"		8: day 1-365\n");
		fprintf(stderr,"	-z: date-zero (for -o 7 or 8) - must match -i format [%s]\n",setzero);
		fprintf(stderr,"	-v: verbose output (0=NO 1=YES 999=DEBUG) [%d]\n",setverb);
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s dates.txt -i 1\n",thisprog);
		fprintf(stderr,"	echo 12-31-1999 | %s stdin -i 2\n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	one modified date per valid input line\n");
		fprintf(stderr,"	example:  19991231\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}


	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	infile= argv[1];
	for(ii=2;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-c")==0) setcoldate= atol(argv[++ii]);
			else if(strcmp(argv[ii],"-i")==0) setin= atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-o")==0) setout= atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-z")==0) setzero= argv[++ii];
			else if(strcmp(argv[ii],"-h")==0) sethead= atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-v")==0) setverb= atoi(argv[++ii]);
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
	}}
	if(setcoldate<1)  {fprintf(stderr,"\n--- Error[%s]: input date column (-c %ld) - must be >0\n\n",thisprog,setcoldate);exit(1);}
	if(setin<1||setin>4)  {fprintf(stderr,"\n--- Error[%s]: input format (-i %d) must be 1-4\n\n",thisprog,setin);exit(1);}
	if(setout<1||setout>8) {fprintf(stderr,"\n--- Error[%s]: output format (-o %d) must be 1-8\n\n",thisprog,setout);exit(1);}
	if(setzero!=NULL && (setout!=7&&setout!=8)) {fprintf(stderr,"\n--- Error[%s]: -z (%s) incmpatible with -o (%d): use -o 7 or 8\n\n",thisprog,setzero,setout);exit(1);}
	if(sethead!=0 && sethead!=1) { fprintf(stderr,"\n--- Error [%s]: invalid -h [%d] must be 0 or 1\n\n",thisprog,sethead);exit(1);}
	if(setverb!=0 && setverb!=1 && setverb != 999) { fprintf(stderr,"\n--- Error [%s]: invalid -v [%d] must be 0,1, or 999\n\n",thisprog,setverb);exit(1);}

	/* modified date column (zero offset) - for reduced maths later */
	scdm1= setcoldate-1;

	/* determine adjustment for outputs 7 (week) or day (8) */
	if(setzero!=NULL) {
		z= xf_dateparse1(setzero,setin,&year,&month,&day,message);
		if(z==-1) { fprintf(stderr,"\n*** %s/%s\n\n",thisprog,message); exit(1); }
		if(setout==7) adj1= xf_dateconv1(year,month,day,1,message);
		if(setout==8) adj1= xf_dateconv1(year,month,day,2,message);
		if(adj1==-1) { fprintf(stderr,"\n*** %s/%s\n\n",thisprog,message); exit(1); }
	}

	/* READ THE DATA */
	mm=0;
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
	while((line1=xf_lineread1(line1,&maxlen,fpin))!=NULL) {
		if(maxlen==-1)  {fprintf(stderr,"\n--- Error[%s]: readline function encountered insufficient memory\n\n",thisprog);exit(1);}
		/* preserve header and leading blank lines if required */
		if(sethead==1) { if(mm==0) { printf("%s",line1); if(strlen(line1)>1) mm++; continue; }}
		/* make a temporary copy of the line before parsing it */
		if(maxlen>prevlen) { prevlen=maxlen; line2= realloc(line2,(maxlen+1)*sizeofline2); if(line2==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}}
		strcpy(line2,line1);
		/* parse the line */
		iword= xf_lineparse2(line1,"\t",&nwords);
		if(nwords<0) {fprintf(stderr,"\n--- Error[%s]: lineparse function encountered insufficient memory\n\n",thisprog);exit(1);};
		/* make sure required columns are present */
		if(nwords<setcoldate) {printf("%s",line2);continue;}
		/* scan the line looking for the date column to be modified */
		for(word=0;word<nwords;word++) {
			if(word>0) printf("\t");
			if(word==scdm1) {
				/* parse the word into year, month, date */
				z= xf_dateparse1((line1+iword[word]),setin,&year,&month,&day,message);
				if(z==-1) { if(setverb==1) fprintf(stderr,"*** %s/%s\n",thisprog,message); printf("NAN"); continue; }
				/* output */
				year2= year%100;
				if(setout==1) printf("%02d/%02d/%04d",day,month,year);
				else if(setout==2) printf("%02d/%02d/%04d",month,day,year);
				else if(setout==3) printf("%04d/%02d/%02d",year,month,day);
				else if(setout==4) printf("%04d%02d%02d",year,month,day);
				else if(setout==5) printf("%02d%02d%02d",year2,month,day);
				else if(setout==6) printf("%02d:%02d:%02d",year2,month,day);
				else if(setout==7) { // week-in-year, 0-52, 0= before first Monday
					z= xf_dateconv1(year,month,day,1,message);
					if(z>=0) printf("%d",(z-adj1));
					else { if(setverb==1) fprintf(stderr,"*** %s/%s\n",thisprog,message); printf("NAN"); continue; }

				}
				else if(setout==8) { // day-in-year
					z= xf_dateconv1(year,month,day,2,message);
					if(z>0) printf("%d",(z-adj1));
					else { if(setverb==1) fprintf(stderr,"*** %s/%s\n",thisprog,message); printf("NAN"); continue; }
				}
			}
			else printf("%s",(line1+iword[word]));
		}
		printf("\n");
	}

	/* TIDY UP AND EXIT */
	if(strcmp(infile,"stdin")!=0) fclose(fpin);
	if(line1!=NULL) free(line1);
	if(line2!=NULL) free(line2);
	if(iword!=NULL) free(iword);
	exit(0);
}
