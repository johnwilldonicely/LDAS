#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define thisprog "xe-statsgrp1"
#define thisprog "xe-statsgrp2"
#define thisprog "xe-statsgrp3"
#define TITLE_STRING thisprog" v 9: 29.April.2019 [JRH]"

/*
<TAGS>math stats</TAGS>

v 9: 29.April.2019 [JRH]
	- rework so "grep -vE" can be used to generate code for xe-statsgroup2 and xe-statsgroup1 from xe-statsgroup3
		grep -vE 'grp3|cg3' xe-statsgrp3.c > xe-statsgrp2.c
		grep -vE 'grp2|cg2' xe-statsgrp2.c > xe-statsgrp1.c

	VALIDATION:
	in1=/opt/LDAS/docs/sample_data/sample_group_x_time.txt
	list1=$(xe-cut1 $in1 subject -o 1 | sort -nu)
	list2=$(xe-cut1 $in1 group -o 1 | sort -nu)
	list3=$(xe-cut1 $in1 dname -o 1 | sort -nu)
	for i in $list1 ; do for j in $list2 ; do for k in $list3 ; do xe-dbmatch1 $in1 subject $i | xe-dbmatch1 stdin group $j | xe-dbmatch1 stdin dname $k | xe-cut1 stdin gamma -o 1 | xe-statsd1 stdin | xe-getkey stdin MEAN | awk '{print "'$i'\t'$j'\t'$k'\t",$1}' ; done ; done ; done
	...compare with...
	xe-statsgrp3 $in1 -cg1 1 -cg2 4 -cg3 3 -cy 9

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
	- change to having default grouping & data columns, and added -cg1 -cg2 -cg3 and -cy options to override defaults

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
	double *grp2=NULL,*listgrp2=NULL; long nlistgrp2=0; int sizeofgrp2=sizeof(*grp2); double tempgrp2;
	double *grp3=NULL,*listgrp3=NULL; long nlistgrp3=0; int sizeofgrp3=sizeof(*grp3); double tempgrp3;
	long nwords=0,*iword=NULL,colmatch;
	long ntempdata=0,ngrptot=0;
	double *data=NULL,*tempdata=NULL;
	int sizeofdata=sizeof(*data);
	/* arguments */
	char *infile=NULL;
	int setgint=0;
	long setcolgrp1=1;
	long setcolgrp2=2;
	long setcolgrp3=3;
	long setcoldata;

	setcoldata= setcolgrp1+1;
	setcoldata= setcolgrp2+1;
	setcoldata= setcolgrp3+1;

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
		fprintf(stderr,"	-cg2:  column defining grouping-variable 2  [%ld]\n",setcolgrp2);
		fprintf(stderr,"	-cg3:  column defining grouping-variable 3  [%ld]\n",setcolgrp3);
		fprintf(stderr,"	-cy:   column containing dependent variable [%ld]\n",setcoldata);
		fprintf(stderr,"	-gint: output groups as integers? (0=NO 1=YES) [%d]\n",setgint);
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s data.txt",thisprog);
		fprintf(stderr," -cg1 5");
		fprintf(stderr," -cg2 7");
		fprintf(stderr," -cg3 9");
		fprintf(stderr,"\n",thisprog);
		fprintf(stderr,"	cat temp.txt | %s stdin -gint 1\n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"\tgrp1");
		fprintf(stderr,"\tgrp2");
		fprintf(stderr,"\tgrp3");
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
			else if(strcmp(argv[ii],"-cg2")==0)  setcolgrp2= atol(argv[++ii]);
			else if(strcmp(argv[ii],"-cg3")==0)  setcolgrp3= atol(argv[++ii]);
			else if(strcmp(argv[ii],"-cy")==0)   setcoldata= atol(argv[++ii]);
			else if(strcmp(argv[ii],"-gint")==0) setgint= atoi(argv[++ii]);
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n",thisprog,argv[ii]); exit(1);}
	}}
	if(setcolgrp1<1) {fprintf(stderr,"\n--- Error[%s]: invalid group column (-cg1 %ld) - must be >0\n",thisprog,setcolgrp1); exit(1);}
	if(setcolgrp2<1) {fprintf(stderr,"\n--- Error[%s]: invalid group column (-cg2 %ld) - must be >0\n",thisprog,setcolgrp2); exit(1);}
	if(setcolgrp3<1) {fprintf(stderr,"\n--- Error[%s]: invalid group column (-cg3 %ld) - must be >0\n",thisprog,setcolgrp3); exit(1);}
	if(setcoldata<1) {fprintf(stderr,"\n--- Error[%s]: invalid data column (-cy %ld) - must be >0\n",thisprog,setcoldata); exit(1);}

	/* DECREMENT COLUMN-NUMBERS SO THEY'RE ZERO-OFFSET */
	setcolgrp1--;
	setcolgrp2--;
	setcolgrp3--;
	setcoldata--;

	/* STORE DATA */
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
	nn=0;
	dd=NAN;
	tempgrp1=NAN;
	tempgrp2=NAN;
	tempgrp3=NAN;
	while((line=xf_lineread1(line,&maxlinelen,fpin))!=NULL) {
		if(maxlinelen==-1)  {fprintf(stderr,"\n--- Error[%s]: readline function encountered insufficient memory\n\n",thisprog);exit(1);}
		if(line[0]=='#') continue;
		/* parse the line & make sure all required columns are present */
		iword= xf_lineparse2(line,"\t",&nwords);
		if(nwords<0) {fprintf(stderr,"\n--- Error[%s]: lineparse function encountered insufficient memory\n\n",thisprog);exit(1);};
		if(
			nwords<setcoldata
			|| nwords<setcolgrp1
			|| nwords<setcolgrp2
			|| nwords<setcolgrp3
			) continue;
		/* make sure each group-columns are numeric & finite, and convert non-numeric data to NAN */
		tempgrp1=tempgrp2=tempgrp3=dd= NAN;
		sscanf(line+iword[setcolgrp1],"%lf",&tempgrp1);
		sscanf(line+iword[setcolgrp2],"%lf",&tempgrp2);
		sscanf(line+iword[setcolgrp3],"%lf",&tempgrp3);
		sscanf(line+iword[setcoldata],"%lf",&dd);
		if(
			!isfinite(tempgrp1)
			|| !isfinite(tempgrp2)
			|| !isfinite(tempgrp3)
		) continue;
		/* reallocate memory */
		data= realloc(data,(nn+1)*sizeofdata);
		grp1= realloc(grp1,(nn+1)*sizeofgrp1);
		grp2= realloc(grp2,(nn+1)*sizeofgrp2);
		grp3= realloc(grp3,(nn+1)*sizeofgrp3);
		if(
			data==NULL
			|| grp1==NULL
			|| grp2==NULL
			|| grp3==NULL
			) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		/* assign values */
		data[nn]= dd;
		grp1[nn]= tempgrp1;
		grp2[nn]= tempgrp2;
		grp3[nn]= tempgrp3;
		nn++;
	}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);
	//TEST:	for(ii=0;ii<nn;ii++) printf("%g\t%g\t%g\t%g\n",grp1[ii],grp2[ii],grp3[ii],data[ii]); exit(0);


	/* ALLOCATE MEMORY FOR LISTS AND TEMPDATA */
	listgrp1= realloc(listgrp1,(nn+1)*sizeofgrp1);
	listgrp2= realloc(listgrp2,(nn+1)*sizeofgrp2);
	listgrp3= realloc(listgrp3,(nn+1)*sizeofgrp3);
	tempdata= realloc(tempdata,(nn+1)*sizeofdata);
	if(
		listgrp1==NULL
		|| listgrp2==NULL
		|| listgrp3==NULL
	) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};


	/* CREATE A SORTED LIST OF THE ELEMENTS IN grp1 */
	for(ii=0;ii<nn;ii++) listgrp1[ii]= grp1[ii];
	qsort(listgrp1,nn,sizeof(double),xf_compare1_d);
	/* copy only unique items to new version of listgrp1 */
	aa=listgrp1[0]; for(ii=nlistgrp1=1;ii<nn;ii++) {if(listgrp1[ii]!=aa) listgrp1[nlistgrp1++]=listgrp1[ii];aa=listgrp1[ii]; }

	/* CREATE A SORTED LIST OF THE ELEMENTS IN grp2 */
	for(ii=0;ii<nn;ii++) listgrp2[ii]=grp2[ii];
	qsort(listgrp2,nn,sizeof(double),xf_compare1_d);
	/* copy only unique items to new version of listgrp2 */
	aa=listgrp2[0]; for(ii=nlistgrp2=1;ii<nn;ii++) {if(listgrp2[ii]!=aa) listgrp2[nlistgrp2++]=listgrp2[ii];aa=listgrp2[ii]; }

	/* CREATE A SORTED LIST OF THE ELEMENTS IN grp3 */
	for(ii=0;ii<nn;ii++) listgrp3[ii]=grp3[ii];
	qsort(listgrp3,nn,sizeof(double),xf_compare1_d);
	/* copy only unique items to new version of listgrp3 */
	aa=listgrp3[0]; for(ii=nlistgrp3=1;ii<nn;ii++) {if(listgrp3[ii]!=aa) listgrp3[nlistgrp3++]=listgrp3[ii];aa=listgrp3[ii]; }

	//TEST:	for(ii=0;ii<nn;ii++) printf("%g\t%g\t%g\t%g\n",listgrp1[ii],listgrp2[ii],listgrp3[ii],data[ii]); exit(0);
	//TEST:	printf("1:%ld\n2:%ld\n3:%ld\n",nlistgrp1,nlistgrp2,nlistgrp3);exit(0);


	/* CALCULATE STATS ON DATA IN EACH COMBINATION OF GROUP-CATEGORIES */
	printf("grp1\t");
	printf("grp2\t");
	printf("grp3\t");
	printf("n\tmean\tsd\tsem\tntot\n");
	for(ii=0;ii<nlistgrp1;ii++)
	{
		//printf("%g\n",listgrp1[ii]);
		for(jj=0;jj<nlistgrp2;jj++)
		{
			//printf("\t%g\n",listgrp2[jj]);
			for(kk=0;kk<nlistgrp3;kk++)
			{
				//printf("\t\t%g\n",listgrp3[kk]); continue;
				ngrptot= 0;
				result_d[0]=result_d[2]=result_d[3]= NAN;
				/* copy the good data for this category to a temp array */
				ntempdata=0;
				for(mm=0;mm<nn;mm++) {
					if(
						grp1[mm]==listgrp1[ii]
						&& grp2[mm]==listgrp2[jj]
						&& grp3[mm]==listgrp3[kk]
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
					printf("%g\t",listgrp2[jj]);
					printf("%g\t",listgrp3[kk]);
					printf("%ld\t%g\t%g\t%g\t%ld\n",ntempdata,result_d[0],result_d[2],result_d[3],ngrptot);
				}
				else {
					printf("%ld\t",(long)listgrp1[ii]);
					printf("%ld\t",(long)listgrp2[jj]);
					printf("%ld\t",(long)listgrp3[kk]);
					printf("%ld\t%g\t%g\t%g\t%ld\n",ntempdata,result_d[0],result_d[2],result_d[3],ngrptot);
				}
			}
		}
	}

	if(line!=NULL) free(line);
	if(iword!=NULL) free(iword);
	if(grp1!=NULL) free(grp1);
	if(grp2!=NULL) free(grp2);
	if(grp3!=NULL) free(grp3);
	if(listgrp1!=NULL) free(listgrp1);
	if(listgrp2!=NULL) free(listgrp2);
	if(listgrp3!=NULL) free(listgrp3);
	if(data!=NULL) free(data);
	if(tempdata!=NULL) free(tempdata);
	exit(0);
}
