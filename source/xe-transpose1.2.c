#define thisprog "xe-transpose1"
#define TITLE_STRING thisprog" v 1.2: 11.October.2016 [JRH]"
#define MAXLINELEN 1000

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
<TAGS>file</TAGS>

v 1.2: 11.October.2016 [JRH]
	- add capability to handle long-line input
		- make use of lineread and lineparse functions
	- update variable naming

v 1.2: 19.July.2012 [JRH]
	- add option to suppress column-number output (-n)

*/


/* external functions start */
char *xf_lineread1(char *line, long *maxlinelen, FILE *fpin);
long *xf_lineparse1(char *line,long *nwords);
long *xf_lineparse2(char *line,char *delimiters, long *nwords);
char *xf_strescape1(char *line);
/* external functions end */

int main (int argc, char *argv[]) {
	/* general variables */
	char outfile[256],*line=NULL,*templine=NULL,word[256],*matchstring=NULL,*pline,*pcol;
	long int ii,jj,kk,nn;
	int v,w,x,y,z,col,colmatch;
	int sizeofchar=sizeof(char),sizeofshort=sizeof(short),sizeoflong=sizeof(long),sizeofint=sizeof(int),sizeoffloat=sizeof(float),sizeofdouble=sizeof(double);
	float a,b,c,d;
	double aa,bb,cc,dd;
	FILE *fpin,*fpout;
	/* program-specific variables */
	long *index=NULL,maxlinelen=0,nwords=0;
	/* arguments */
	char *infile=NULL,delimiters[MAXLINELEN],delimout[4];
	int setdelimiters=0;
	long setstart=1,setcolnum=1;

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"- Transpose multiple columns into column-number [tab] column-contents\n");
		fprintf(stderr,"- this program will handle long-line input\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [input] [options]\n",thisprog);
		fprintf(stderr,"	[input]: file name or \"stdin\"\n");
		fprintf(stderr,"VALID OPTIONS:\n");
		fprintf(stderr,"	-start : first column to transpose\n");
		fprintf(stderr,"	     NOTE: preceeding columns will be output for every line\n");
		fprintf(stderr,"	-n: output original column numbers (0=NO 1=YES) [%ld]\n",setcolnum);
		fprintf(stderr,"	[-d]: characters to use as column-delimiters\n");
		fprintf(stderr,"		- unset by default: multiple whitespace treated as one\n");
		fprintf(stderr,"		- if set, any matching delimiter marks a new column\n");
		fprintf(stderr,"			- suitable for files with \"empty\" columns\n");
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s data.txt \n",thisprog);
		fprintf(stderr,"	cat temp.txt | %s stdin -start 3 -n 0 \n",thisprog);
		fprintf(stderr,"OUTPUT (by column):\n");
		fprintf(stderr,"	non-transposed columns (if -start defined)\n");
		fprintf(stderr,"	subsequent column numbers ( if -n is set to 1)\n");
		fprintf(stderr,"	subsequent column contents\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}


	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	infile= argv[1];
	for(ii=2;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-start")==0) setstart=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-n")==0)     setcolnum=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-d")==0)  {
				setdelimiters=1;
				sprintf(delimiters,"%s",xf_strescape1(argv[++ii]));
			}
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
	}}

	if(setstart<1) {fprintf(stderr,"\n--- Error[%s]: -start (%ld) must be >0\n\n",thisprog,setstart); exit(1);}
	if(setcolnum!=0&&setcolnum!=1) {fprintf(stderr,"\n--- Error[%s]: -n (%ld) must be 0 or 1\n\n",thisprog,setcolnum); exit(1);}
	if(setdelimiters==0) sprintf(delimout,"\t");
	else if(setdelimiters==1) sprintf(delimout,"%c",delimiters[0]);

	/* READ DATA */
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}

 	while((line=xf_lineread1(line,&maxlinelen,fpin))!=NULL) {
 		if(maxlinelen==-1)  {fprintf(stderr,"\n--- Error[%s]: readline function encountered insufficient memory\n\n",thisprog);exit(1);}
		if((templine=(char *)realloc(templine,maxlinelen))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};

		if(setdelimiters==0) index= xf_lineparse1(line,&nwords);
		else index= xf_lineparse2(line,delimiters,&nwords);
		if(index==NULL) {
			if(nwords==0)  { fprintf(stderr,"\n--- Error[%s]: remove blank lines from input before transposing\n\n",thisprog);exit(1);}
			if(nwords==-1) { fprintf(stderr,"\n--- Error[%s]: lineparser function encountered an error\n\n",thisprog);exit(1);}
		}

		/* pre-fill templine with the non-transposed columns (may be none) */
		if(setstart>1) {
			for(ii=jj=0;ii<(setstart-1);ii++) { sprintf((templine+jj),"%s%s",(line+index[ii]),delimout); jj+=strlen((line+index[ii]))+1; }
		}
		//TEST: printf("%s***\n",templine); continue;

		/* output one line for each transposed column, prefixing with the non-transposed columns if necessary */
		/* note that templine will be empty if -start was set to 1 */
		for(ii=(setstart-1);ii<nwords;ii++) {
			if(setcolnum==0) printf("%s%s\n",templine,(line+index[ii]));
			if(setcolnum==1) printf("%s%ld%s%s\n",templine,(ii+1),delimout,(line+index[ii]));
		}
	}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);

	if(line!=NULL) free(line);
	if(templine!=NULL) free(templine);
	if(index!=NULL) free(index);
	exit(0);
	}
