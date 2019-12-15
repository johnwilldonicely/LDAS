#define thisprog "xe-matrixstats1"
#define TITLE_STRING thisprog" v 1: 19.March.2018 [JRH]"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
<TAGS>matrix</TAGS>

v 3: 21.October.2017 [JRH]
*/


/* external functions start */
long xf_matrixread2_d(char *infile, long idcol, double **matrix1, double **id1, long *ncols, long *nrows, char *message);
char *xf_lineread1(char *line, long *maxlinelen, FILE *fpin);
long *xf_lineparse1(char *line,long *nwords);
int xf_smoothgauss2_d(double *data,int xbintot,int ybintot,int xsmooth,int ysmooth);
double xf_correlate_simple_d(double *x, double *y, long nn, double *result_d);
int xf_percentile1_d(double *data, long n, double *result);
int xf_compare1_d(const void *a, const void *b);
double xf_matrixcoh1_d(double *rate,long width,long height,char *message);

/* external functions end */

int main (int argc, char *argv[]) {
	/* general variables */
	char outfile[256],*line=NULL,*templine=NULL,*pline,*pcol,message[256];
	long ii,jj,kk,nn,*index=NULL;
	int v,w,x,y,z,n,result_i[32];
	float a,b,c,d;
	double aa,bb,cc,dd,result_d[8];
	FILE *fpin,*fpout;
	/* program-specific variables */
	long nmatrices,width,height,count=0;
	double xmin,xmax,ymin,ymax;
	double *matrix1=NULL,*id1=NULL,*pmatrix1,binsize=1;
	double rate_mean,rate_base,rate_max,rate_median,rate_peak,space_info,space_spar,space_coh;

	/* arguments */
	char *filematrix;
	long setidcol=1;
	double setbinsize=1.0;

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Analyze content of a single- or multi-matrix ASCII file\n");
		fprintf(stderr,"USAGE: %s [matrix] [options]\n",thisprog);
		fprintf(stderr,"	[matrix]: file or \"stdin\" in (multi)matrix format\n");
		fprintf(stderr,"		- matrices separated by \"# <id-number>\" lines\n");
		fprintf(stderr,"VALID OPTIONS (defaults in []):\n");
		fprintf(stderr,"	-idcol: the column on comment-lines holding the ID [%ld]\n",setidcol);
		fprintf(stderr,"	-binsize: 4-item CSV list defining xmin,ymin,xmax,ymax [unset]\n");
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s matrix.txt -thresh 1 -first 0 > matrix2.txt ; } 2> report.txt \n",thisprog);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	filematrix= argv[1];
	for(ii=2;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-idcol")==0) setidcol=atol(argv[++ii]);
			else if(strcmp(argv[ii],"-binsize")==0)  setbinsize= atof(argv[++ii]);
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
	}}

	/********************************************************************************/
	/* STORE THE MATRICES */
	/********************************************************************************/
	/* call the read function */
	nmatrices= xf_matrixread2_d(filematrix,setidcol,&matrix1,&id1,&width,&height,message);
	if(nmatrices==-1) {fprintf(stderr,"\n--- Error[%s]: %s\n\n",thisprog,message);exit(1);}
	nn= width*height;

	/********************************************************************************/
	/* DEFINE THE DATA-POSITION RANGES AUTOMATICALLY */
	/********************************************************************************/
	xmin=0; xmax= (double)width*binsize;
	ymin=0; ymax= (double)height*binsize;
	//
	fprintf(stderr,"xmin=%g ymin=%g xmax=%g ymax=%g\n",xmin,ymin,xmax,ymax);


	/********************************************************************************/
	/* FOR EACH MATRIX */
	/********************************************************************************/
	printf("matrix	id	width	height	coh\n");
	for(ii=0;ii<nmatrices;ii++) {

		/* set the matrix pointer */
		pmatrix1= matrix1+(nn*ii);

		space_coh= xf_matrixcoh1_d(pmatrix1,width,height,message);

		if(id1[ii]==(long)id1[ii])
			printf("%ld\t%ld\t%ld\t%ld\t%g\n",ii,(long)id1[ii],width,height,space_coh);
		else
			printf("%ld\t%g\t%ld\t%ld\t%g\n",ii,id1[ii],width,height,space_coh);

	} // END OF PER-MATRIX LOOP (KK)

	/* FREE MEMORY AND EXIT */
	if(matrix1!=NULL) free(matrix1);
	if(id1!=NULL) free(id1);
	exit(0);

	}
