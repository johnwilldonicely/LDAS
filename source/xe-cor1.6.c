#define thisprog "xe-cor1"
#define TITLE_STRING thisprog" v 6, 17.April.2016 [JRH]"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define MAXLINELEN 1000

/*
<TAGS>stats</TAGS>

CHANGES:
v 6, 17.April.2016 [JRH]
	- add explicit fclose command at end of file read - somehow this had been omitted!
	- add explicit exit at end
	- add note that data is stored in memory
v 6, 26.October.2012 [JRH]
	- add xmin/xmax option
v 5, 11.June.2012 [JRH]
	- add output of n to beginning of outout line
v 1.4 8.February.2012 [JRH]
	- use more specific call to xf_correlate_simple_d instead of xf_correlate_simple
v 1.3 24.August.2011 [JRH]
	- removed unused -v 2 option (plot)

v 1.2 21.July.2011 [JRH]
	- uses slightly more informative version of xf_correlate_simple - includes slope and intercept output
*/

/* external functions start */
double xf_correlate_simple_d(double *x, double *y, long n, double *result_d);
/* external functions end */

int main(int argc, char *argv[]) {

	/* general variables */
	char infile[256],outfile[256],line[MAXLINELEN],templine[MAXLINELEN],word[256],*matchstring=NULL,*pline,*pcol;
	long int i,j,k,n;
	int v,w,x,y,z,col,colmatch;
	int sizeofchar=sizeof(char),sizeofshort=sizeof(short),sizeoflong=sizeof(long),sizeofint=sizeof(int),sizeoffloat=sizeof(float),sizeofdouble=sizeof(double);
	float a,b,c,d;
	double aa,bb,cc,dd,result_d[32];
	FILE *fpin,*fpout;
	/* program-specific variables */
	long nlines=0, nskipped=0;
	double *xdat=NULL,*ydat=NULL,r=0.0;
	/* arguments */
	int setcolx=1,setcoly=2,setverb=0;
	double setxmin=NAN,setxmax=NAN;

	if(argc==1) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Simple Pearson's correlation between a pair of columns (fast)\n");
		fprintf(stderr,"	- filters-out non-numeric data, NaN and Inf\n");
		fprintf(stderr,"	- will report r=0 if n<2, and r=1 if n=2\n");
		fprintf(stderr,"	- reads data into memory: for low-memory, use xe-correlate\n");
		fprintf(stderr,"	- Not siutable for large matrices - cut columns out first\n");
		fprintf(stderr,"\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [filename] [options]\n",thisprog);
		fprintf(stderr,"		[filename]: file name or \"stdin\"\n");
		fprintf(stderr,"VALID OPTIONS [DEFAULT IN BRACKETS]...\n");
		fprintf(stderr,"	-cx column containing independent variable [%d]\n",setcolx);
		fprintf(stderr,"	-cy column containing dependent variable [%d]\n",setcoly);
		fprintf(stderr,"	-xmin minimum value of independent value to include [%g]\n",setxmin);
		fprintf(stderr,"	-xmax maximum value of independent value to include [%g]\n",setxmax);
		fprintf(stderr,"	-v sets verbosity of output (0=single line, 1=full report) [%d]\n",setverb);
		fprintf(stderr,"\n");
		fprintf(stderr,"EXAMPLE:\n");
 		fprintf(stderr,"	%s temp.dat -cx 5 -cy 7\n",thisprog);
		fprintf(stderr,"OUTPUT: n, r, intercept and slope\n");
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	sprintf(infile,"%s",argv[1]);
	for(i=2;i<argc;i++) {
		if( *(argv[i]+0) == '-') {
			if((i+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[i]); exit(1);}
			else if(strcmp(argv[i],"-cx")==0) 	{ setcolx=atoi(argv[i+1]); i++;}
			else if(strcmp(argv[i],"-cy")==0) 	{ setcoly=atoi(argv[i+1]); i++;}
			else if(strcmp(argv[i],"-xmin")==0) 	{ setxmin=atof(argv[i+1]); i++;}
			else if(strcmp(argv[i],"-xmax")==0) 	{ setxmax=atof(argv[i+1]); i++;}
			else if(strcmp(argv[i],"-v")==0) 	{ setverb=atoi(argv[i+1]); i++;}
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[i]); exit(1);}
	}}

	/* READ THE DATA - SPECIFIC COLUMNS HOLD DATA  */
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
	n=0;
	while(fgets(line,MAXLINELEN,fpin)!=NULL) {
		nlines++;
		if(line[0]=='#') {nskipped++; continue;}
		pline=line; colmatch=2; // number of columns to match
		for(col=1;(pcol=strtok(pline," ,\t\n"))!=NULL;col++) {
			pline=NULL;
			if(col==setcolx && sscanf(pcol,"%lf",&aa)==1) colmatch--; // store value - check if input was actually a number
			if(col==setcoly && sscanf(pcol,"%lf",&bb)==1) colmatch--; // store value - check if input was actually a number
		}
		if( colmatch!=0 || !isfinite(aa) || !isfinite(bb) ) { nskipped++; continue;}
		if( (isfinite(setxmin) && aa <setxmin) || (isfinite(setxmax) && aa>setxmax)) { nskipped++; continue;}
		if((xdat=(double *)realloc(xdat,(n+1)*sizeofdouble))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		if((ydat=(double *)realloc(ydat,(n+1)*sizeofdouble))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		xdat[n]=aa;
		ydat[n]=bb;
		n++;
	}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);

	/*CALCULATE THE CORRELATION IF N>1 (IF N=1, R=0. IF N=2, R=1) */
	if(n>1) r=xf_correlate_simple_d(xdat,ydat,n,result_d);
	else r=NAN;

	/* OUTPUT THE RESULTS */
	if(setverb==0) printf("%ld\t%.5f\t%.5f\t%.5f\n",n,r,result_d[0],result_d[1]);
	else {
		printf("lines= %ld\n",nlines);
		printf("skipped= %ld\n",nskipped);
		printf("n= %ld\n",n);
		printf("r= %.5f\n",r);
		printf("intercept= %.5f\n",result_d[0]);
		printf("slope= %.5f\n",result_d[1]);
	}

	exit(0);

}
