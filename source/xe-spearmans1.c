#define thisprog "xe-spearmans1"
#define TITLE_STRING thisprog" v 1: 17.July.2018 [JRH]"
#define MAXLINELEN 1000
#define MAXLABELS 1000

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
<TAGS>stats</TAGS>

v 1: 10.August.2018 [JRH]
	- bugfix - corrected assignment of ranks to values
v 1: 14.August.2012 [JRH]
	- bugfix - changed use of fscanf to read data with fgets/sscanf, to avoid problems related to "-" and "."
*/

/* external functions start */
char *xf_lineread1(char *line, long *maxlinelen, FILE *fpin);
long *xf_lineparse2(char *line,char *delimiters, long *nwords);
double xf_spearmans1_f(float *data1, float *data2, long ndata, char *message);
double xf_prob_T1(double t, long df, int tails);
float xf_prob_F(float F,int df1,int df2);
void xf_qsortindex1_f(float *data, long *index,long nn);
/* external functions end */


int main (int argc, char *argv[]) {

	/* general variables */
	char *line=NULL,*templine=NULL,*pline,*pcol,message[MAXLINELEN];
	int v,w,x,y,z,col,colmatch;
	long ii,jj,kk,ll,mm,nn,nbad,nchars=0,maxlinelen=0;
	float a,b,c,d,result_f[64];
	double aa,bb,cc,dd,ee,result_d[64];
	FILE *fpin,*fpout;

	/* program-specific variables */
	char *infile=NULL,*words=NULL;
	int sizeofx,sizeofy;
	long nwords=0,*iword=NULL;
	float *xdatf=NULL,*ydatf=NULL,prob;
	double rho=NAN,fstat,tstat;

	/* arguments */
	int setverb=0;
	long setcolx=1,setcoly=2;

	if((line=(char *)realloc(line,6))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
	sizeofx= sizeof(*xdatf);
	sizeofy= sizeof(*ydatf);


	/********************************************************************************
	PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED
	********************************************************************************/
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Calculate the Spearman's rank-order correlation (rho)\n");
		fprintf(stderr,"- non-parametric relatoinship between the rankings of two variables\n");
		fprintf(stderr,"- relationship need not be linear, merely monotonic\n");
		fprintf(stderr,"- similar to Pearson's r for eliptical distributions\n");
		fprintf(stderr,"- less sensitive to the influence of outliers\n");
		fprintf(stderr,"- non-numeric values ignored\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [input] [options]\n",thisprog);
		fprintf(stderr,"	[input]: file name or \"stdin\" with x and y values in columns\n");
		fprintf(stderr,"VALID OPTIONS: defaults in []\n");
		fprintf(stderr,"	-cx: column holding x-value (first col = 1) [%ld]\n",setcolx);
		fprintf(stderr,"	-cy: column holding y-value (first col = 1) [%ld]\n",setcoly);
		fprintf(stderr,"	-verb: verbose output (0=NO 1=YES, 999=DEBUG) [%d]\n",setverb);
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s data.txt -cx 2 -cy 3\n",thisprog);
		fprintf(stderr,"	cat temp.txt | %s stdin\n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	rho= [Spearmans rank coefficient]\n");
		fprintf(stderr,"	n= [number of valid data-pairs]]\n");
		fprintf(stderr,"	F= [F-statistic] \n");
		fprintf(stderr,"	prob= [probability]\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}



	/********************************************************************************
	READ THE FILENAME AND OPTIONAL ARGUMENTS - including comma-separated list item
	********************************************************************************/
	infile= argv[1];
	for(ii=2;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-cx")==0)   setcolx=atol(argv[++ii]);
			else if(strcmp(argv[ii],"-cy")==0)   setcoly=atol(argv[++ii]);
			else if(strcmp(argv[ii],"-verb")==0) setverb=atoi(argv[++ii]);
			else {fprintf(stderr,"\n*** %s [ERROR: invalid command line argument \"%s\"]\n\n",thisprog,argv[ii]); exit(1);}
	}}
	if(setverb!=0 && setverb!=1 && setverb!=999) { fprintf(stderr,"\n--- Error [%s]: invalid -verb [%d] must be 0,1 or 999\n\n",thisprog,setverb);exit(1);}

	setcolx--;
	setcoly--;

	/********************************************************************************
	STORE DATA - ASSUME WE DON'T KNOW THE LENGTH OF EACH INPUT LINE
	********************************************************************************/
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
	nn=0;
	while((line=xf_lineread1(line,&maxlinelen,fpin))!=NULL) {
		if(maxlinelen==-1)  {fprintf(stderr,"\n--- Error[%s]: readline function encountered insufficient memory\n\n",thisprog);exit(1);}
		/* parse the line */
		iword= xf_lineparse2(line,"\t",&nwords);
		if(nwords<0) {fprintf(stderr,"\n--- Error[%s]: lineparse function encountered insufficient memory\n\n",thisprog);exit(1);};
		/* make sure required columns are present */
		if(nwords<setcolx || nwords<setcoly) continue;
		/* make sure content in x- and y-columns is numeric */
		if(sscanf(line+iword[setcolx],"%f",&a)!=1 || !isfinite(a)) continue;
		if(sscanf(line+iword[setcoly],"%f",&b)!=1 || !isfinite(b)) continue;
		/* dynamically allocate memory */
		if((xdatf= realloc(xdatf,(nn+1)*sizeofx))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
 		if((ydatf= realloc(ydatf,(nn+1)*sizeofy))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		/* store values */
		xdatf[nn]= a;
		ydatf[nn]= b;
		/* increment data-counter */
		nn++;
	}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);

	/* DIAGNOSTIC */
	if(setverb==999) {
		fprintf(stderr,"\n");
		for(ii=0;ii<nn;ii++) fprintf(stderr,"%f	%f\n",xdatf[ii],ydatf[ii]);
		fprintf(stderr,"\n");
	}


	/********************************************************************************
	CALCULATE SPEARMAN'S RANK CORRELATION
	********************************************************************************/
	rho= xf_spearmans1_f(xdatf,ydatf,nn,message);
	if(!isfinite(rho)) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }

	/* calculate statistics */
	tstat= rho * sqrt((nn-2)/(1-rho*rho));
	fstat= tstat*tstat;
	prob= xf_prob_F((float)fstat,1,(nn-2));


	printf("rho= %g\n",rho);
	printf("n= %ld\n",nn);
	printf("F= %g\n",fstat);
	printf("prob= %.6f\n",prob);


END:
	if(xdatf!=NULL) free(xdatf);
	if(ydatf!=NULL) free(ydatf);
	if(words!=NULL) free(words);
	if(iword!=NULL) free(iword);
	if(line!=NULL) free(line);
	exit(0);
}
