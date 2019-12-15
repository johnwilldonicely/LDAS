#define thisprog "xe-math_div1"
#define TITLE_STRING thisprog" v 2: 27.November.2017 [JRH]"

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#define MAXLINELEN 10000

/*
<TAGS>math </TAGS>
v 2: 27.November.2017 [JRH]
	- switch to using %f for output to avoid precision loss with exponential notation
v 2: 26.July.2016 [JRH]
	- add long-integer-options
	- update variable names
	- increase memory for lines to 10000 characters
	- add warning - currently not safe for very wide matrices
	- set default divisor to "1" instead of "0"
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
	long ii,jj,kk,ll,mm,nn;
	float a,b,c;
	double aa,bb,cc,dd;
	FILE *fpin;
	/* arguments */
	int dvcol=1,setlong=0;
	double setvalue=1.0;

	/******************************************************************************
	If only one argument (executable's name) print instructions
	******************************************************************************/
	if(argc<2) 	{
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"- Divide a column of data by a number\n");
		fprintf(stderr,"- Non-numeric fields will not be modified\n");
		fprintf(stderr,"- NOTE: safe for input lines < %ld characters\n",MAXLINELEN);
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr," 	%s [infile] [options]\n",thisprog);
		fprintf(stderr,"VALID OPTIONS (defaults) in []:\n");
		fprintf(stderr,"	-cy column containing data (0=none, -1=all) [%d]\n",dvcol);
		fprintf(stderr,"	-v value to divide this column by [%g]\n",setvalue);
		fprintf(stderr,"	-long: assume input is long-integers (0=NO 1=YES) [%d]\n",setlong);
		fprintf(stderr,"EXAMPLES:\n",setvalue);
		fprintf(stderr,"	cat datafile.txt | %s stdin -cy 1 -v 3.14\n",thisprog);
		fprintf(stderr,"	%s datafile.txt -cy 2 -v 7\n",thisprog);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	sprintf(infile,"%s",argv[1]);
	for(ii=2;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-cy")==0)   dvcol=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-v")==0)    setvalue=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-long")==0) setlong=atoi(argv[++ii]);
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
	}}
	if(setlong!=0&&setlong!=1) {fprintf(stderr,"\n--- Error[%s]: invalid -long (%d): must be 0 or 1\n\n",thisprog,setlong);exit(1);}

	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}

	if(setlong==0) {
		while(fgets(line,MAXLINELEN,fpin)!=NULL) {
			if(line[0]=='#') continue;
			pline=line;
			for(col=1;(pcol=strtok(pline," ,\t\n"))!=NULL;col++) {
				pline=NULL;
				if(col>1)printf("\t",pcol);
				if(col==dvcol||dvcol==-1) {
					if(sscanf(pcol,"%lf",&aa)!=1 || !isfinite(aa)) {printf("%s",pcol);continue;}
					bb=aa/setvalue;
					printf("%f",bb);
				}
				else printf("%s",pcol);
			}
			printf("\n");
		}
	}
	if(setlong==1) {
		while(fgets(line,MAXLINELEN,fpin)!=NULL) {
			if(line[0]=='#') continue;
			pline=line;
			for(col=1;(pcol=strtok(pline," ,\t\n"))!=NULL;col++) {
				pline=NULL;
				if(col>1)printf("\t",pcol);
				if(col==dvcol||dvcol==-1) {
					if(sscanf(pcol,"%ld",&ii)!=1) {printf("%s",pcol);continue;}
					jj=ii/setvalue;
					printf("%ld",jj);
				}
				else printf("%s",pcol);
			}
			printf("\n");
		}
	}



	if(strcmp(infile,"stdin")!=0) fclose(fpin);
	exit(0);
}
