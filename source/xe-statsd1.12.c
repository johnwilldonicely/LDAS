#define thisprog "xe-statsd1"
#define TITLE_STRING thisprog"v 12  14.September.2016 [JRH]"
#define MAXLINELEN 10000
#define MAXGROUP 8000

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


/*
<TAGS>math stats</TAGS>

v 12  28.September.2018 [JRH]
	- fix variable names and change "varcalc" to "setlarge" in function
	- for large datasets (setlarge2), the stats function also uses a mean based on the mean-noirmalizad data


v 12  14.September.2016 [JRH]
	- add support of long-lines input
	- add SUM to table-output

v 12  5.May.2013 [JRH]
	- include xf_compare_d to support revised version of xf_percentile1_d

v 11: 20.November.2012 [JRH]
	- update output format to %lg instead of %lf - better representation of very small numbers

v 10: 6.October.2012 [JRH]
	- updated call to xf_percentile1_d
		- "message" string is no longer used, as printing messages with the function name complicated determination of program dependencies

v 9: 14.August.2012 [JRH]
	- increase memory storage for each line
	- include warning about memory allocated per line

v 1.8: 9.July.2012 [JRH]
	- bugfix: previously, a "-" or "." could not be re-read as a string in order to be skipped - the following data-point was read instead, and hence omitted
		- this is a peculiarity of the fscanf function, which does not behave this way when reading other non-numbers
		- all data is now read into line which is then scanned, which prevents this behaviour

v 1.7: JRH, 16 May 2011
	- bugfix - program will now exclude NaN and Inf from calculations instead of allowing them to become zero

v.1.6 (24 Jan 2011)
	- bugfix - fixed problem with median calculation - new function [xf]_percentiled1 replaces old hux_percentiled1
	- eliminated SUM output in verbose mode - unnecessary given N and MEAN are reported

v.1.5 (8 Jan 2011)
	- bugfix - noow handles empty datasets

v.1.4
	- added SUM to table-output

*/


/* external functions start */
char *xf_lineread1(char *line, long *maxlinelen, FILE *fpin);
int xf_stats2_d(double *data, long n, int varcalc, double *result_d);
int xf_percentile1_d(double *data, long n, double *result);
int xf_compare1_d(const void *a, const void *b);
double xf_stats1_d(double *data1, long nn, int digits);
/* external functions end */

int main (int argc, char *argv[]) {
	/* general variables */
	char infile[256],outfile[256],*line=NULL,*pline,*pcol,*perror;
	long int i,j,k,n;
	int w,x,y,z,col;
	int sizeofint=sizeof(int),sizeoffloat=sizeof(float),sizeofdouble=sizeof(double);
 	long maxlinelen=1000;
 	float a,b,c;
	double aa,bb,cc,result_d[64];
	FILE *fpin,*fpout;
	/* program-specific variables */
	double *data=NULL,binsize=1.0,sumtot=0.0;
	double min,max,mean,sum,sumsquares,variance,stddev,sem,ci,skew;
	/* arguments */
	int groupcol=-1,varcalc=2,percalc=0,alphapercent=5,setformat=1;

	sprintf(outfile,"stdout");
	n=0;mean=0.0;sum=sumsquares=variance=sem=ci=-1.0;

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");

		fprintf(stderr,"Calculates summary statistics for an array of double-precision data \n");
		fprintf(stderr,"Non-numeric values will be ignored\n");
		fprintf(stderr,"INPUT: \n");
		fprintf(stderr,"	A single stream of data - rows or columns\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [filename] [options]\n",thisprog);
		fprintf(stderr,"		[filename]: file name or \"stdin\"\n");
		fprintf(stderr,"VALID OPTIONS ( defaults in [] ):\n");
		fprintf(stderr,"	-f format of output: verbose (1) or single-line (0) [%d]\n",setformat);
		fprintf(stderr,"	-out specify an output file or \"stdout\" (screen) [%s]\n",outfile);
		fprintf(stderr,"	-var variance & mean calculation for large datasets [%d]\n",varcalc);
		fprintf(stderr,"		1=computational, 2=per-sample with correction\n");
		fprintf(stderr,"	-per percentile calculation[%d]\n",percalc);
		fprintf(stderr,"		0=skip, 1=calculate mdeian and other percentiles\n");
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s data.txt -var 1\n",thisprog);
		fprintf(stderr,"	cat temp.txt | %s stdin \n",thisprog);
		fprintf(stderr,"OUTPUT: Statistics specified by keyword\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/**********************************************************************************/
	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	/**********************************************************************************/
	sprintf(infile,"%s",argv[1]);
	for(i=2;i<argc;i++) {
		if( *(argv[i]+0) == '-') {
			if((i+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[i]); exit(1);}
			else if(strcmp(argv[i],"-f")==0) 	{ setformat=atoi(argv[i+1]); i++;}
			else if(strcmp(argv[i],"-out")==0) 	{ sprintf(outfile,"%s",argv[i+1]); i++;}
			else if(strcmp(argv[i],"-per")==0) 	{ percalc=atoi(argv[i+1]); i++;}
			else if(strcmp(argv[i],"-var")==0) 	{ varcalc=atoi(argv[i+1]); i++;}
			else {fprintf(stderr,"\t\aError[%s]: invalid command line argument \"%s\"\n",thisprog,argv[i]); exit(1);}
	}}
	if(varcalc!=1 && varcalc!=2) { fprintf(stderr,"\n--- Error [%s]: invalid -var [%d] must be 1 or 2\n\n",thisprog,varcalc);exit(1);}


	if(strcmp(outfile,"stdout")==0) fpout=stdout;
 	else if((fpout=fopen(outfile,"w"))==0) {fprintf(stderr,"\t\aError[%s]: could not write to \"%s\"\n",thisprog,outfile);exit(1);}

	/**********************************************************************************/
	/* STORE DATA **/
	/**********************************************************************************/
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\t\aError[%s]: file \"%s\" not found\n",thisprog,infile);exit(1);}
	n=0;
 	while((line=xf_lineread1(line,&maxlinelen,fpin))!=NULL) {
 		if(maxlinelen==-1)  {fprintf(stderr,"\n--- Error[%s]: readline function encountered insufficient memory\n\n",thisprog);exit(1);}

		pline=line;
		for(col=1;(pcol=strtok(pline," ,\t\n\r"))!=NULL;col++) {
			if(pcol[0]=='#') break;
			pline=NULL;
			if(sscanf(pcol,"%lf  ",&aa)==1) {
				data=(double *)realloc(data,(n+1)*sizeofdouble);
				if(data==NULL) {fprintf(stderr,"\t\aError[%s]: insufficient memory\n",thisprog);exit(1);}
				if(isfinite(aa)) data[n++]=aa; // only add to memory if the number is normal (not imaginary, not NaN or Inf)
	}}}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);

	/**********************************************************************************/
	/* CALL BASIC STATS FUNCTION TO CALCULATE VALUES */
	/**********************************************************************************/
	xf_stats2_d(data,n,varcalc,result_d);int xf_percentile1_d(double *data, long n, double *result);
	mean=result_d[0];
	variance=result_d[1];
	stddev=result_d[2];
	sem=result_d[3];
	min=result_d[4];
	max=result_d[5];
	skew=result_d[6];
	sum=result_d[7];

	/* DEAL WITH EMPTY DATASET (N=0) */
	if(n<1) {
		if(setformat==0) {
			printf("0	-	-	-	-	-	-	-	-	-");
			if(percalc==1) for(i=0;i<11;i++) fprintf(fpout,"\t-");
			fprintf(fpout,"\n"); free(data); exit(0);
		}
		if(setformat==1) {
			fprintf(fpout,"\nN 0\nSUM -\nMEAN -\nMIN -\nMAX -\nRANGE -\nVARIANCE -\nSTDDEV -\nSEM -\nSKEW -\n");
			if(percalc==1) fprintf(fpout,"\nPERCENTILE_1 -\nPERCENTILE_2.5 -\nPERCENTILE_5 -\nPERCENTILE_10 -\nPERCENTILE_25 -\nPERCENTILE_50 -\nPERCENTILE_75 -\nPERCENTILE_90 -\nPERCENTILE_95 -\nPERCENTILE_97.5 -\nPERCENTILE_99 -\n");
			fprintf(fpout,"\n"); free(data); exit(0);
		}
	}

	/* PRINT OUTPUT IN SINGLE-LINE MODE */
	if(setformat==0) {
		fprintf(fpout,"%ld\t%lg\t%lg\t%lg\t%lg\t%lg\t%lg\t%lg\t%lg\t%lg",
			n,sum,mean,min,max,(max-min),variance,stddev,sem,skew);
		if(percalc==1) { // IF PERCENTILES ARE REQUIRED

			z=xf_percentile1_d(data,n,result_d);
			if(z!=0) {fprintf(stderr,"\t\aError[%s]: insufficient memory for calculation of percentiles\n",thisprog);exit(1);}
			else for(i=0;i<11;i++) fprintf(fpout,"\t%lg",result_d[i]);
		}
		fprintf(fpout,"\n");
	}
	/* PRINT OUTPUT IN VERBOSE MODE */
	else if(setformat==1) {
		fprintf(fpout,"\n");
		fprintf(fpout,"N %ld\n",n);
		fprintf(fpout,"SUM %lf\n",sum);
		fprintf(fpout,"MEAN %lf\n",mean);
		fprintf(fpout,"MIN %lf\n",min);
		fprintf(fpout,"MAX %lf\n",max);
		fprintf(fpout,"RANGE %lf\n",(max-min));
		fprintf(fpout,"VARIANCE %lf\n",variance);
		fprintf(fpout,"STDDEV %lf\n",stddev);
		fprintf(fpout,"SEM %lf\n",sem);
		fprintf(fpout,"SKEW %lf\n",skew);
		if(percalc==1) { // IF PERCENTILES ARE REQUIRED

			z=xf_percentile1_d(data,n,result_d);
			if(z!=0) {fprintf(stderr,"\t\aError[%s]: insufficient memory for calculation of percentiles\n",thisprog);exit(1);}

			fprintf(fpout,"\n");
			fprintf(fpout,"PERCENTILE_1	%lg\n",result_d[0]);
			fprintf(fpout,"PERCENTILE_2.5	%lg\n",result_d[1]);
			fprintf(fpout,"PERCENTILE_5	%lg\n",result_d[2]);
			fprintf(fpout,"PERCENTILE_10	%lg\n",result_d[3]);
			fprintf(fpout,"PERCENTILE_25	%lg\n",result_d[4]);
			fprintf(fpout,"PERCENTILE_50	%lg\n",result_d[5]);
			fprintf(fpout,"PERCENTILE_75	%lg\n",result_d[6]);
			fprintf(fpout,"PERCENTILE_90	%lg\n",result_d[7]);
			fprintf(fpout,"PERCENTILE_95	%lg\n",result_d[8]);
			fprintf(fpout,"PERCENTILE_97.5	%lg\n",result_d[9]);
			fprintf(fpout,"PERCENTILE_99	%lg\n",result_d[10]);
		}
		fprintf(fpout,"\n");
	}


	if(strcmp(outfile,"stdout")!=0) fclose(fpout);
	free(line);
	free(data);
	exit(0);
}
