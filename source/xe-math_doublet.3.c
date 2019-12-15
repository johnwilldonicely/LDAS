#define thisprog "xe-math_doublet"
#define TITLE_STRING thisprog" v 3: 20.January.2019 [JRH]"

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

/*
<TAGS>math </TAGS>

v 3: 20.January.2019 [JRH]
	- major overhaul, now allows outputting of other columns

v 2: 1.December.2014 [JRH]
	- now all lines inpurt have a corresponding output - nan is output as required
*/

/* external functions start */
char *xf_lineread1(char *line, long *maxlinelen, FILE *fpin);
long *xf_lineparse2(char *line,char *delimiters, long *nwords);
/* external functions end */

int main(int argc, char *argv[]) {
	/******************************************************************************
	Initialise variables
	******************************************************************************/
	char infile[256],message[256],*line=NULL,*templine=NULL;
	int x,y,z,col,decimals;
	long ii,jj,kk,nn,mm,sizeoftempline=sizeof(*templine),maxlinelen=0,prevlinelen=0,nwords=0,*iword=NULL;
	float a,b,c;
	double aa,bb,cc,dd,resultd[64];
	FILE *fpin;
	/* program-specific */
	long colx,coly,jj2;
	double bb2;
	/* arguments */
	int settype=1,setlong=0, setoutall=0;
	long setcolx=1,setcoly=2;

	/******************************************************************************
	If only one argument (executable's name) print instructions
	******************************************************************************/
	if(argc<2) 	{
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Modify column-y by column-x\n");
		fprintf(stderr,"Non-numeric or non-finite fields produce NAN output (line-count preserved)\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr," 	%s [input] [options]\n",thisprog);
		fprintf(stderr,"		[input]: file name or \"stdin\" in format <col1> <col2>\n");
		fprintf(stderr,"VALID OPTIONS: defaults in []:\n");
		fprintf(stderr,"	-cx: column containing the modifier [%ld]\n",setcolx);
		fprintf(stderr,"	-cy: column to be modified [%ld]\n",setcoly);
		fprintf(stderr,"	-long: assume input is long-integers (0=NO 1=YES) [%d]\n",setlong);
		fprintf(stderr,"	-t type of operation[%d]\n",settype);
		fprintf(stderr,"		1: y+x addition\n");
		fprintf(stderr,"		2: y-x subtraction\n");
		fprintf(stderr,"		3: y*x multiplication\n");
		fprintf(stderr,"		4: y/x division\n");
		fprintf(stderr,"	-out: output modified column (0) or all columns (1) [%d]\n",setoutall);
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	cat datafile.txt | %s stdin -t 2\n",thisprog);
		fprintf(stderr,"	%s datafile.txt -t 1\n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	single column result\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	sprintf(infile,"%s",argv[1]);
	for(ii=2;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-cx")==0)    setcolx= atol(argv[++ii]);
			else if(strcmp(argv[ii],"-cy")==0)    setcoly= atol(argv[++ii]);
			else if(strcmp(argv[ii],"-t")==0)     settype= atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-long")==0)  setlong= atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-out")==0)   setoutall= atoi(argv[++ii]);
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
	}}
	if(setcolx<1) {fprintf(stderr,"\n--- Error[%s]: invalid input column (-cx %ld) - must be >=1\n\n",thisprog,setcolx);exit(1);}
	if(setcoly<1) {fprintf(stderr,"\n--- Error[%s]: invalid input column (-cy %ld) - must be >=1\n\n",thisprog,setcoly);exit(1);}
	if(settype<1||settype>4)   {fprintf(stderr,"\n--- Error[%s]: invalid operation type (-t %d): must be 1-4\n\n",thisprog,settype);exit(1);}
	if(setlong!=0&&setlong!=1) {fprintf(stderr,"\n--- Error[%s]: invalid -long (%d): must be 0 or 1\n\n",thisprog,setlong);exit(1);}
	if(setoutall!=0&&setoutall!=1) {fprintf(stderr,"\n--- Error[%s]: invalid -out (%d): must be 0 or 1\n\n",thisprog,setoutall);exit(1);}

	/* SET COLUMNS TO ZERO-OFFSET REFERENCE */
	colx= setcolx-1;
	coly= setcoly-1;

	/* STAR TSCANNING TTHE INPUT  */
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}

	while((line=xf_lineread1(line,&maxlinelen,fpin))!=NULL) {
		if(maxlinelen==-1)  {fprintf(stderr,"\n--- Error[%s]: readline function encountered insufficient memory\n\n",thisprog);exit(1);}
		/* make a temporary copy of the line before parsing it */
		if(maxlinelen>prevlinelen) { prevlinelen= maxlinelen; templine= realloc(templine,(maxlinelen+1)*sizeoftempline); if(templine==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}}
		strcpy(templine,line);
		/* parse the line */
		iword= xf_lineparse2(line,"\t",&nwords);
		if(nwords<0) {fprintf(stderr,"\n--- Error[%s]: lineparse function encountered insufficient memory\n\n",thisprog);exit(1);};
		/* if missing coly, output nan */
		if(nwords<setcolx || nwords<setcoly) {
		//TEST: printf("nwords=%ld setcoly=%ld\n",nwords,setcoly);
			if(setoutall==0) printf("nan\n");
			else {for(ii=0;ii<nwords;ii++) {if(ii>0) printf("\t");if(ii==coly) printf("nan"); else printf("%s",(line+iword[ii]));} printf("\n");}
			continue;
		}

		/* OPTION-1: IF BOTH COLUMNS ARE FOUND, OUTPUT FOR FLOATING-POINT */
		mm= 2;
		if(setlong==0) {
			if(sscanf(line+iword[colx],"%lf",&aa)==1) mm--;
			if(sscanf(line+iword[coly],"%lf",&bb)==1) mm--;
			//TEST:	printf("%s %s mm=%ld\n",(line+iword[colx]),(line+iword[coly]),mm);
			if(mm!=0) {
				//TEST:	printf("header\n");
				if(setoutall==0) printf("%s\n",(line+iword[coly]));
				else printf("%s",templine);
				continue;
			}
			/* detemine converted output (bb2) */
			if(!isfinite(aa) || !isfinite(bb)) bb2=NAN;
			else if(settype==1) bb2= bb+aa;
			else if(settype==2) bb2= bb-aa;
			else if(settype==3) bb2= bb*aa;
			else if(settype==4) {
				if(aa==0.0) bb2= NAN;
				else if(bb==0.0) bb2= INFINITY;
				else bb2= bb/aa;
			}
			/* output */
			if(setoutall==0) printf("%lf\n",bb2);
			else {
				for(ii=0;ii<nwords;ii++) {
					if(ii>0) printf("\t");
					if(ii==coly) printf("%lf",bb2);
					else printf("%s",(line+iword[ii]));
				}
				printf("\n");
			}
		}
		/* OPTION-2: IF BOTH COLUMNS ARE FOUND, OUTPUT FOR LONG-INTEGERS */
		if(setlong==1) {
			if(sscanf(line+iword[colx],"%ld",&ii)==1) mm--;
			if(sscanf(line+iword[coly],"%ld",&jj)==1) mm--;
			if(mm!=0) {
				if(setoutall==0) printf("%s\n",(line+iword[coly]));
				else printf("%s",templine);
				continue;
			}
			/* detemine converted output (jj2) */
			else if(settype==1) jj2= jj+ii;
			else if(settype==2) jj2= jj-ii;
			else if(settype==3) jj2= jj*ii;
			else if(settype==4) {
				if(ii==0) jj2= LONG_MAX;
				else if(jj==0) jj2= 0;
				else jj2= jj/ii;
			}
			/* output */
			if(setoutall==0) {
				if(jj2!=LONG_MAX) printf("%ld\n",jj2);
				else printf("inf\n");
			}
			else {
				for(ii=0;ii<nwords;ii++) {
					if(ii>0) printf("\t");
					if(ii==coly) {
						if(jj2!=LONG_MAX) printf("%ld",jj2);
						else printf("inf");
					}
					else printf("%s",(line+iword[ii]));
				}
				printf("\n");
			}
		}

	} // end of while lineread
	if(strcmp(infile,"stdin")!=0) fclose(fpin);

	/* CLEANUP AND EXIT */
	if(iword!=NULL) free(iword);
	if(line!=NULL) free(line);
	if(templine!=NULL) free(templine);
	exit(0);


}
