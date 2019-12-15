#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define thisprog "xe-transpose4"
#define TITLE_STRING thisprog" v 8: 4.April.2018 [JRH]"
#define MAXLINELEN 1000
#define MAXLABELS 1000

/*
<TAGS>math stats</TAGS>
v 7: 4.April.2018 [JRH]
	- use newer variable naming conventions
	- retire unused word, bin setlow, sethigh, grp variables
	- upgrade group-variables to double
	- fix instructions


v 7: 5.May.2013 [JRH]
	- update usage of qsort to call external compare function xf_compare1_d

v 7: 10.February.2013 [JRH]
	- bugfix - previous versions omitted header for grp3
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
void xf_stats2_d(double *data, long n, int varcalc, double *result_d);
int xf_compare1_d(const void *a, const void *b);
/* external functions end */

int main (int argc, char *argv[]) {
	/* general variables */
	char line[MAXLINELEN],templine[MAXLINELEN],*pline,*pcol;
	long int ii,jj,kk,ll,mm,nn,col,colmatch;
	double aa,bb,cc,dd,result_d[64];
	FILE *fpin,*fpout;
	/* program-specific variables */
	int sizeofgrp1,sizeofgrp2,sizeofgrp3,sizeofdata;
	long nlistgrp1=0,nlistgrp2=0,nlistgrp3=0,ntempdata=0,count;
	double *grp1=NULL,*grp2=NULL,*grp3=NULL,*listgrp1=NULL,*listgrp2=NULL,*listgrp3=NULL;
	double *data=NULL,*tempdata=NULL;
	/* arguments */
	char *infile=NULL;
	int setgint=0,setcolg1=1,setcolg2=2,setcolg3=3,setcoldata=4;

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"- Transpose data for mixed-design repeated measures analysis\n");
		fprintf(stderr,"- Data can be NAN, but grouping variables must be finite numbers\n");
		fprintf(stderr,"- Max input line length = %d\n",MAXLINELEN);
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [input] [g1] [g2] [g3] [data]\n",thisprog);
		fprintf(stderr,"	[input]: file name or \"stdin\"\n");
		fprintf(stderr,"VALID OPTIONS:\n");
		fprintf(stderr,"	-cg1 column defining subject [%d]\n",setcolg1);
		fprintf(stderr,"	-cg2 column defining between-subjects groups  [%d]\n",setcolg2);
		fprintf(stderr,"	-cg3 column defining repeated measure levels  [%d]\n",setcolg3);
		fprintf(stderr,"	-cy column containing dependent variable [%d]\n",setcoldata);
		fprintf(stderr,"	-gint: output groups as integers? (0=NO 1=YES) [%d]\n",setgint);
		fprintf(stderr,"NOTE: \n");
		fprintf(stderr,"	- if there is no between-subjects grouping column, set -cg2 to 0\n");
		fprintf(stderr,"	- in this case, all subjects are placed in group \"1\"\n");
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s data.txt -cg1 1 -cg2 2 -cg3 3 -cy 7\n",thisprog);
		fprintf(stderr,"	cat temp.txt | %s stdin \n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	subj    grp    r_1    r_2    r_3    r_4    etc...\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}


	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	infile= argv[1];
	for(ii=2;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-cg1")==0) 	{ setcolg1=atoi(argv[ii+1]); ii++;}
			else if(strcmp(argv[ii],"-cg2")==0) 	{ setcolg2=atoi(argv[ii+1]); ii++;}
			else if(strcmp(argv[ii],"-cg3")==0) 	{ setcolg3=atoi(argv[ii+1]); ii++;}
			else if(strcmp(argv[ii],"-cy")==0) 	{ setcoldata=atoi(argv[ii+1]); ii++;}
			else if(strcmp(argv[ii],"-gint")==0) 	{ setgint=atoi(argv[ii+1]); ii++;}
			else {fprintf(stderr,"\t\a--- Error[%s]: invalid command line argument \"%s\"\n",thisprog,argv[ii]); exit(1);}
	}}


	sizeofdata= sizeof(*data);
	sizeofgrp1= sizeof(*grp1);
	sizeofgrp2= sizeof(*grp2);
	sizeofgrp3= sizeof(*grp3);

	/* STORE DATA METHOD 3 - newline delimited input  from user-defined columns */
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
	nn=0;
	while(fgets(line,MAXLINELEN,fpin)!=NULL) {
		if(line[0]=='#') continue;
		pline=line; colmatch=4; // number of columns to match
		for(col=1;(pcol=strtok(pline," ,\t\n\r"))!=NULL;col++) {
			pline=NULL;
			if(col==setcolg1 && sscanf(pcol,"%lf",&aa)==1) colmatch--;
			if(col==setcolg2 && sscanf(pcol,"%lf",&bb)==1) colmatch--;
			if(col==setcolg3 && sscanf(pcol,"%lf",&cc)==1) colmatch--;
			if(col==setcoldata && sscanf(pcol,"%lf",&dd)==1) colmatch--;
		}
		if(colmatch!=0 || !isfinite(aa) || !isfinite(bb) || !isfinite(cc) || !isfinite(dd)) continue;
		grp1= realloc(grp1,(nn+1)*sizeofgrp1);
		grp2= realloc(grp2,(nn+1)*sizeofgrp2);
		grp3= realloc(grp3,(nn+1)*sizeofgrp3);
		data= realloc(data,(nn+1)*sizeofdata);
		if(grp1==NULL||grp2==NULL||grp3==NULL||data==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		grp1[nn]= aa;
		grp2[nn]= bb;
		grp3[nn]= cc;
		data[nn]= dd;
		nn++;
	}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);
	//TEST:	for(ii=0;ii<nn;ii++) printf("%g\t%g\t%g\n",grp1[ii],grp2[ii],grp3[ii],data[ii]); exit(0);


	/* ALLOCATE MEMORY FOR LISTS AND TEMPDATA */
	listgrp1= realloc(listgrp1,(nn+1)*sizeofgrp1);
	listgrp2= realloc(listgrp2,(nn+1)*sizeofgrp2);
	listgrp3= realloc(listgrp3,(nn+1)*sizeofgrp3);
	tempdata= realloc(tempdata,(nn+1)*sizeofdata);
	if(listgrp1==NULL||listgrp2==NULL||listgrp3==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};

	/* CREATE A SORTED LIST OF THE ELEMENTS IN GRP1 */
	for(ii=0;ii<nn;ii++) listgrp1[ii]= grp1[ii];
	qsort(listgrp1,nn,sizeof(double),xf_compare1_d);
	/* copy only unique items to new version of listgrp1 */
	aa=listgrp1[0]; for(ii=nlistgrp1=1;ii<nn;ii++) {if(listgrp1[ii]!=aa) listgrp1[nlistgrp1++]=listgrp1[ii];aa=listgrp1[ii]; }
	/* CREATE A SORTED LIST OF THE ELEMENTS IN GRP2 */
	for(ii=0;ii<nn;ii++) listgrp2[ii]=grp2[ii];
	qsort(listgrp2,nn,sizeof(double),xf_compare1_d);
	/* copy only unique items to new version of listgrp2 */
	aa=listgrp2[0]; for(ii=nlistgrp2=1;ii<nn;ii++) {if(listgrp2[ii]!=aa) listgrp2[nlistgrp2++]=listgrp2[ii];aa=listgrp2[ii]; }
	/* CREATE A SORTED LIST OF THE ELEMENTS IN GRP3 */
	for(ii=0;ii<nn;ii++) listgrp3[ii]=grp3[ii];
	qsort(listgrp3,nn,sizeof(double),xf_compare1_d);
	/* copy only unique items to new version of listgrp3 */
	aa=listgrp3[0]; for(ii=nlistgrp3=1;ii<nn;ii++) {if(listgrp3[ii]!=aa) listgrp3[nlistgrp3++]=listgrp3[ii];aa=listgrp3[ii]; }
	//TEST:	for(ii=0;ii<nn;ii++) printf("%g\t%g\t%g\n",listgrp1[ii],listgrp2[ii],listgrp3[ii],data[ii]); exit(0);
	//TEST: printf("1:%ld\n2:%ld\n3:%ld\n",nlistgrp1,nlistgrp2,nlistgrp3);exit(0);


	/* TRANSPOSE THE DATA */
	printf("subj\tgrp");
	if(setgint==0) { for(kk=0;kk<nlistgrp3;kk++) printf("\tr_%ld",(long)listgrp3[kk]); printf("\n"); }
	else           { for(kk=0;kk<nlistgrp3;kk++) printf("\tr_%ld",(long)listgrp3[kk]); printf("\n"); }
	for(ii=0;ii<nlistgrp1;ii++) {
		for(jj=0;jj<nlistgrp2;jj++) {
			count=0;
			for(mm=0;mm<nn;mm++) { if(grp1[mm]==listgrp1[ii] && grp2[mm]==listgrp2[jj]) { count=1; break; }	}
			if(count==0) continue;
			if(setgint==0) printf("%f\t%f",listgrp1[ii],listgrp2[jj]);
			else           printf("%ld\t%ld",(long)listgrp1[ii],(long)listgrp2[jj]);
			for(kk=0;kk<nlistgrp3;kk++) {
				count=0;
				for(mm=0;mm<nn;mm++) {
					if(grp1[mm]==listgrp1[ii] && grp2[mm]==listgrp2[jj] && grp3[mm]==listgrp3[kk]) {
						printf("\t%f",data[mm]);
						count++;
					}
				}
				if(count==0) printf("\t-");
				//TEST:	fprintf(stderr,"%g	%g	%g	%ld\n",listgrp1[i],listgrp2[j],listgrp3[k],count);
			}
			printf("\n");
		}
	}



	free(grp1);
	free(grp2);
	free(grp3);
	free(listgrp1);
	free(listgrp2);
	free(listgrp3);
	free(data);
	free(tempdata);
	exit(0);
}
