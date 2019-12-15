#define thisprog "xe-spectdenoise1"
#define TITLE_STRING thisprog" v 1: 22.January.2019 [JRH]"
#define MAXLINELEN 1000

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
<TAGS>matrix signal_processing spectra noise</TAGS>

v 1: 22.January.2019 [JRH]
	- add clipping capabilities

v 1: 2.January.2019 [JRH]
*/

/* external functions start */
double *xf_matrixread1_d(long *nmatrices, long *ncols, long *nrows, char *message, FILE *fpin);
long xf_spectdenoise1_d(double *matrix1,long width,long height,double setclip,double setz,int setsign,double setper,int setrotate,char *message);
int xf_matrixrotate2_d(double *data1, long *width, long *height, int r);
/* external functions end */

int main (int argc, char *argv[]) {

	/* general variables */
	char outfile[256],*line=NULL,*templine=NULL,word[256],*pline,*pcol,message[MAXLINELEN];
	long ii,jj,kk,ll,mm,nn,maxlinelen=0;
	int v,w,x,y,z,col,colmatch;
	float a,b,c,d,result_f[64];
	double aa,bb,cc,dd,ee,result_d[64];
	FILE *fpin,*fpout;
	/* program-specific variables */
	long *freqcount=NULL,nmatrices,width,height;
	double *matrix=NULL,*temprow=NULL;
	/* arguments */
	char *infile=NULL;
	int setsign=0,setrotate=0,setverb=0;
	double setclip=-1.0,setz=1.0,setp=25.0;


	if((line=(char *)realloc(line,6))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
	if((templine=(char *)realloc(templine,MAXLINELEN))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};

	/********************************************************************************
	PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED
	********************************************************************************/
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Remove noise from a spectral-matrix (time x freq)\n");
		fprintf(stderr," - noise=  single-timepoint increases that span many frequencies\n");
		fprintf(stderr," - timeseries for each freq. converted to Z-scores for thresholding\n");
		fprintf(stderr," - outputs a modified matrix with noise-timepoints set to NAN\n");
		fprintf(stderr,"USAGE: %s [in] [options]\n",thisprog);
		fprintf(stderr,"	[in]: filename or \"stdin\", format: column=time row=freq\n");
		fprintf(stderr,"VALID OPTIONS: defaults in []\n");
		fprintf(stderr,"	-c: clipping value (de-noising only,-1=noclip) [%g]\n",setclip);
		fprintf(stderr,"	-z: Z-score threshold for noise at each freq (NAN=skip) [%g]\n",setz);
		fprintf(stderr,"	-s: sign of thesholding (-1=NEG,+1=POS,0=BOTH) [%d]\n",setsign);
		fprintf(stderr,"	-p: %% of freq > z needed to invalidate timepoint [%g]\n",setp);
		fprintf(stderr,"		- e.g. if Z>%g for %g%% of the spectrum at column 123\n",setz,setp);
		fprintf(stderr,"	-r: 90-deg rotate for analysis (0=NO 1=YES) [%d]\n",setrotate);
		fprintf(stderr,"		- use for if input column=freq and row=time\n");
		fprintf(stderr,"		- matrix will be rotated back for output\n");
		//fprintf(stderr,"	-verb: verbose output (0=NO 1=YES 999=DEBUG) [%d]\n",setverb);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	stdout: matrix with noise timepoints invalidated (NAN)\n");
		fprintf(stderr,"	stderr: summary\n");
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s matrix1.txt 2> report.txt 1> matrix2.txt\n",thisprog);
		fprintf(stderr,"	cat matrix1.txt | %s stdin -r 1 > matrix2.txt\n",thisprog);
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
			else if(strcmp(argv[ii],"-c")==0)    setclip= atof(argv[++ii]);
			else if(strcmp(argv[ii],"-s")==0)    setsign= atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-z")==0)    setz= atof(argv[++ii]);
			else if(strcmp(argv[ii],"-p")==0)    setp= atof(argv[++ii]);
			else if(strcmp(argv[ii],"-r")==0)    setrotate= atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-verb")==0) setverb= atoi(argv[++ii]);
			else {fprintf(stderr,"\n--- Error [%s]: invalid command line argument [%s]\n\n",thisprog,argv[ii]); exit(1);}
	}}
	if(setverb!=0 && setverb!=1 && setverb != 999) { fprintf(stderr,"\n--- Error [%s]: invalid -verb [%d] must be 0,1, or 999\n\n",thisprog,setverb);exit(1);}
	if(setsign<-1||setsign>1) {fprintf(stderr,"\n--- Error[%s]: invalid -s [%d] must be -1, 0 or 1\n\n",thisprog,setsign);exit(1);}
	if(setrotate!=0&&setrotate!=1) {fprintf(stderr,"\n--- Error[%s]: invalid -r [%d] must be 0 or 1\n\n",thisprog,setrotate);exit(1);}
	if(setz==0.0) {fprintf(stderr,"\n--- Error[%s]: invalid -z [%d], cannot be zero\n\n",thisprog,setz);exit(1);}
	if(setp<=0.0) {fprintf(stderr,"\n--- Error[%s]: invalid -per [%d], must be >0\n\n",thisprog,setp);exit(1);}
	//TEST printf("setz=%g\n",setz); goto END;

	/********************************************************************************
	STORE THE MATRIX -  THIS METHOD IS ROBUST AGAINST LINES OF UNKNOWN LENGTH
	********************************************************************************/
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
	matrix= xf_matrixread1_d(&nmatrices,&width,&height,message,fpin);
	if(strcmp(infile,"stdin")!=0) fclose(fpin);
	if(matrix==NULL) {fprintf(stderr,"\n--- Error[%s]: %s\n\n",thisprog,message);exit(1);}
	nn= width*height;

	/********************************************************************************
	REMOVE NOISE
	********************************************************************************/
	mm=0; bb=0.0;
	if(isfinite(setz)) {
		mm= xf_spectdenoise1_d(matrix,width,height,setclip,setz,setsign,setp,setrotate,message);
		if(mm==-1) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }
		bb= 100.0*(double)mm/(double)width;
	}
	fprintf(stderr,"sd= %g per= %g noise= %.2f %% count=%ld\n",setz,setp,bb,mm);

	/********************************************************************************
	OUTPUT THE MATRIX
	********************************************************************************/
	for(ii=0;ii<height;ii++) {
		jj= ii*width;
		kk= jj+width;
		printf("%g",matrix[jj++]);
		for(jj=jj;jj<kk;jj++) printf(" %g",matrix[jj]); // only print a tab separator for columns after the first column
		printf("\n");
	}

	/********************************************************************************
	CLEANUP AND EXIT - jump here using goto END;
	********************************************************************************/
END:
	if(matrix!=NULL) free(matrix);
	if(templine!=NULL) free(templine);
	if(temprow!=NULL) free(temprow);
	if(freqcount!=NULL) free(freqcount);
	exit(0);
}
