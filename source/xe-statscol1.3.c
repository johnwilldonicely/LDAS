#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define thisprog "xe-statscol1"
#define TITLE_STRING thisprog" v 3: 1.April.2014 [JRH]"

/*
<TAGS>math stats</TAGS>

v 7: 16.February.2016 [JRH]
	- avoid in-loop realloc for some variables to improve performance
	- bugfix: column stats now do actually ignore NANs and INFs as suggested by the instructions!

v 7: 1.April.2014 [JRH]
	- don't pre-allocate memory for line - unnecessary (let xf_lineread function handle it)
	- free line at end of program

v 2: 5.May.2013 [JRH]
	- update USAGE of qsort to call external compare function xf_compare1_l
*/


/* external functions start */
char *xf_lineread1(char *line, long *maxlinelen, FILE *fpin);
int xf_stats2_d(double *data, long n, int varcalc, double *result_d);
int xf_compare1_l(const void *a, const void *b);
/* external functions end */


int main (int argc, char *argv[]) {
	/* general variables */
	char infile[256],outfile[256],*line=NULL,word[256],*pline,*pcol;
	long int i,j,k,n,maxlinelen=0;
	int v,w,x,y,z,col,colmatch;
	int sizeofchar=sizeof(char),sizeofshort=sizeof(short),sizeoflong=sizeof(long),sizeofint=sizeof(int),sizeoffloat=sizeof(float),sizeofdouble=sizeof(double);
	float a,b,c,d;
	double aa,bb,cc,dd, result_d[64];
	FILE *fpin,*fpout;
	/* program-specific variables */
	long *column=NULL,*collist=NULL,ncollist=0,n2;
	double *data=NULL,*tempdata=NULL;
	/* arguments */
	int setformat=1,setbintot=25,coldata=1,setnbuff;
	float setlow=0.0,sethigh=0.0,setbinwidth=0.0;

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Calculate statistics on columns of numbers\n");
		fprintf(stderr,"Non-numeric values will be ignored\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [input] [options]\n",thisprog);
		fprintf(stderr,"	[input]: file with multiple columns of data\"\n");
		fprintf(stderr,"VALID OPTIONS:\n");
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s data.txt\n",thisprog);
		fprintf(stderr,"	cut -f 1-3 temp.txt | %s stdin\n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	col  n  mean  stdev  sem\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	sprintf(infile,"%s",argv[1]);
	for(i=2;i<argc;i++) {
		if( *(argv[i]+0) == '-') {
			if((i+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[i]); exit(1);}
			else if(strcmp(argv[i],"-t")==0) 	{ setformat=atoi(argv[i+1]); i++;}
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[i]); exit(1);}
	}}


	/* STORE DATA METHOD 4 - if uncertain about total line length (eg reading large matrices) */
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
	n=0;

	while((line=xf_lineread1(line,&maxlinelen,fpin))!=NULL) {
		if(maxlinelen==-1)  {fprintf(stderr,"\n--- Error[%s]: readline function encountered insufficient memory\n\n",thisprog);exit(1);}
 		pline=line;
 		for(col=0;(pcol=strtok(pline," ,\t\n"))!=NULL;col++)	{
			pline=NULL;
			if((data=(double *)realloc(data,(n+1)*sizeofdouble))==NULL) {fprintf(stderr,"\n\a--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
			if((column=(long *)realloc(column,(n+1)*sizeoflong))==NULL) {fprintf(stderr,"\n\a--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
			if(sscanf(pcol,"%lf",&data[n])!=1 || !isfinite(data[n]) ) data[n]=NAN;
			column[n]=col;
			n++;
	}}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);
	if((tempdata=(double *)realloc(tempdata,n*sizeofdouble))==NULL) {fprintf(stderr,"\n\a--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
	if((collist=(long *)realloc(collist,n*sizeoflong))==NULL) {fprintf(stderr,"\n\a--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}

	/* make a list of the unique elements in "column" */
	for(i=0;i<n;i++) collist[i]=column[i];
	qsort(collist,n,sizeof(long),xf_compare1_l);
	k=collist[0]; for(i=ncollist=1;i<n;i++) {if(collist[i]!=k) collist[ncollist++]=collist[i];k=collist[i]; }

	printf("col	n	mean	stdev	sem\n");
	for(i=0;i<ncollist;i++) {
		n2=0;
		for(j=0;j<n;j++) {
			if(column[j]==i && isfinite(data[j])) tempdata[n2++]=data[j];
		}
		xf_stats2_d(tempdata,n2,1,result_d);
		printf("%d	%d	%g	%g	%g\n",i,n2,result_d[0],result_d[2],result_d[3]);
	}

	if(line!=NULL) free(line);
	free(data);
	free(tempdata);
	free(column);
	free(collist);
	exit(0);
	}
