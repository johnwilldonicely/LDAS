#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define thisprog "xe-rms2"
#define TITLE_STRING thisprog" v 2: 14.February.2014 [JRH]"
#define MAXLINELEN 1000

/*
<TAGS>stats signal_processing</TAGS>

v 2: 14.February.2014 [JRH]
	- correct reported program-name!
	- improve instructions
	- remove unnecessary inclusion of xf_stats2_d

*/

/* external functions start */
int xf_rms2_d(double *input, double *output, size_t nn, size_t nwin1, char *message);
long xf_interp3_d(double *data, long ndata);
int xf_detrend1_d(double *y, size_t nn, double *result_d);
/* external functions end */

int main (int argc, char *argv[]) {
	/* general variables */
	char infile[256],outfile[256],line[MAXLINELEN];
	long int i,j,k,n;
	size_t ii,jj,kk,nn,sizeofdouble=sizeof(double);
	int v,w,x,y,z,col,colmatch;
	float a,b,c,d;
	double aa,bb,cc,dd,result_d[16];
	FILE *fpin,*fpout;
	/* program-specific variables */
	char message[MAXLINELEN];
	int baddata=0;
	double *data=NULL,*rms=NULL,sum,mean,result[8];
	/* arguments */
	int setwinsize=-1,setverb=0,setout=0, setdetrend=0;

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Calculate root mean square (RMS)  power for a data series\n");
		fprintf(stderr,"- RMS calculated in a sliding window shifted by one sample at a time\n");
		fprintf(stderr,"- add-one/drop-one method use for efficient calculation\n");
		fprintf(stderr,"- interpolates across non-numeric values, NAN and INF\n");
		fprintf(stderr,"- option remove linear trends from data\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [input] [options]\n",thisprog);
		fprintf(stderr,"	[input]: file name or \"stdin\", single column of numbers\n");
		fprintf(stderr,"VALID OPTIONS:\n");
		fprintf(stderr,"	-ws: window size (samples) for RMS calc. (-1 = whole record) [%d]\n",setwinsize);
		fprintf(stderr,"	-d: detrend - removes linear trends from input (0=NO 1=YES) [%d]\n",setdetrend);
		fprintf(stderr,"	-o: output (0=average RMS, 1=RMS at each sample [%d]\n",setout);
		fprintf(stderr,"	-v: set verbosity (0=RMS only, 1=report) [%d]\n",setverb);
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s data.txt \n",thisprog);
		fprintf(stderr,"	cat temp.txt | %s stdin -ws 100\n",thisprog);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}


	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	sprintf(infile,"%s",argv[1]);
	for(i=2;i<argc;i++) {
		if( *(argv[i]+0) == '-') {
			if((i+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[i]); exit(1);}
			else if(strcmp(argv[i],"-ws")==0) 	setwinsize=atoi(argv[++i]);
			else if(strcmp(argv[i],"-d")==0) 	setdetrend=atoi(argv[++i]);
			else if(strcmp(argv[i],"-o")==0) 	setout=atoi(argv[++i]);
			else if(strcmp(argv[i],"-v")==0) 	setverb=atoi(argv[++i]);
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[i]); exit(1);}
	}}

	if(setwinsize<3&&setwinsize!=-1) {fprintf(stderr,"\n--- Error[%s]: -ws must be -1, or greater than 2\n\n",thisprog); exit(1);}
	if(setout!=0&&setout!=1) {fprintf(stderr,"\n--- Error[%s]: option -o [%d] must be 0 or 1\n\n",thisprog,setout); exit(1);}
	if(setverb!=0&&setverb!=1) {fprintf(stderr,"\n--- Error[%s]: option -v [%d] must be 0 or 1\n\n",thisprog,setverb); exit(1);}

	/* READ THE DATA */
	nn=0;baddata=0;
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
	while(fgets(line,MAXLINELEN,fpin)!=NULL) {
		if(sscanf(line,"%lf",&aa)!=1) aa=NAN;
		if(!isfinite(aa)) baddata=1;
		if((data=(double *)realloc(data,(nn+1)*sizeofdouble))==NULL) {
			fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);
			exit(1);
		}
		data[nn++]=aa;
	}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);


	/* INTERPOLATE ACROSS INVALID POINTS IF NECESSARY */
	if(baddata==1) i= xf_interp3_d(data,(long)nn);
	if(i<0) {fprintf(stderr,"\n--- Error[%s]: no valid numerical data in \"%s\"\n\n",thisprog,infile);exit(1);}


	/* REMOVE ANY LINEAR TRENDS FROM THE DATA */
	if(setdetrend==1) {
		x= xf_detrend1_d(data,nn,result);
		if(x<0) {
			fprintf(stderr,"\n--- Error[%s]: problem with detrending function\n\n",thisprog);
			free(data);
			exit(1);
	}}

	//TEST: for(ii=0;ii<nn;ii++) printf("%g\n",data[ii]); free(data);exit(0);

	/* ALLOCATE MEMORY FOR RMS POWER ARRAY - NO NEED TO INITIALIZE */
	if((rms=(double *)realloc(rms,(nn)*sizeofdouble))==NULL) {
		fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);
		free(data);
		exit(1);
	}


	/* CALCULATE THE RMS POWER */
	if(setwinsize==-1) setwinsize=nn;
	x= xf_rms2_d(data,rms,nn,(size_t)setwinsize,message);
	if(x!=0){
		fprintf(stderr,"\n--- Error[%s]: %s\n\n",thisprog,message);
		free(data);
		free(rms);
		exit(1);
	}
	if(setverb==1) fprintf(stderr,"%s: %s\n",thisprog,message);


	/* OUTPUT THE RESULTS */
	if(setout==0) {
		sum=0.0;
		for(ii=0;ii<nn;ii++) sum+=rms[ii];
		printf("%g\n",(sum/(double)nn));
	}
	if(setout==1) {
		for(ii=0;ii<nn;ii++) printf("%g\n",rms[ii]);
	}

	free(data);
	free(rms);
	exit(0);
	}
