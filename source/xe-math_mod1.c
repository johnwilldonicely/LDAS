#define thisprog "xe-math_mod1"
#define TITLE_STRING thisprog" v 1: 36 September 2020 [JRH]"

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

/*
<TAGS>math </TAGS>
v 1: 36 September 2020
	- multi-purpose program to replace several math programs
		-[x] make long-line compatible
		-[x] update storage for infile (should be dynamic)
		-[x] make math mode selectable
		-[ ] add mult, div, abs , pow modes
		-[ ] make able to deal with header-lines
		-[ ] allow selection of a list of columns
		-[ ] allow selection of columns by name
 */

/* external functions start */
char *xf_lineread1(char *line, long *maxlinelen, FILE *fpin);
long *xf_lineparse2(char *line,char *delimiters, long *nwords);
/* external functions end */


int main(int argc, char *argv[]) {
	/******************************************************************************
	Initialise variables
	******************************************************************************/
	char *line=NULL,*pcol;
	int x,y,z,decimals;
	long ii,jj,kk,col,maxlinelen=0;
	long *iword=NULL,nwords;
	double aa,bb,cc,dd;
	FILE *fpin;
	/* arguments */
	char *infile=NULL,*setmode=NULL;
	short mode=0;
	int setlong=0;
	long setdvcol=1,setvall=0.0;
	double setvald=0.0;

	/******************************************************************************
	If only one argument (executable's name) print instructions
	******************************************************************************/
	if(argc<3) 	{
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Modify numbers in a table column\n");
		fprintf(stderr,"- No line-length limits\n");
		fprintf(stderr,"- Non-numeric fields will not be modified\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr," 	%s [in] [mode] [options]\n",thisprog);
		fprintf(stderr,"	[in]: file name or \"stdin\"\n");
		fprintf(stderr,"	[mode]: choices= add\n");
		fprintf(stderr,"VALID OPTIONS: defaults in []\n");
		fprintf(stderr,"	-cy column containing data (0=none, -1=all) [%ld]\n",setdvcol);
		fprintf(stderr,"	-v value to add to this column [%g]\n",setvald);
		fprintf(stderr,"	-long: assume input is long-integers (0=NO 1=YES) [%d]\n",setlong);
		fprintf(stderr,"Examples:\n");
		fprintf(stderr,"	%s datafile.txt add -cy 2 -v 100\n",thisprog);
		fprintf(stderr,"	cat datafile.txt | %s stdin add -cy 1 -v 25\n",thisprog);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/* READ THE FILENAME , MMODE, AND OPTIONAL ARGUMENTS */
	infile= argv[1];
	setmode= argv[2];
	for(ii=3;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-long")==0) setlong=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-cy")==0)   setdvcol=atol(argv[++ii]);
			else if(strcmp(argv[ii],"-v")==0)    {
				setvald=atof(argv[ii+1]);
				setvall=atol(argv[ii+1]);
				ii++;
			}
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
	}}
	if(setdvcol<1&&setdvcol!=-1) {fprintf(stderr,"\n--- Error[%s]: invalid -cy (%ld): must be -1 or >0\n\n",thisprog,setdvcol);exit(1);}
	if(setlong!=0&&setlong!=1) {fprintf(stderr,"\n--- Error[%s]: invalid -long (%d): must be 0 or 1\n\n",thisprog,setlong);exit(1);}

	if(strcmp(setmode,"add")==0) mode=1;
	else {fprintf(stderr,"\n--- Error[%s]: invalid mode (%s)\n\n",thisprog,setmode);exit(1);}

	setdvcol--;

	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
	while((line=xf_lineread1(line,&maxlinelen,fpin))!=NULL) {
		if(maxlinelen==-1)  {fprintf(stderr,"\n--- Error[%s]: readline function encountered insufficient memory\n\n",thisprog);exit(1);}
		iword= xf_lineparse2(line,"\t",&nwords);
		if(nwords<0) {fprintf(stderr,"\n--- Error[%s]: lineparse function encountered insufficient memory\n\n",thisprog);exit(1);};
		if(line[0]=='#') continue;

		for(col=0;col<nwords;col++) {
			pcol= (line+iword[col]);
			if(col>0)printf("\t");
			if(col==setdvcol||setdvcol==-1) {
				if(setlong==0) {
					if(sscanf(pcol,"%lf",&aa)!=1 || !isfinite(aa)) {printf("%s",pcol);continue;}
					if(mode==1) printf("%f",(aa+setvald));
				}
				else {
					if(sscanf(pcol,"%ld",&ii)!=1) {printf("%s",pcol);continue;}
					if(mode==1) printf("%ld",(ii+setvall));
				}
			}
			else printf("%s",pcol);
		}
		printf("\n");
	}

	if(strcmp(infile,"stdin")!=0) fclose(fpin);

	/********************************************************************************/
	/* CLEANUP AND EXIT */
	/********************************************************************************/
END:
	if(line!=NULL) free(line);
	if(iword!=NULL) free(iword);
	exit(0);




}
