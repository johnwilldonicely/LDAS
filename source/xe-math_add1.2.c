#define thisprog "xe-math_add1"
#define TITLE_STRING thisprog" v 2: 27.November.2017 [JRH]"

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#define MAXLINELEN 2000

/*
<TAGS>math </TAGS>
v 2: 27.November.2017 [JRH]
	- switch to using %f for output to avoid precision loss with exponential notation
v 2: 26.September.2016 [JRH]
	- add support for long integer input
	- update variable usage
v 2: 17.February.2015 [JRH]
	- enabled operation on all columns (-cy -1)
	- fixed bug where tab's wre output at the end of each line
v 1: 30.April.2012 [JRH]
 */

/* external functions start */
/* external functions end */


int main(int argc, char *argv[]) {
	/******************************************************************************
	Initialise variables
	******************************************************************************/
	char infile[256],message[256],line[MAXLINELEN+1],word[MAXLINELEN+1],*pline,*pcol;
	int x,y,z,col,decimals;
	long ii,jj,kk;
	float a,b,c,*data=NULL,mean,sd,var;
	double aa,bb,cc,dd,resultd[64];
	FILE *fpin;
	/* arguments */
	int dvcol=1,setlong=0;
	long setvall=0.0;
	double setvald=0.0;

	/******************************************************************************
	If only one argument (executable's name) print instructions
	******************************************************************************/
	if(argc<2) 	{
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Add a number to a column of data\n");
		fprintf(stderr,"Non-numeric fields will not be modified\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr," 	%s [infile|stdin] [arguments]\n",thisprog);
		fprintf(stderr,"Valid arguments - defaults in []:\n");
		fprintf(stderr,"	-cy column containing data (0=none, -1=all) [%d]\n",dvcol);
		fprintf(stderr,"	-v value to add to this column [%g]\n",setvald);
		fprintf(stderr,"	-long: assume input is long-integers (0=NO 1=YES) [%d]\n",setlong);
		fprintf(stderr,"Examples:\n");
		fprintf(stderr,"	cat datafile.txt | %s stdin -cy 1 -v 25\n",thisprog);
		fprintf(stderr,"	%s datafile.txt -cy 2 -v 1\n",thisprog);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	sprintf(infile,"%s",argv[1]);
	for(ii=2;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-long")==0) setlong=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-cy")==0)   dvcol=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-v")==0)    { setvald=atof(argv[ii+1]); setvall=atol(argv[ii+1]); ii++; }
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
	}}
	if(setlong!=0&&setlong!=1) {fprintf(stderr,"\n--- Error[%s]: invalid -long (%d): must be 0 or 1\n\n",thisprog,setlong);exit(1);}

	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}

	while(fgets(line,MAXLINELEN,fpin)!=NULL) {
		if(line[0]=='#') continue;
		pline=line;
		for(col=1;(pcol=strtok(pline," ,\t\n"))!=NULL;col++) {
			pline=NULL;
			if(col>1)printf("\t");
			if(col==dvcol||dvcol==-1) {
				if(setlong==0) {
					if(sscanf(pcol,"%lf",&aa)!=1 || !isfinite(aa)) {printf("%s",pcol);continue;}
					bb=aa+setvald;
					printf("%f",bb);
				}
				else {
					if(sscanf(pcol,"%ld",&ii)!=1) {printf("%s",pcol);continue;}
					jj=ii+setvall;
					printf("%ld",jj);
				}
			}
			else printf("%s",pcol);
		}
		printf("\n");
	}

	if(strcmp(infile,"stdin")!=0) fclose(fpin);
}
