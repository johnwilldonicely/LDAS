#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define thisprog "xe-matrixdiff1"
#define TITLE_STRING thisprog" v 2: 13.April.2013 [JRH]"
#define MAXLINELEN 10000
#define MAXLABELS 1000

/*
<TAGS>math matrix</TAGS>

v 2: 13.April.2013 [JRH]
	- add t-test as a way of determining sifgnificant differences - independent or paired

*/


/* external functions start */
double *xf_matrixread1_d(long *nmatrices, long *ncols, long *nrows, char *message, FILE *fpin);
long xf_stats3_d(double *data, long n, int varcalc, double *result_d);
int xf_ttest2_d(double *data1, double *data2, long n1, long n2, int varcalc, double *result_d);
int xf_ttest3_d(double *data1, double *data2, long n, int varcalc, double *result_d);
float xf_crit_T1(long df, float setalpha, int tails);
/* external functions end */

int main (int argc, char *argv[]) {
	/* general variables */
	char infile1[256],infile2[256],outfile[256],*line=NULL,*templine=NULL,*pline,*pcol;
	long int i,j,k,m,n,row,col,maxlinelen=MAXLINELEN;
	int v,w,x,y,z,colmatch,result_i[16];
	float a,b,c,d;
	double aa,bb,cc,dd,result_d[64];
	FILE *fpin,*fpout;
	/* program-specific variables */
	char message[256];
	long nrows1,nrows2,ncols1,ncols2,nmatrices1,nmatrices2,n1,n2,ntot1,ntot2,bintot1,bintot2,df;
	long *tot1=NULL,*tot2=NULL;
	double *data1=NULL,*mean1=NULL,*sd1=NULL,*sem1=NULL;
	double *data2=NULL,*mean2=NULL,*sd2=NULL,*sem2=NULL;
	double *tempdata1=NULL,*tempdata2=NULL;
	double diff,tstat,tcrit;
	/* arguments */
	int setpaired=0;
	int settype=1;
	float setalpha=0.05;

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<3) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Calculate the difference between two matrices or multi-matrices\n");
		fprintf(stderr,"Second matrix is subtracted from the first\n");
		fprintf(stderr,"First matrix defines the matrix format (rows & columns)\n");
		fprintf(stderr,"All other matrices must have the same format\n");
		fprintf(stderr,"Uses a T-test to mask non-significant differences (set to NAN) \n");
		fprintf(stderr,"Data may be treated as paired or independent\n");
		fprintf(stderr,"	- if unpaired, the input-avg. in each bin is calculated first\n");
		fprintf(stderr,"	- if paired, the average difference in each bin is used\n");
		fprintf(stderr,"	- if paired, each input must contain the same number of matrices\n");
		fprintf(stderr,"	- if paired, the order of the matrices specifies the pairing\n");
		fprintf(stderr,"\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [infile1] [infile2]\n",thisprog);
		fprintf(stderr,"\n");
		fprintf(stderr,"	[infile1]: file containing reference data matrix/matrices\n");
		fprintf(stderr,"		- format: space-delimited numbers in columns and rows\n");
		fprintf(stderr,"		- multiple matrices must be separated by a blank line\n");
		fprintf(stderr,"		- lines beginning with \"#\" may also separate matrices\n");
		fprintf(stderr,"		- missing values require placeholders (NAN, \"-\", etc.)\n");
		fprintf(stderr,"	[infile2]: the reference matrix is subtracted from this\n");
		fprintf(stderr,"\n");
		fprintf(stderr,"VALID OPTIONS:\n");
		fprintf(stderr,"		-p(aired) t-test? (0=NO, 1=YES) [%d]\n",setpaired);
		fprintf(stderr,"		-a(lpha) significance level for masking output [%g]\n",setalpha);
		fprintf(stderr,"			valid alpha levels: 1, .05 .02 .01 .002 .001\n");
		fprintf(stderr,"		-t(ype) of output [%d]\n",settype);
		fprintf(stderr,"			1= difference (mean2-mean1)\n");
		fprintf(stderr,"			2= ratio (mean2/mean1)\n");
		fprintf(stderr,"			3= t-statistic on the differences\n");
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s matrix1.txt matrix2.txt -p 1 -a 0.01\n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	The average difference between the matrices, masked using -a\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}


	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	sprintf(infile1,"%s",argv[1]);
	sprintf(infile2,"%s",argv[2]);
	for(i=3;i<argc;i++) {
		if( *(argv[i]+0) == '-') {
			if((i+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[i]); exit(1);}
			else if(strcmp(argv[i],"-p")==0) 	{ setpaired=atoi(argv[i+1]); i++;}
			else if(strcmp(argv[i],"-a")==0) 	{ setalpha=atof(argv[i+1]); i++;}
			else if(strcmp(argv[i],"-t")==0) 	{ settype=atoi(argv[i+1]); i++;}
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[i]); exit(1);}
	}}

	if(settype<1||settype>3) {fprintf(stderr,"\n\a--- Error[%s]: invalid -t (%d) - must be 1,2 or 3\n\n",thisprog,settype);exit(1);}
	if(setpaired!=0&&setpaired!=1) {fprintf(stderr,"\n\a--- Error[%s]: invalid -p (%d) - must be 0 or 1\n\n",thisprog,setpaired);exit(1);}
	if(setalpha!=1.0F && setalpha!=0.05F && setalpha!=0.02F && setalpha!=0.01F && setalpha!=0.002F && setalpha!=0.001F) {fprintf(stderr,"\n\a--- Error[%s]: invalid -a (%g) - must be .05 .02 .01 .002 or .001\n\n",thisprog,setalpha);exit(1);}


	/* STORE MULTI-MATRIX DATA #1  */
	n1=ntot1=nrows1=ncols1=nmatrices1=0;
	if((fpin=fopen(infile1,"r"))==0) {fprintf(stderr,"\n\a--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile1);exit(1);}
	data1= xf_matrixread1_d(&nmatrices1,&ncols1,&nrows1,message,fpin);
	fclose(fpin);
	if(data1==NULL) {fprintf(stderr,"\n\a--- Error[%s]: file %s: %s\n\n",thisprog,infile1,message);exit(1);}
	bintot1= nrows1*ncols1;
	ntot1= n1*nmatrices1;

	/* STORE MULTI-MATRIX DATA #2  */
	n2=ntot2=nrows2=ncols2=nmatrices2=0;
	if((fpin=fopen(infile2,"r"))==0) {fprintf(stderr,"\n\a--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile2);exit(1);}
	data2= xf_matrixread1_d(&nmatrices2,&ncols2,&nrows2,message,fpin);
	fclose(fpin);
	if(data2==NULL) {fprintf(stderr,"\n\a--- Error[%s]: file %s: %s\n\n",thisprog,infile2,message);exit(1);}
	bintot2= nrows2*ncols2;
	ntot2= n2*nmatrices2;

	/* MAKE SURE THE TEMPLATE MATRICES FROM EACH FILE HAVE THE SAME DIMENSIONS */
	if(nrows2!=nrows1) {fprintf(stderr,"\n\a--- Error[%s]: number of matrix-rows differs between \"%s\" and \"%s\"\n\n",thisprog,infile1,infile2);exit(1);}
	if(ncols2!=ncols1) {fprintf(stderr,"\n\a--- Error[%s]: number of matrix-columns differs between \"%s\" and \"%s\"\n\n",thisprog,infile1,infile2);exit(1);}

	/* RESERVE MEMORY FOR RESULTS MATRICES */
	if((tempdata1=(double *)realloc(tempdata1,nmatrices1*sizeof(double)))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
	if((tempdata2=(double *)realloc(tempdata2,nmatrices2*sizeof(double)))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};


	/* CALCULATE THE DIFFERENCE FOR EACH BIN - INDEPENDENT SAMPLES */
	if(setpaired==0) for(i=col=0;i<bintot1;i++) {

		/* collect the data from the current bin for each matrix */
		for(j=0;j<nmatrices1;j++) tempdata1[j]=data1[j*bintot1+i];
		for(j=0;j<nmatrices2;j++) tempdata2[j]=data2[j*bintot1+i];

		/* get the stats */
		z= xf_ttest2_d(tempdata1,tempdata2,nmatrices1,nmatrices2,1,result_d);

		tstat= result_d[0];
		df= (long)result_d[1];
		aa= result_d[4]; // mean, group1
		bb= result_d[5]; // mean, group2
		diff= result_d[6]; // difference

		if(setalpha!=1.0F) tcrit= xf_crit_T1(df,setalpha,2);
		else tcrit=0.0;

		if(fabs(tstat)<tcrit) cc= NAN;
		else if(settype==1) cc= diff;
		else if(settype==2) cc= bb/aa;
		else if(settype==3) cc= tstat;

// if(i==0) {
// 	for(j=0;j<nmatrices1;j++) fprintf(stderr,"%d	%g\n",j,tempdata1[j]);
// 	fprintf(stderr,"mean1:%g\n\n",aa);
// 	for(j=0;j<nmatrices2;j++) fprintf(stderr,"%d	%g\n",j,tempdata2[j]);
// 	fprintf(stderr,"mean2:%g\n\n",bb);
//
// 	fprintf(stderr,"diff:%g\n\n",diff);
// 	fprintf(stderr,"cc:%g\n\n",cc);
// }

		printf("%g",cc);
		if(++col<ncols1) printf(" ");
		else { col=0;printf("\n"); }

	}

	/* CALCULATE THE DIFFERENCE FOR EACH BIN - PAIRED SAMPLES */
	if(setpaired==1) for(i=col=0;i<bintot1;i++) {

		/* collect the data from the current bin for each matrix */
		for(j=0;j<nmatrices1;j++) tempdata1[j]=data1[j*bintot1+i];
		for(j=0;j<nmatrices2;j++) tempdata2[j]=data2[j*bintot1+i];

		/* get the stats */
		z= xf_ttest3_d(tempdata1,tempdata2,nmatrices1,1,result_d);

		tstat= result_d[0];
		df= (long)result_d[1];
		aa= result_d[4]; // mean, group1
		bb= result_d[5]; // mean, group2
		diff= result_d[6]; // difference, valid pairs only

		if(setalpha!=1.0F) tcrit= xf_crit_T1(df,setalpha,2);
		else tcrit=0.0;

		if(fabs(tstat)<tcrit) cc= NAN;
		else if(settype==1) cc= diff;
		else if(settype==2) cc= bb/aa;
		else if(settype==3) cc= tstat;

		printf("%g",cc);
		if(++col<ncols1) printf(" ");
		else { col=0;printf("\n"); }

	}

FINISH:
	if(line!=NULL) free(line);
 	if(data1!=NULL) free(data1);
 	if(data2!=NULL) free(data2);
 	if(tempdata1!=NULL) free(tempdata1);
 	if(tempdata2!=NULL) free(tempdata2);

	exit(0);
	}
