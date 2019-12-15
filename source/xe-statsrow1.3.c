#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>

#define thisprog "xe-statsrow1"
#define TITLE_STRING thisprog" v 3: 24.November.2018 [JRH]"
#define MAXLINELEN 1000

/*
<TAGS>math stats</TAGS>
v 3: 24.November.2018 [JRH]
	- fix uninitialized value for nn in some conditions
	- update variable namig and infile definition

v 3: 24.January.2017 [JRH]
	- add ability to detect peak value (-t 5) or sample (-t 6)

v 3: 15.March.2015 [JRH]
	- improved speed by making separate lops for each type of output

v 2: 24 October.2014 [JRH]
	- added option to output standard deviation
*/


/* external functions start */
char *xf_lineread1(char *line, long *maxlinelen, FILE *fpin);
/* external functions end */

int main (int argc, char *argv[]) {
	/* general variables */
	char *line=NULL,*pline,*pcol;
	long ii,jj,kk,nn;
	int v,w,x,y,z,col,colmatch;
	long maxlinelen=0;
	double aa,bb;
	FILE *fpin,*fpout;
	/* program-specific variables */
	double sum,sumsquares;
	/* arguments */
	char *infile=NULL;
	int setstat=3;

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Calculate the stats on individual rows of numbers\n");
		fprintf(stderr,"Non-numeric values will be ignored\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [input]\n",thisprog);
		fprintf(stderr,"	[input]: file name or \"stdin\"\n");
		fprintf(stderr,"VALID OPTIONS:\n");
		fprintf(stderr,"	-t(ype) of statistic to output: [%d]\n",setstat);
		fprintf(stderr,"		1=counts\n");
		fprintf(stderr,"		2=sum\n");
		fprintf(stderr,"		3=mean\n");
		fprintf(stderr,"		4=standard deviation\n");
		fprintf(stderr,"		5=maximum value\n");
		fprintf(stderr,"		6=sample-number corresponding to maximum value\n");
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	one statistic per row\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	infile= argv[1];
	for(ii=2;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-t")==0)   setstat=atoi(argv[++ii]);
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
	}}

	if(setstat<1||setstat>6) {fprintf(stderr,"\n--- Error[%s]: invalid statistic-type (%d), must be 1-6\n\n",thisprog,setstat); exit(1);}

	/* STORE DATA */
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}

	/* output the number-count for each line */
	if(setstat==1) {
		while((line=xf_lineread1(line,&maxlinelen,fpin))!=NULL) {
			if(maxlinelen==-1)  {fprintf(stderr,"\n--- Error[%s]: readline function encountered insufficient memory\n\n",thisprog);exit(1);}
			pline=line;
			nn=0;
			for(col=1;(pcol=strtok(pline," ,\t\n\r"))!=NULL;col++) {
				pline=NULL;
				if(sscanf(pcol,"%lf",&aa)==1) nn++;
			}
			printf("%ld\n",nn);
		}
	}

	/* output the sum for each line (requires finite values) */
	if(setstat==2) {
		while((line=xf_lineread1(line,&maxlinelen,fpin))!=NULL) {
			if(maxlinelen==-1)  {fprintf(stderr,"\n--- Error[%s]: readline function encountered insufficient memory\n\n",thisprog);exit(1);}
			pline=line;
			sum=0.0; nn=0;
			for(col=1;(pcol=strtok(pline," ,\t\n\r"))!=NULL;col++) {
				pline=NULL;
				if(sscanf(pcol,"%lf",&aa)==1) {
					if(isfinite(aa)) {
					sum+=aa;
					nn++;
			}}}
			if(nn>0) printf("%lf\n",sum);
			else printf("NAN\n");
		}
	}

	/* output the mean for each line (requires finite values) */
	if(setstat==3) {
		while((line=xf_lineread1(line,&maxlinelen,fpin))!=NULL) {
			if(maxlinelen==-1)  {fprintf(stderr,"\n--- Error[%s]: readline function encountered insufficient memory\n\n",thisprog);exit(1);}
			pline=line;
			sum=0.0; nn=0;
			for(col=1;(pcol=strtok(pline," ,\t\n\r"))!=NULL;col++) {
				pline=NULL;
				if(sscanf(pcol,"%lf",&aa)==1) {
					if(isfinite(aa)) {
					sum+=aa;
					nn++;
			}}}
			if(nn>0) printf("%lf\n",(sum/(double)nn));
			else printf("NAN\n");
		}
	}

	/* output the std.deviation for each line (requires finite values) */
	if(setstat==4) {
		while((line=xf_lineread1(line,&maxlinelen,fpin))!=NULL) {
			if(maxlinelen==-1)  {fprintf(stderr,"\n--- Error[%s]: readline function encountered insufficient memory\n\n",thisprog);exit(1);}
			pline=line;
			sum=sumsquares=0.0; nn=0;
			for(col=1;(pcol=strtok(pline," ,\t\n\r"))!=NULL;col++) {
				pline=NULL;
				if(sscanf(pcol,"%lf",&aa)==1) {
					if(isfinite(aa)) {
					sum+=aa;
					sumsquares+=aa*aa;
					nn++;
			}}}
			if(nn==0) printf("NAN\n");
			else {
				if(nn>1)	bb= ( sumsquares - ( (sum*sum)/(double)nn )) / ((double)nn-1.0) ;
				else bb=0.0;
				printf("%lf\n",(sqrt(bb)));
		}}
	}
	/* output the sample-number corresponding to the peak for each line (requires finite values) */
	if(setstat==5||setstat==6) {
		while((line=xf_lineread1(line,&maxlinelen,fpin))!=NULL) {
			if(maxlinelen==-1)  {fprintf(stderr,"\n--- Error[%s]: readline function encountered insufficient memory\n\n",thisprog);exit(1);}
			kk= nn= 0;
			bb=DBL_MIN; /* default maximum */
			pline=line;
			for(col=0;(pcol=strtok(pline," ,\t\n\r"))!=NULL;col++) {
				pline=NULL;
				if(sscanf(pcol,"%lf",&aa)==1) {
					if(isfinite(aa)) {
					if(aa>bb) { bb=aa; kk=col; }
					nn++;
			}}}
			if(nn==0) printf("NAN\n");
			else {
				if(setstat==5) printf("%lf\n",bb);
				if(setstat==6) printf("%ld\n",kk);
		}}
	}



	if(strcmp(infile,"stdin")!=0) fclose(fpin);

	free(line);
	exit(0);
	}
