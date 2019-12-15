#define thisprog "xe-matrixcut1"
#define TITLE_STRING thisprog" v 1: 2.March.2018 [JRH]"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
<TAGS>matrix</TAGS>

v 1: 2.March.2018 [JRH]
	- user can now define whether a header-comment is included in output

v 1: 27.February.2018 [JRH]
	- allow user to specify column in which identifier is found
	- update instructions
v 1: 22.October.2017 [JRH]
	- first created
*/


/* external functions start */
long xf_matrixread2_d(char *infile, long idcol, double **matrix1, double **id1, long *ncols, long *nrows, char *message);
char *xf_lineread1(char *line, long *maxlinelen, FILE *fpin);
long *xf_lineparse1(char *line,long *nwords);
/* external functions end */

int main (int argc, char *argv[]) {

	/* general variables */
	char *line=NULL,*templine=NULL,*pline,*pcol,message[256];
	long ii,jj,kk,nn;
	FILE *fpin,*fpout;
	/* program-specific variables */
	long nmatrices,width,height,count;
	double *matrix1=NULL,*id1=NULL,*pmatrix1;
	/* arguments */
	char *filematrix;
	int sethead=1;
	long setidcol=1;
	double setid=1.0;

	/********************************************************************************/
	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	/********************************************************************************/
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Extract a single matrix from a multi-matrix file\n");
		fprintf(stderr,"- looks for a unique numeric ID on a line starting with \"# \"\n");
		fprintf(stderr,"USAGE: %s [matrix] [options]\n",thisprog);
		fprintf(stderr,"	[matrix]: file or \"stdin\" in (multi)matrix format\n");
		fprintf(stderr,"		- matrices separated by \"# <id-number>\" lines\n");
		fprintf(stderr,"VALID OPTIONS (defaults in []):\n");
		fprintf(stderr,"	-idcol: zero-offset column on comment-lines holding the ID [%ld]\n",setidcol);
		fprintf(stderr,"	-id:  numeric ID to match [%g]\n",setid);
		fprintf(stderr,"	-head:  output header line (0=NO 1=YES) [%g]\n",sethead);
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s matrix.txt -id 001\n",thisprog);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}


	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	filematrix= argv[1];
	for(ii=2;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-idcol")==0) setidcol= atol(argv[++ii]);
			else if(strcmp(argv[ii],"-id")==0) setid= atof(argv[++ii]);
			else if(strcmp(argv[ii],"-head")==0) sethead= atoi(argv[++ii]);
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
	}}
	if(sethead!=0 && sethead!=1) { fprintf(stderr,"\n--- Error [%s]: invalid -head [%d] must be 0 or 1\n\n",thisprog,sethead);exit(1);}

	/* STORE THE MULTI-MATRIX */
	nmatrices= xf_matrixread2_d(filematrix,setidcol,&matrix1,&id1,&width,&height,message);
	if(nmatrices==-1) {fprintf(stderr,"\n--- Error[%s]: %s\n\n",thisprog,message);exit(1);}
	nn= width*height;

	/* OUTPUT EACH MATRIX MATCHING THE ID  */
	for(kk=0;kk<nmatrices;kk++) {
		if(id1[kk]==setid) {
			/* set the matrix pointer */
			pmatrix1= matrix1+(nn*kk);
			/* replicate the id-portion of the header line */
			if(sethead==1) {
				if(id1[kk]==(long)id1[kk]) printf("# %ld\n",(long)id1[kk]);
				else printf("# %.16f\n",id1[kk]);
			}
			/* output the matrix*/
			for(ii=count=0;ii<nn;ii++) {
				if(++count>=width) {
					printf("%g\n",pmatrix1[ii]);
					count=0;
				}
				else printf("%g\t",pmatrix1[ii]);
			}
		}
	}

	/* FREE MEMORY AND EXIT */
	if(matrix1!=NULL) free(matrix1);
	if(id1!=NULL) free(id1);
	exit(0);

	}
