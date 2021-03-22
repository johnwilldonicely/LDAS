#define thisprog "xe-checkreplicate1"
#define TITLE_STRING thisprog" 22.March.2021 [JRH]"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
/*
<TAGS> LDAS </TAGS>

 22.March.2021 [JRH]
 	- add checks for missing columns 
v 1: DAY.MONTH.YEAR [JRH]
	- bugfix - changed use of fscanf to read data with fgets/sscanf, to avoid problems related to "-" and "."
*/



/* external functions start */
char *xf_lineread1(char *line, long *maxlinelen, FILE *fpin);
long *xf_lineparse2(char *line,char *delimiters, long *nwords);
int xf_compare1_d(const void *a, const void *b);
/* external functions end */


int main (int argc, char *argv[]) {

	/* general variables */
	char *line=NULL;
	long ii,jj,kk,nn,mm,maxlinelen=0;
	double aa,bb,cc,dd;
	FILE *fpin;
	/* program-specific variables */
	int sizeofgrp1,sizeofgrp2,sizeofgrp3,sizeofdata;
	long *iword=NULL,nwords,colmatch;
	long nlistgrp1=0,nlistgrp2=0,nlistgrp3=0,count=0;
	double *grp1=NULL,*grp2=NULL,*grp3=NULL,*listgrp1=NULL,*listgrp2=NULL,*listgrp3=NULL;
	double *data=NULL;

	/* arguments */
	char *infile=NULL;
	int setverb=0,setval=0;
	long setcolg1=1,setcolg2=2,setcolg3=3,setcoldata=4,sethead=0;

	/********************************************************************************
	PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED
	********************************************************************************/
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Test for data-replicates for all combinations of groups in a table\n");
		fprintf(stderr,"- requires 3 columns defining groupings, and a fourth data-column\n");
		fprintf(stderr,"    - example: repeated-measures study: subject,treatment,time,data\n");
		fprintf(stderr,"    - column-numbering starts at \"1\"\n");
		fprintf(stderr,"    - there must be 1 datum for every combination of groups\n");
		fprintf(stderr,"    - grouping columns must only contain finite numbers\n");
		fprintf(stderr,"    - header-lines must be excluded (see -head option below)\n");
		fprintf(stderr,"USAGE: %s [in] [options]\n",thisprog);
		fprintf(stderr,"    [in]: file name or \"stdin\"\n");
		fprintf(stderr,"VALID OPTIONS: defaults in []\n");
		fprintf(stderr,"    -cg1: grouping column 1 [%ld]\n",setcolg1);
		fprintf(stderr,"    -cg2: grouping column 2 [%ld]\n",setcolg2);
		fprintf(stderr,"    -cg3: grouping column 3 [%ld]\n",setcolg3);
		fprintf(stderr,"    -cy : data column containing results for group-combination [%ld]\n",setcoldata);
		fprintf(stderr,"    -head: number of lines at top of file to skip [%ld]\n",sethead);
		fprintf(stderr,"    -val: also require valid data in column \"-cy\" (0=NO 1=YES) [%d]\n",setval);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"    --- Error messages (to stderr) for missing data\n");
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"    %s data.txt -cy 20  2> temp.err\n",thisprog);
		fprintf(stderr,"    [[ -s temp.err ]] && cat temp.err\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/********************************************************************************
	READ THE FILENAME AND OPTIONAL ARGUMENTS
	********************************************************************************/
	infile= argv[1];
	for(ii=2;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-verb")==0) setverb= atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-cg1")==0)  setcolg1= atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-cg2")==0)  setcolg2= atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-cg3")==0)  setcolg3= atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-cy")==0)   setcoldata= atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-head")==0) sethead= atol(argv[++ii]);
			else if(strcmp(argv[ii],"-val")==0) setval= atoi(argv[++ii]);
			else {fprintf(stderr,"\n--- Error [%s]: invalid command line argument [%s]\n\n",thisprog,argv[ii]); exit(1);}
	}}
	if(setverb!=0 && setverb!=1) { fprintf(stderr,"\n--- Error [%s]: invalid -verb [%d] must be 0 or 1\n\n",thisprog,setverb);exit(1);}
	if(setcolg1<1) {fprintf(stderr,"\n--- Error[%s]: invalid -cg1 (%ld) - must be >0\n\n",thisprog,setcolg1); exit(1);}
	if(setcolg2<1) {fprintf(stderr,"\n--- Error[%s]: invalid -cg2 (%ld) - must be >0\n\n",thisprog,setcolg2); exit(1);}
	if(setcolg3<1) {fprintf(stderr,"\n--- Error[%s]: invalid -cg3 (%ld) - must be >0\n\n",thisprog,setcolg3); exit(1);}
	if(setcoldata<1) {fprintf(stderr,"\n--- Error[%s]: invalid -cy (%ld) - must be >0\n\n",thisprog,setcoldata); exit(1);}
	if(sethead<0) { fprintf(stderr,"\n--- Error [%s]: invalid -head [%ld] must be >=0\n\n",thisprog,sethead);exit(1);}
	if(setval!=0 && setval!=1) { fprintf(stderr,"\n--- Error [%s]: invalid -val [%d] must be 0 or 1\n\n",thisprog,setval);exit(1);}

	/********************************************************************************
	CREATE VARIABLES FOR DATA-SIZES TO SPEED UP REALLOC() CALLS DURING DATA STORAGE
	********************************************************************************/
	sizeofgrp1= sizeof(*grp1);
	sizeofgrp2= sizeof(*grp2);
	sizeofgrp3= sizeof(*grp3);
	sizeofdata= sizeof(*data);

	/********************************************************************************
	DECREMENT COLUMN-NUMBERS FOR ZERO-OFFSET NUMBERING FROM THE LINEPARSE FUNCTION
	********************************************************************************/
	setcolg1--;
	setcolg2--;
	setcolg3--;
	setcoldata--;

	/********************************************************************************
	STORE DATA - ASSUME WE DON'T KNOW THE LENGTH OF EACH INPUT LINE
	********************************************************************************/
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
	nn=mm=0;
	while((line=xf_lineread1(line,&maxlinelen,fpin))!=NULL) {
		if(maxlinelen==-1)  {fprintf(stderr,"\n--- Error[%s]: readline function encountered insufficient memory\n\n",thisprog);exit(1);}
		mm++;
		iword= xf_lineparse2(line,"\t",&nwords);
		if(nwords<0) {fprintf(stderr,"\n--- Error[%s]: lineparse function encountered insufficient memory\n\n",thisprog);exit(1);};
		if(nwords<4) {fprintf(stderr,"\n--- Error[%s]: input contains fewer than 4 columns on line %ld\n\n",thisprog,mm);exit(1);};
		if(mm<=sethead) continue;
		colmatch=4;
		for(ii=0;ii<nwords;ii++) {
			if(ii==setcolg1) {
				colmatch--;
				if(sscanf(line+iword[ii],"%lf",&aa)!=1 || !isfinite(aa)) { fprintf(stderr,"\n--- Error[%s]: non-numeric value on line %ld column %ld (%s)\n\n",thisprog,(nn+1),(ii+1),(line+iword[ii])); exit(1);} }
			else if(ii==setcolg2) {
				colmatch--;
				if(sscanf(line+iword[ii],"%lf",&bb)!=1 || !isfinite(bb)) { fprintf(stderr,"\n--- Error[%s]: non-numeric value on line %ld column %ld (%s)\n\n",thisprog,(nn+1),(ii+1),(line+iword[ii])); exit(1);} }
			else if(ii==setcolg3){
				colmatch--;
				if(sscanf(line+iword[ii],"%lf",&cc)!=1 || !isfinite(cc)) { fprintf(stderr,"\n--- Error[%s]: non-numeric value on line %ld column %ld (%s)\n\n",thisprog,(nn+1),(ii+1),(line+iword[ii])); exit(1);} }
			else if(ii==setcoldata) {
				colmatch--;
				if(sscanf(line+iword[ii],"%lf",&dd)!=1 || !isfinite(dd)) dd= NAN;
			}
		}
		if(colmatch!=0 || !isfinite(aa) || !isfinite(bb) || !isfinite(cc)) continue;
		grp1= realloc(grp1,(nn+1)*sizeofgrp1);
		grp2= realloc(grp2,(nn+1)*sizeofgrp2);
		grp3= realloc(grp3,(nn+1)*sizeofgrp3);
		data= realloc(data,(nn+1)*sizeofdata);
		if(grp1==NULL||grp2==NULL||grp3==NULL||data==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		grp1[nn]= aa;
		grp2[nn]= bb;
		grp3[nn]= cc;
		data[nn]= dd;
		/* report if invalid data was found (if required by -val setting) */
		if(setval==1 && !isfinite(dd)) fprintf(stderr,"--- Error[%s]: non-numeric data value on line %ld [%s]\n",thisprog,mm,(line+iword[setcoldata]));
		nn++;
	}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);
	if(nn==0) {fprintf(stderr,"\n--- Error[%s]: input contains no valid lines of data\n\n",thisprog);exit(1);};

	//TEST for(ii=0;ii<nn;ii++) printf("data[%ld]= %g\n",ii,data[ii]);exit(0);


	/********************************************************************************
	ALLOCATE MEMORY FOR THE LISTS OF GROUPING-VARIABLES
	********************************************************************************/
	listgrp1= realloc(listgrp1,nn*sizeofgrp1);
	listgrp2= realloc(listgrp2,nn*sizeofgrp2);
	listgrp3= realloc(listgrp3,nn*sizeofgrp3);
	if(listgrp1==NULL||listgrp2==NULL||listgrp3==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};


	/********************************************************************************
	CREATE A SORTED LIST OF THE UNIQUE ELEMENTS IN EACH GROUPING VARIABLE
	********************************************************************************/
	for(ii=0;ii<nn;ii++) listgrp1[ii]= grp1[ii];
	qsort(listgrp1,nn,sizeof(double),xf_compare1_d);
	aa=listgrp1[0]; for(ii=nlistgrp1=1;ii<nn;ii++) {if(listgrp1[ii]!=aa) listgrp1[nlistgrp1++]= listgrp1[ii]; aa= listgrp1[ii]; }
	for(ii=0;ii<nn;ii++) listgrp2[ii]=grp2[ii];
	qsort(listgrp2,nn,sizeof(double),xf_compare1_d);
	aa=listgrp2[0]; for(ii=nlistgrp2=1;ii<nn;ii++) {if(listgrp2[ii]!=aa) listgrp2[nlistgrp2++]= listgrp2[ii]; aa= listgrp2[ii]; }
	for(ii=0;ii<nn;ii++) listgrp3[ii]=grp3[ii];
	qsort(listgrp3,nn,sizeof(double),xf_compare1_d);
	aa=listgrp3[0]; for(ii=nlistgrp3=1;ii<nn;ii++) {if(listgrp3[ii]!=aa) listgrp3[nlistgrp3++]= listgrp3[ii]; aa= listgrp3[ii]; }
	//TEST:	for(ii=0;ii<nn;ii++) printf("%g\t%g\t%g\t\t%g\n",listgrp1[ii],listgrp2[ii],listgrp3[ii],data[ii]); exit(0);
	//TEST: printf("1:%ld\n2:%ld\n3:%ld\n",nlistgrp1,nlistgrp2,nlistgrp3);exit(0);


	/********************************************************************************
	NOW SCAN THE DATA AND CHECK EVERY GROUPING COMBINATION IS FOUND
	********************************************************************************/
	for(ii=0;ii<nlistgrp1;ii++) {
		aa= listgrp1[ii];
		for(jj=0;jj<nlistgrp2;jj++) {
			bb= listgrp2[jj];
			for(kk=0;kk<nlistgrp3;kk++) {
				cc= listgrp3[kk];
				count=0;
				for(mm=0;mm<=nn;mm++) {
					if(grp1[mm]==aa && grp2[mm]==bb && grp3[mm]==cc) count++;
				}
				if(count==0) fprintf(stderr,"--- Error[%s]: missing data for combination:  [%g] [%g] [%g]\n",thisprog,aa,bb,cc);
				if(count>1) fprintf(stderr,"--- Error[%s]: %ld duplicates of combination:  [%g] [%g] [%g]\n",thisprog,count,aa,bb,cc);
	}}}

	goto END;


	/********************************************************************************/
	/* CLEANUP AND EXIT */
	/********************************************************************************/
END:
	if(line!=NULL) free(line);
	if(iword!=NULL) free(iword);
	if(grp1!=NULL) free(grp1);
	if(grp2!=NULL) free(grp2);
	if(grp3!=NULL) free(grp3);
	if(data!=NULL) free(data);
	if(listgrp1!=NULL) free(listgrp1);
	if(listgrp2!=NULL) free(listgrp2);
	if(listgrp3!=NULL) free(listgrp3);
	exit(0);
}
