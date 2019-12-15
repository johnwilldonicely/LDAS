#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define thisprog "xe-statsgrp1"
#define TITLE_STRING thisprog" v 9: 29.April.2019 [JRH]"

/*
<TAGS>math stats</TAGS>

v 9: 29.April.2019 [JRH]
	- rework so "grep -vE" can be used to generate code for xe-statsgroup2 and xe-statsgroup1 from xe-statsgroup3

	VALIDATION:
	in1=/opt/LDAS/docs/sample_data/sample_group_x_time.txt
	list1=$(xe-cut1 $in1 subject -o 1 | sort -nu)
	list2=$(xe-cut1 $in1 group -o 1 | sort -nu)
	list3=$(xe-cut1 $in1 dname -o 1 | sort -nu)
	for i in $list1 ; do for j in $list2 ; do for k in $list3 ; do xe-dbmatch1 $in1 subject $i | xe-dbmatch1 stdin group $j | xe-dbmatch1 stdin dname $k | xe-cut1 stdin gamma -o 1 | xe-statsd1 stdin | xe-getkey stdin MEAN | awk '{print "'$i'\t'$j'\t'$k'\t",$1}' ; done ; done ; done
	...compare with...

v 9: 28.April.2019 [JRH]
	- bugfix: check for finite data before adding it to the temporary data array
	- bugfix: initialize results to NAN before calling stats function
	- use xf_lineread1 and xf_lineparse2 so there is no limit to line-length
	- add checks for valid column-specification

v 8: 4.April.2019 [JRH]
	- use newer variable naming conventions
	- retire unused word, bin setlow, sethigh, grp variables
	- upgrade group-variables to double
	- fix instructions

v 8: 5.May.2013 [JRH]
	- update usage of qsort to call external compare function xf_compare1_d

v 7: 10.February.2013 [JRH]
	- bugfix - previous versions omitted header for group3
	- remove leading blank line from output

v 6: 15.January.2013 [JRH]
	- bugfix - switched from double compare function to float compare function, as appropriate for floating point precision grouping variables
	- this led to some pretty bizarre output previously!

v 5: 3.December.2012 [JRH]
	- switch to using built-in qsort function
	- retire unused word, and bin variables

v 4: 28.October.2012 [JRH]
	- bugfix - was doubling output from first group, because "a" was being initialized to grp[0] instead of listgrp[0] before eliminating duplicates in listgrp[]

v 3: 28.September.2012 [JRH]

v 2: 24.September.2012 [JRH]
	resolve a few minor memory allocation and freeing issues

v 1: 9.September.2012 [JRH]
	A program to calculate stats on a variable using 3 grouping variable
	NOTE: I tried having flexible column-numbers - just too complicated for data storage esp. if we want to pipe input to the program
*/


/* external functions start */
char *xf_lineread1(char *line, long *maxlinelen, FILE *fpin);
long *xf_lineparse2(char *line,char *delimiters, long *nwords);
void xf_stats2_d(double *data, long n, int varcalc, double *result_d);
int xf_compare1_d(const void *a, const void *b);
/* external functions end */

int main (int argc, char *argv[]) {
	/* general variables */
	char *line=NULL;
	long int ii,jj,kk,mm,nn,maxlinelen=0;
	double aa,bb,cc,dd,result_d[64];
	FILE *fpin;
	/* program-specific variables */
	double *grp1=NULL,*listgrp1=NULL; long nlistgrp1=0; int sizeofgrp1=sizeof(*grp1); double tempgrp1;
	long nwords=0,*iword=NULL,colmatch;
	long ntempdata=0,ngrptot=0;
	double *data=NULL,*tempdata=NULL;
	int sizeofdata=sizeof(*data);
	/* arguments */
	char *infile=NULL;
	int setgint=0;
	long setcolgrp1=1;
	long setcoldata;

	setcoldata= setcolgrp1+1;

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Calculate stats on a data-column using grouping-columns\n");
		fprintf(stderr,"- input must be tab-delimited\n");
		fprintf(stderr,"- grouping-variables must be numeric (can be floating-point)\n");
		fprintf(stderr,"- non-numeric data-values will be ignored for stats calculations\n");
		fprintf(stderr,"\n");
		fprintf(stderr,"USAGE: %s [input] [options]\n",thisprog);
		fprintf(stderr,"	[input]: file name or \"stdin\"\n");
		fprintf(stderr,"VALID OPTIONS:\n");
		fprintf(stderr,"	-cg1:  column defining grouping-variable 1  [%ld]\n",setcolgrp1);
		fprintf(stderr,"	-cy:   column containing dependent variable [%ld]\n",setcoldata);
		fprintf(stderr,"	-gint: output groups as integers? (0=NO 1=YES) [%d]\n",setgint);
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s data.txt",thisprog);
		fprintf(stderr," -cg1 5");
		fprintf(stderr,"\n",thisprog);
		fprintf(stderr,"	cat temp.txt | %s stdin -gint 1\n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"\tgrp1");
		fprintf(stderr,"\tn	mean	sd	sem	ntot\n");
		fprintf(stderr,"\n");
		fprintf(stderr,"	NOTE:\n");
		fprintf(stderr,"		ntot= total datapoints for a given group-combination\n");
		fprintf(stderr,"		n= valid numbers contributing to statistical result\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	infile= argv[1];
	for(ii=2;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-cg1")==0)  setcolgrp1= atol(argv[++ii]);
			else if(strcmp(argv[ii],"-cy")==0)   setcoldata= atol(argv[++ii]);
			else if(strcmp(argv[ii],"-gint")==0) setgint= atoi(argv[++ii]);
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n",thisprog,argv[ii]); exit(1);}
	}}
	if(setcolgrp1<1) {fprintf(stderr,"\n--- Error[%s]: invalid group column (-cg1 %ld) - must be >0\n",thisprog,setcolgrp1); exit(1);}
	if(setcoldata<1) {fprintf(stderr,"\n--- Error[%s]: invalid data column (-cy %ld) - must be >0\n",thisprog,setcoldata); exit(1);}

	/* DECREMENT COLUMN-NUMBERS SO THEY'RE ZERO-OFFSET */
	setcolgrp1--;
	setcoldata--;

	/* STORE DATA */
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
	nn=0;
	dd=NAN;
	tempgrp1=NAN;
	while((line=xf_lineread1(line,&maxlinelen,fpin))!=NULL) {
		if(maxlinelen==-1)  {fprintf(stderr,"\n--- Error[%s]: readline function encountered insufficient memory\n\n",thisprog);exit(1);}
		if(line[0]=='#') continue;
		/* parse the line & make sure all required columns are present */
		iword= xf_lineparse2(line,"\t",&nwords);
		if(nwords<0) {fprintf(stderr,"\n--- Error[%s]: lineparse function encountered insufficient memory\n\n",thisprog);exit(1);};
		if(
			nwords<setcoldata
			|| nwords<setcolgrp1
			) continue;
		/* make sure each group-columns are numeric & finite, and convert non-numeric data to NAN */
		sscanf(line+iword[setcolgrp1],"%lf",&tempgrp1);
		sscanf(line+iword[setcoldata],"%lf",&dd);
		if(
			!isfinite(tempgrp1)
		) continue;
		/* reallocate memory */
		data= realloc(data,(nn+1)*sizeofdata);
		grp1= realloc(grp1,(nn+1)*sizeofgrp1);
		if(
			data==NULL
			|| grp1==NULL
			) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		/* assign values */
		data[nn]= dd;
		grp1[nn]= tempgrp1;
		nn++;
	}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);


	/* ALLOCATE MEMORY FOR LISTS AND TEMPDATA */
	listgrp1= realloc(listgrp1,(nn+1)*sizeofgrp1);
	tempdata= realloc(tempdata,(nn+1)*sizeofdata);
	if(
		listgrp1==NULL
	) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};


	/* CREATE A SORTED LIST OF THE ELEMENTS IN grp1 */
	for(ii=0;ii<nn;ii++) listgrp1[ii]= grp1[ii];
	qsort(listgrp1,nn,sizeof(double),xf_compare1_d);
	/* copy only unique items to new version of listgrp1 */
	aa=listgrp1[0]; for(ii=nlistgrp1=1;ii<nn;ii++) {if(listgrp1[ii]!=aa) listgrp1[nlistgrp1++]=listgrp1[ii];aa=listgrp1[ii]; }





	/* CALCULATE STATS ON DATA IN EACH COMBINATION OF GROUP-CATEGORIES */
	printf("grp1\t");
	printf("n\tmean\tsd\tsem\tntot\n");
	for(ii=0;ii<nlistgrp1;ii++)
	{
		//printf("%g\n",listgrp1[ii]);
		{
			{
				ngrptot= 0;
				result_d[0]=result_d[2]=result_d[3]= NAN;
				/* copy the good data for this category to a temp array */
				ntempdata=0;
				for(mm=0;mm<nn;mm++) {
					if(
						grp1[mm]==listgrp1[ii]
					) {
						ngrptot++;
						if(isfinite(data[mm])) tempdata[ntempdata++]= data[mm];
				}}
				if(ngrptot<=0) continue;
				/* get stats on the temp array */
				if(ntempdata>0) xf_stats2_d(tempdata,ntempdata,2,result_d);
				/* output the results */
				if(setgint==0) {
					printf("%g\t",listgrp1[ii]);
					printf("%ld\t%g\t%g\t%g\t%ld\n",ntempdata,result_d[0],result_d[2],result_d[3],ngrptot);
				}
				else {
					printf("%ld\t",(long)listgrp1[ii]);
					printf("%ld\t%g\t%g\t%g\t%ld\n",ntempdata,result_d[0],result_d[2],result_d[3],ngrptot);
				}
			}
		}
	}

	if(line!=NULL) free(line);
	if(iword!=NULL) free(iword);
	if(grp1!=NULL) free(grp1);
	if(listgrp1!=NULL) free(listgrp1);
	if(data!=NULL) free(data);
	if(tempdata!=NULL) free(tempdata);
	exit(0);
}
