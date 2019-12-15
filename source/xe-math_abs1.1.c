#define thisprog "xe-math_abs1"
#define TITLE_STRING thisprog" v 1: 9.January.2016 [JRH]"

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#define MAXLINELEN 1000

/*
<TAGS>math </TAGS>

v 1: 9.January.2016 [JRH]
 */

/* external functions start */
/* external functions end */


int main(int argc, char *argv[]) {
	/******************************************************************************
	Initialise variables
	******************************************************************************/
	char infile[256],message[256],line[MAXLINELEN+1],word[MAXLINELEN+1],*pline,*pcol;
	int x,y,z,col,decimals;
	long int i,j,k;
	float a,b,c,*data=NULL,mean,sd,var;
	double aa,bb,cc,dd,resultd[64];
	FILE *fpin;
	/* arguments */
	int dvcol=1;

	/******************************************************************************
	If only one argument (executable's name) print instructions
	******************************************************************************/
	if(argc<2) 	{
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Convert a column of data to its absolute value\n");
		fprintf(stderr,"Non-numeric fields will not be modified\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr," 	%s [infile|stdin] [arguments]\n",thisprog);
		fprintf(stderr,"Valid arguments - defaults in []:\n");
		fprintf(stderr,"	-cy column containing data (0=none, -1=all) [%d]\n",dvcol);
		fprintf(stderr,"Examples:\n");
		fprintf(stderr,"	cat datafile.txt | %s stdin -cy 1\n",thisprog);
		fprintf(stderr,"	%s datafile.txt -cy 2\n",thisprog);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	sprintf(infile,"%s",argv[1]);
	for(i=2;i<argc;i++) {
		if( *(argv[i]+0) == '-') {
			if((i+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[i]); exit(1);}
			else if(strcmp(argv[i],"-cy")==0) 	{ dvcol=atoi(argv[i+1]); i++;}
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[i]); exit(1);}
	}}

	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}

	while(fgets(line,MAXLINELEN,fpin)!=NULL) {
		if(line[0]=='#') continue;
		pline=line;
		for(col=1;(pcol=strtok(pline," ,\t\n"))!=NULL;col++) {
			pline=NULL;
			if(col>1)printf("\t",pcol);
			if(col==dvcol||dvcol==-1) {
				if(sscanf(pcol,"%lf",&aa)!=1 || !isfinite(aa)) {printf("%s",pcol);continue;}
				if(aa<0.0) aa=0.0-aa;
				printf("%g",aa);
			}
			else printf("%s",pcol);
		}
		printf("\n");
	}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);
}
