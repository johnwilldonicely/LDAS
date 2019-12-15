#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define thisprog "xe-getdelta1"
#define TITLE_STRING thisprog" v 2: 3.June.2016 [JRH]"
#define MAXLINELEN 1000

/*
<TAGS>math</TAGS>
v 2: 3.June.2016 [JRH]
	- allow specification of input numerical type (integer or float)
	- change output to "nan" if invalid, rather than "-"

v 2: 14.August.2012 [JRH]
	- bugfix - changed use of fscanf to read data with fgets/sscanf, to avoid problems related to "-" and "."
*/



int main (int argc, char *argv[]) {
	/* general variables */
	char infile[256],outfile[256],line[MAXLINELEN],templine[MAXLINELEN],word[256],*matchstring=NULL,*pline,*pcol;
	long ii,jj,goodii,goodjj;
	double aa,bb,cc,dd;
	FILE *fpin,*fpout;
	/* arguments */
	int settype=2;

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Produce the change (delta) between consecutive values in a column\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [input] [options]\n",thisprog);
		fprintf(stderr,"	[input]: file name or \"stdin\", single column of numbers\n");
		fprintf(stderr,"VALID OPTIONS: defaults in []\n");
		fprintf(stderr,"	-t(ype) of numbers: 1=integers 2=float [%d]\n",settype);
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s data.txt -t 1\n",thisprog);
		fprintf(stderr,"	cat temp.txt | %s stdin -t 2\n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	n-1 delta scores\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	sprintf(infile,"%s",argv[1]);
	for(ii=2;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-t")==0)   settype=atoi(argv[++ii]);
			else {fprintf(stderr,"\n*** %s [ERROR: invalid command line argument \"%s\"]\n\n",thisprog,argv[ii]); exit(1);}
	}}
	if(settype!=1 && settype!=2) { fprintf(stderr,"\n--- Error [%s]: invalid input type (it %d): must be 1 or 2\n\n",thisprog,settype);exit(1);}

	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
	if(settype==1) {
		goodii=goodjj=0; // goodii is a marker for a previously good value being found
		while(fgets(line,MAXLINELEN,fpin)!=NULL) {
			if(sscanf(line,"%ld",&ii)==1) goodii=1; else goodii=0;
			if(goodii && goodjj) printf("%ld\n",(ii-jj));
			else printf("nan\n");
			jj=ii;
			goodjj=goodii;
	}}
	else if(settype==2) {
		aa=bb=NAN;
		while(fgets(line,MAXLINELEN,fpin)!=NULL) {
			if(sscanf(line,"%lf",&aa)!=1) aa=NAN;
			if(isfinite(aa) && isfinite(bb)) printf("%g\n",(aa-bb));
			else printf("nan\n");
			bb=aa;
	}}

	if(strcmp(infile,"stdin")!=0) fclose(fpin);

	exit(0);
	}
