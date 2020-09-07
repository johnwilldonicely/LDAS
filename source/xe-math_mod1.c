#define thisprog "xe-math_mod1"
#define TITLE_STRING thisprog" v 1: 6 September 2020 [JRH]"

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

/*
<TAGS>math </TAGS>
v 1: 6 September 2020
	- multi-purpose program to replace several math programs
		-[x] make long-line compatible
		-[x] update storage for infile (should be dynamic)
		-[x] make math mode selectable
		-[x] add,sub,mult,div,pow & abs modes
		-[ ] make able to deal with header-lines
		-[ ] allow selection of a list of columns
		-[ ] allow selection of columns by name




 */

/* external functions start */
char *xf_lineread1(char *line, long *maxlinelen, FILE *fpin);
long *xf_lineparse2(char *line,char *delimiters, long *nwords);
long *xf_getkeycol(char *line1, char *d1, char *keys1, char *d2, long *nkeys1, char *message);
/* external functions end */


int main(int argc, char *argv[]) {
	/******************************************************************************
	Initialise variables
	******************************************************************************/
	char *line=NULL,*pcol;
	short mode=0;
	int x,y,z,decimals;
	long ii,jj,kk,nn,col,maxlinelen=0;
	long *iword=NULL,nwords;
	long *ikeys=NULL,*keycols=NULL,nkeys;
	double aa,bb,cc,dd;
	FILE *fpin;
	/* arguments */
	char *infile=NULL,*setmode=NULL;
	int setlong=0;
	long setc=1,setvl=0;
	double setvd=0.0;

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
		fprintf(stderr,"	[mode]: add,sub,mul,div,pow or abs\n");
		fprintf(stderr,"VALID OPTIONS: defaults in []\n");
		fprintf(stderr,"	-c column containing data (0=none, -1=all) [%ld]\n",setc);
		fprintf(stderr,"	-v value to apply using [mode] [%g]\n",setvd);
		fprintf(stderr,"	-long: assume input is long-integers (0=NO 1=YES) [%d]\n",setlong);
		fprintf(stderr,"Examples:\n");
		fprintf(stderr,"	%s datafile.txt add -c 2 -v 100\n",thisprog);
		fprintf(stderr,"	cat datafile.txt | %s stdin add -c 1 -v 25\n",thisprog);
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
			else if(strcmp(argv[ii],"-c")==0)    setc= atol(argv[++ii]);
			else if(strcmp(argv[ii],"-v")==0)    {
				setvd=atof(argv[ii+1]);
				setvl=atol(argv[ii+1]);
				ii++;
			}
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
	}}
	if(setc<1&&setc!=-1) {fprintf(stderr,"\n--- Error[%s]: invalid -cy (%ld): must be -1 or >0\n\n",thisprog,setc);exit(1);}
	if(setlong!=0&&setlong!=1) {fprintf(stderr,"\n--- Error[%s]: invalid -long (%d): must be 0 or 1\n\n",thisprog,setlong);exit(1);}

	if(strcmp(setmode,"add")==0) mode=1;
	else if(strcmp(setmode,"sub")==0) mode=2;
	else if(strcmp(setmode,"mul")==0) mode=3;
	else if(strcmp(setmode,"div")==0) mode=4;
	else if(strcmp(setmode,"pow")==0) mode=5;
	else if(strcmp(setmode,"abs")==0) mode=6;
	else {fprintf(stderr,"\n--- Error[%s]: invalid mode (%s)\n\n",thisprog,setmode);exit(1);}

	setc--;
	nn= 0;

	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
	while((line=xf_lineread1(line,&maxlinelen,fpin))!=NULL) {
		if(maxlinelen==-1)  {fprintf(stderr,"\n--- Error[%s]: readline function encountered insufficient memory\n\n",thisprog);exit(1);}

		if(line[0]=='#') continue;
		nn++;

		iword= xf_lineparse2(line,"\t",&nwords);
		if(nwords<0) {fprintf(stderr,"\n--- Error[%s]: lineparse function encountered insufficient memory\n\n",thisprog);exit(1);};

		for(col=0;col<nwords;col++) {
			pcol= (line+iword[col]);
			if(col>0)printf("\t");
			if(col==setc||setc==-1) {
				if(setlong==0) {
					if(sscanf(pcol,"%lf",&aa)!=1 || !isfinite(aa)) {printf("%s",pcol);continue;}
					if(mode==1)      printf("%f",(aa+setvd));
					else if(mode==2) printf("%f",(aa-setvd));
					else if(mode==3) printf("%f",(aa*setvd));
					else if(mode==4) printf("%f",(aa/setvd));
					else if(mode==5) printf("%f",pow(aa,setvd));
					else if(mode==6) { if(aa>=0.0) printf("%f",aa); else printf("%f",(0.0-aa)); }
				}
				else {
					if(sscanf(pcol,"%ld",&ii)!=1) {printf("%s",pcol);continue;}
					if(mode==1) printf("%ld",(ii+setvl));
					else if(mode==2) printf("%ld",(ii-setvl));
					else if(mode==3) printf("%ld",(ii*setvl));
					else if(mode==4) printf("%ld",(ii/setvl));
					else if(mode==5) printf("%ld",(long)(pow((double)ii,(double)setvl)));
					else if(mode==6) { if(ii>=0) printf("%ld",ii); else printf("%ld",(0-ii)); }
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
