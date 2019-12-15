#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define thisprog "xe-getdelta2"
#define TITLE_STRING thisprog" v 1: 3.June.2016 [JRH]"
#define MAXLINELEN 1000

/*
<TAGS>math</TAGS>
v 1: 3.June.2016 [JRH]
	- allow specification of input numerical type (integer or float)
	- add check for whether input is finite before outputting difference

v 1: 26.October.2012 [JRH]
	- calculate column2-column1 "
*/



int main (int argc, char *argv[]) {
	/* general variables */
	char infile[256],line[MAXLINELEN];
	long ii,jj;
	double aa,bb;
	FILE *fpin;
	/* arguments */
	int settype=2;

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Produce the change (delta) between values in a pair of columns\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [input] [options]\n",thisprog);
		fprintf(stderr,"	[input]: file name or \"stdin\", double-column of numbers\n");
		fprintf(stderr,"VALID OPTIONS: defaults in []\n");
		fprintf(stderr,"	-t(ype) of numbers: 1=integers 2=float [%d]\n",settype);
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s data.txt\n",thisprog);
		fprintf(stderr,"	cat temp.txt | %s stdin\n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	delta scores\n");
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
		while(fgets(line,MAXLINELEN,fpin)!=NULL) {
			if(sscanf(line,"%ld %ld",&ii,&jj)!=2) printf("nan\n");
			else printf("%ld\n",jj-ii);


	}}
	if(settype==2) {
		while(fgets(line,MAXLINELEN,fpin)!=NULL) {
			if(sscanf(line,"%lf %lf",&aa,&bb)!=2) printf("nan\n");
			else if(!isfinite(aa) && !isfinite(bb)) printf("nan\n");
			else printf("%g\n",bb-aa);
	}}

	if(strcmp(infile,"stdin")!=0) fclose(fpin);

	exit(0);
	}
