#define thisprog "xe-math_sum1"
#define TITLE_STRING thisprog" v 4: 27.November.2017 [JRH]"
#define MAXLINELEN 1000
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
<TAGS>math </TAGS>


v 4: 27.November.2017 [JRH]
	- switch to using %f for output to avoid precision loss with exponential notation
v 4: 31.August.2016 [JRH]
	- add support for long integer input
	- update variable usage
v 4: 21.May.2013 [JRH]
	- bugfix: bb variable (sum) is now properly initialized to zero
v 3: 14.May.2013 [JRH]
	- bugfix: now report error if there is no input
v 2: 14.August.2012 [JRH]
	- bugfix - changed use of fscanf to read data with fgets/sscanf, to avoid problems related to "-" and "."
*/


/* external functions start */
/* external functions end */

int main (int argc, char *argv[]) {
	/* general variables */
	char infile[256],line[MAXLINELEN];
	long int ii,jj,kk,nn;
	int v,w,x,y,z,col,colmatch;
	double aa,bb;
	FILE *fpin,*fpout;
	/* program-specific variables */
	/* arguments */
	int setlong=0;

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Calculate the sum of a stream of numbers\n");
		fprintf(stderr,"Non-numeric values will be ignored\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [input] [options]\n",thisprog);
		fprintf(stderr,"	[input]: filename or \"stdin\", single-column of numbers\n",thisprog);
		fprintf(stderr,"VALID OPTIONS (defaults) in []:\n");
		fprintf(stderr,"	-long: assume input is long-integers (0=NO 1=YES) [%d]\n",setlong);
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s data.txt\n",thisprog);
		fprintf(stderr,"	cut -f 1 data.txt | %s stdin\n",thisprog);
		fprintf(stderr,"	echo \"1 2 3 4 5\" | %s stdin\n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	the sum\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	sprintf(infile,"%s",argv[1]);
	for(ii=2;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-long")==0) setlong=atoi(argv[++ii]);
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
	}}
	if(setlong!=0&&setlong!=1) {fprintf(stderr,"\n--- Error[%s]: invalid -long (%d): must be 0 or 1\n\n",thisprog,setlong);exit(1);}

	/* OPEN THE INPUT  */
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}

	if(setlong==0) {
		nn=0; bb=0.0;
		while(fgets(line,MAXLINELEN,fpin)!=NULL) {
			if(sscanf(line,"%lf",&aa)==1 && isfinite(aa)) {bb+=aa; nn++;}
	}}
	if(setlong==1) {
		nn=0; jj=0;
		while(fgets(line,MAXLINELEN,fpin)!=NULL) {
			if(sscanf(line,"%ld",&ii)==1) {jj+=ii; nn++;}
	}}

	if(strcmp(infile,"stdin")!=0) fclose(fpin);

	if(nn<1){ printf("%g\n",NAN); fprintf(stderr,"\n--- Error[%s]: no input data\n\n",thisprog); exit(1); }

	else {
		if(setlong==0) printf("%f\n",bb);
		if(setlong==1) printf("%ld\n",jj);
	}

	exit(0);
	}
