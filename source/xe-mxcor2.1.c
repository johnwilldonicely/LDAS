#define thisprog "xe-mxcor2"
#define TITLE_STRING thisprog" v 2.1: 7.February.2019 [JRH]"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define MAXLINELEN 1000
#define MAXCELLS 1000
/*
<TAGS>signal_processing matrix</TAGS>

Versions History:
v 2.1: 7.February.2019 [JRH]
	- bugfix: output was referring to an unused results variable

v 2.1: 18.April.2016 [JRH]
	- update correlate function used - switch to more robust float version
	- update to new xf_strkey1 function (replaces hux_getword)

v.2.1: JRH, 16 October 2010
*/

/* external functions start */
int xf_correlate_f(float *x, float *y, long nn, float setinv, double *result, char *message);
float xf_prob_F(float F,int df1,int df2);
int xf_strkey1(char *input, char *key, long word, char *output);
/* external functions end */

int main (int argc, char *argv[]) {

	/* general and file type variables */
	char line[MAXLINELEN],*pline,*pcol,*tempcol,temp_str[256],message[256];
	long i,j,k;
	int w,x,y,z,n,skip,col,systemtype,sizeofint=sizeof(int);
	long fpintemp;
	float a,b,c,d,e;
	double aa,bb,cc,result_d[32];
	FILE *fpin,*fpout;

	/* special variable for this program */
	int cell,celltot=0,npairs=0,mwidth=-1,mcell[MAXCELLS],celln[MAXCELLS],mcelltot=0,badcount=0;
	int *pair1=NULL,*pair2=NULL;
	long N;
	float *data=NULL,*tempfa1,*tempfa2;
	/* command line arguments */
	char matrixfile[256],cellfile[256];
	int setcellfile=0,setcell1=1,setcell2=1;

	/* PRINT INSTRUCTIONS IF ONLY ONE ARGUMENT */
	if(argc<2) {
		fprintf(stderr,"\n********************************************************************\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"Calculate spatial correlation between pairs of matrix-format rate maps\n");
		fprintf(stderr,"Reads one CRUNCH matrix output file containing multiple matrices\n");
		fprintf(stderr,"Requires either cell-ids for a pair to analyze, or a pair-list file\n");
		fprintf(stderr,"Outputs the spatial correlation for every cell pair specified\n");
		fprintf(stderr,"USAGE: %s [matrixfile] [arguments]\n",thisprog);
		fprintf(stderr,"- valid arguments: \n");
		fprintf(stderr,"	-c1: cell 1 of a pair [%d]\n",setcell1);
		fprintf(stderr,"	-c2: cell 2 of a pair [%d]\n",setcell2);
		fprintf(stderr,"	-cf: file listing cell pairs\n");
		fprintf(stderr,"		NOTE: this overrides -c1 and -c2\n");
		fprintf(stderr,"Examples:\n");
		fprintf(stderr,"	%s crunch_matrix.txt -c1 12 -c2 13\n",thisprog);
		fprintf(stderr,"	%s crunch_matrix.txt -cf cellpairs.txt\n",thisprog);
		fprintf(stderr,"\n");
		exit(0);
	}

	/* READ THE COMMAND LINE ARGUMENTS*/
	strcpy(matrixfile,argv[1]);	// the matrix file
	for(i=2;i<argc;i++) {
			if( *(argv[i]+0) == '-') {
				if((i+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[i]); exit(1);}
				else if(strcmp(argv[i],"-c1")==0) 	{ setcell1=atoi(argv[i+1]); i++; }
				else if(strcmp(argv[i],"-c2")==0) 	{ setcell2=atoi(argv[i+1]); i++; }
				else if(strcmp(argv[i],"-cf")==0) 	{ setcellfile=1; sprintf(cellfile,"%s\0",argv[i+1]); i++;}
				else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[i]); exit(1);}
	}}

	/* READ THE MATRIX FILE TO GET WIDTH */
	mwidth=-1;
	if((fpin=fopen(matrixfile,"r"))==NULL) {fprintf(stderr,"--- Error[%s]: can't open \"%s\"\n\n",thisprog,matrixfile);exit(1);}
	while(fgets(line,MAXLINELEN,fpin)!=NULL) {
		if(xf_strkey1(line, "# Dwell map: ", 1, temp_str)>0) mwidth=atoi(temp_str);
	}
	if(mwidth<=0) {fprintf(stderr,"--- Error[%s]: matrix file %s is missing width-definition line\n",thisprog,matrixfile);exit(1);}
	else N=mwidth*mwidth;
	data= (float *) malloc(N*MAXCELLS*sizeof(float)+1); if(data==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}

	/* RE-READ THE MATRIX FILE TO STORE MATRIX DATA */
	rewind(fpin);
	while(fgets(line,MAXLINELEN,fpin)!=NULL) {
		// finding a line with the word "Cell" triggers data-read
		if(xf_strkey1(line, " Cell ", 1, temp_str)>0) {
			cell=atoi(temp_str);
			if(cell>=MAXCELLS) {fprintf(stderr,"--- Error[%s]: cells defined exceeds max (%d)\n",thisprog,MAXCELLS);exit(1);}
			else mcell[mcelltot]=cell;
			// read the cell rate-matrix
			for(i=0;i<N;i++) {
				if(fscanf(fpin,"%f",&data[cell*N+i])!=1) {
					fprintf(stderr,"--- Error [%s]: bad or missing data in file %s cell %d\n",thisprog,matrixfile,cell);exit(1);}
				else celln[cell]++;
			}
			mcelltot++;
	}}
	fclose(fpin);


	/* STORE CELL-PAIR LIST - WHICH PAIRS TO ANALYZE */
	if(setcellfile==1) {
		if((fpin=fopen(cellfile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,cellfile);exit(1);}
		npairs=0;
		while(!feof(fpin)) {
			if(fscanf(fpin,"%d %d",&x,&y)!=2) {fscanf(fpin,"%s",&line); continue;}
			pair1= (int *) realloc(pair1,(npairs+1)*sizeofint);
			pair2= (int *) realloc(pair2,(npairs+1)*sizeofint);
			if(pair1==NULL || pair2==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
			pair1[npairs]=x; pair2[npairs]=y; npairs++;
		}
		fclose(fpin);
	}
	/* OTHERWISE ASSIGN VALUES FROM -C1 AND -C2 TO THE CELL PAIR LISTS (LIST LENGTH IN THIS CASE = 1) */
	else {
		pair1= (int *) realloc(pair1,(npairs+1)*sizeofint);
		pair2= (int *) realloc(pair2,(npairs+1)*sizeofint);
		if(pair1==NULL || pair2==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
		pair1[npairs]=setcell1; pair2[npairs]=setcell2; npairs=1;
	}

	/* CALCULATE CORRELATION FOR EACH PAIR IN LIST - PRODUCE AN OUTPUT LINE FOR EVERY PAIR */
	badcount=0;
	printf("c1	c2	n	r	F	p\n");
	for(i=0;i<npairs;i++) {
			x=pair1[i]; y=pair2[i]; z=0;
			// make sure each cell in the list is present in the matrix file
			for(j=0;j<mcelltot;j++) if(x==mcell[j]) {z++; break;}
			for(j=0;j<mcelltot;j++) if(y==mcell[j]) {z++; break;}
			if(z<2) {
				printf("%d	%d	-	-	-	-\n",x,y); continue;
			}
			else if(celln[x]<3||celln[y]<3) {printf("%d	%d	-	-	-	-\n",x,y);continue;}
			else {
				tempfa1=data+x*N; // pointer to beginning of data for cell-1
				tempfa2=data+y*N; // pointer to beginning of data for cell-2
				xf_correlate_f(tempfa1,tempfa2,N,-1,result_d,message);
				printf("%d	%d	%d	%.3f	%.3f	%f\n",x,y,(int)result_d[0],result_d[1],result_d[4],result_d[7]);
	}}

	if(badcount>0) fprintf(stderr,"\n--- Warning[%s]: %d cell-pairs were missing from %s\n\n",thisprog,badcount,matrixfile);

	free(data); free(pair1); free(pair2);
	exit(0);
}
