#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define thisprog "xe-ldas-invalidcmt1"
#define TITLE_STRING thisprog" v 2: 14.August.2012 [JRH]"
#define MAXLINELEN 1000
#define MAXWORDS 64

/* <TAGS> ldas screen</TAGS> */

/*
v 2: 14.August.2012 [JRH]
	- bugfix - changed use of fscanf to read data with fgets/sscanf, to avoid problems related to "-" and "."
*/




/* external functions start */
int xf_stats2_d(double *data, long n, int varcalc, double *result_d);
/* external functions end */

int main (int argc, char *argv[]) {
	/* general variables */
	char infile[256],outfile[256],line[MAXLINELEN],templine[MAXLINELEN],word[MAXLINELEN],*matchstring=NULL,*pline,*pcol;
	long int i,j,k,n;
	int v,w,x,y,z,col,colmatch;
	int sizeofchar=sizeof(char),sizeofshort=sizeof(short),sizeoflong=sizeof(long),sizeofint=sizeof(int),sizeoffloat=sizeof(float),sizeofdouble=sizeof(double);
	float a,b,c,d;
	double aa,bb,cc,dd;
	FILE *fpin,*fpout;
	/* program-specific variables */
	char comment1[MAXLINELEN],comment2[MAXLINELEN];
	double time1,timediff;
	int nbuffer=0, nmatches=0;
	int match1=0,match2=0;
	/* arguments */
	char setc1[MAXLINELEN], setc2[MAXLINELEN];
	char setmatchword[MAXLINELEN]="exact\0";
	int setinv=3,setmatch=1;
	double setmin=0,setmax=100;

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<5) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"A tool to select trials with a particular response-time from a series\n");
		fprintf(stderr,"Invalidates pairs of lines in a comment (.cmt) file based on interval\n");
		fprintf(stderr,"That is, if two comments in a row occur at the wrong interval, replace\n");
		fprintf(stderr,"  one or both of them with the word \"INVALID\"\n");
		fprintf(stderr,"Only comment-pairs occurring in the specified sequence will be checked\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [input] [mode] [c1] [c2] [options]\n",thisprog);
		fprintf(stderr,"	[input]: a filefile or \"stdin\" in [time][TAB][comment] format\n");
		fprintf(stderr,"	[mode]:  match mode, \"contains\" or \"exact\"\n");
		fprintf(stderr,"	[c1]: first comment in pair to match\n");
		fprintf(stderr,"	[c2]: second comment in pair to match\n");
		fprintf(stderr,"  [options] defaults in []:\n");
		fprintf(stderr,"	  -min: minimum interval between c1 and c2 (%g)\n",setmin);
		fprintf(stderr,"	  -max: maximum interval between c1 and c2 (%g)\n",setmax);
		fprintf(stderr,"	  -inv: invalidate mode: 0=neither, 1=c1, 2=c2, 3=both [%d]\n",setinv);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	The original input, with some comments replaced with the word \"INVALID\"\n");
		fprintf(stderr,"NOTE:\n");
		fprintf(stderr,"	Start and end words cannnot be the same\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/* READ THE INPUT .CMT FILENAME  */
	sprintf(infile,"%s",argv[1]);

	/* READ THE MATCH MODE */
	sprintf(setmatchword,"%s",argv[2]);
	if(strcmp(setmatchword,"exact")==0) setmatch=1;
	else if(strcmp(setmatchword,"contains")==0) setmatch=2;
	else {fprintf(stderr,"\n--- Error[%s]: \"%s\" is not a valid option for [match]. Must be \"exact\" or \"contains\"\n\n",thisprog,setmatchword);exit(1);}

	/* READ THE FILENAME AND TWO COMMENTS IN THE SEQUENCE TO MATCH */
	sprintf(setc1,"%s",argv[3]);
	sprintf(setc2,"%s",argv[4]);

	/* READ THE OPTIONAL ARGUMENTS */
	for(i=5;i<argc;i++) {
		if( *(argv[i]+0) == '-') {
			if((i+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[i]); exit(1);}
			else if(strcmp(argv[i],"-min")==0) 	{ setmin=atof(argv[i+1]); i++;}
			else if(strcmp(argv[i],"-max")==0) 	{ setmax=atof(argv[i+1]); i++;}
			else if(strcmp(argv[i],"-inv")==0) 	{ setinv=atoi(argv[i+1]); i++;}
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[i]); exit(1);}
	}}

	/* CHECK VALIDITY OF ARGUMENTS */
	if( strcmp(setc1,setc2)==0 ) {fprintf(stderr,"\n--- Error[%s]: comments c1 (%s) and c2 (%s) are identical\n",thisprog,setc1,setc2); exit(1);}
	if( setmatch==2 && strstr(setc1,setc2)!=NULL ) {fprintf(stderr,"\n--- Error[%s]: don't use \"contains\" mode if c1 (%s) is a subset of c2 (%s) or vice versa\n",thisprog,setc1,setc2); exit(1);}
	if( setmatch==2 && strstr(setc2,setc1)!=NULL ) {fprintf(stderr,"\n--- Error[%s]: don't use \"contains\" mode if c1 (%s) is a subset of c2 (%s) or vice versa\n",thisprog,setc1,setc2); exit(1);}
	if( setmin >= setmax ) {fprintf(stderr,"\n--- Error[%s]: -min (%g) is >= max (%g)\n",thisprog,setmin,setmax); exit(1);}
	if( setinv<0 || setinv>3) {fprintf(stderr,"\n--- Error[%s]: -inv (%d) must be 0,1,2 or 3\n",thisprog,setinv); exit(1);}

	/* READ THE DATA */
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
	n= nbuffer= 0;
	time1= 0.0;
	while(fgets(line,MAXLINELEN,fpin)!=NULL) {

		/* if line contains non-numerical values then read the whole thing as a character array and go to next line */
		if(sscanf(line,"%lf %s",&aa,&templine)!=2) {nbuffer=0;continue;}
   		 // don't keep NaN or Inf time entries
		//if(!isfinite(aa)) {nbuffer=0; continue; }

		/* determine if current word matches set comments - different methods for "exact" or "contains" */
		if(setmatch==1) { /* start by assuming a non-match - match if strcmp result is zero */
			match1=0; if(strcmp(setc1,templine)==0) match1=1;
			match2=0; if(strcmp(setc2,templine)==0) match2=1;
		}
		if(setmatch==2) { /* start by assuming a match - nonmatch if strstr result is NULL */
			match1=1; if(strstr(templine,setc1)==NULL) match1=0;
			match2=1; if(strstr(templine,setc2)==NULL) match2=0;
		}
		//printf("\t%g\t%s\t\tmatch1:%d match2:%d\tnbuffer:%d\n",aa,templine,match1,match2,nbuffer);

		/* [1] if the comment matches setc1, save the current line and continue without printing */
		if(match1==1) {
			time1=aa;
			sprintf(comment1,"%s",templine);
			nbuffer=1;
		}
		/* [2] if the comment matches setc2... */
		else if(match2==1) {
			/* .. and there was a previous match of setc1, output must be produced and buffer reset... */
			if(nbuffer==1) {
				nbuffer=0; timediff=aa-time1;
				nmatches++;
				/* if the interval is within bounds, then print the pair */
				if(timediff>=setmin && timediff<=setmax) { printf("%g\t%s\n%g\t%s\n",time1,comment1,aa,templine); }
				/* otherwise invalidate one or both of the most recent comments */
				else {
					if(setinv==1){ printf("%g\t%s\n%g\t%s\n",time1,"INVALID",aa,templine); }
					if(setinv==2){ printf("%g\t%s\n%g\t%s\n",time1,comment1,aa,"INVALID"); }
					if(setinv==3){ printf("%g\t%s\n%g\t%s\n",time1,"INVALID",aa,"INVALID"); }
				}
			}
			/* if a match of setc2 with no preceeding match of setc1 (nbuffer=0), the previous line was printed so print only the current line, as is */
			else printf("%g\t%s\n",aa,templine);
		}


		/* [3] if neither setc1 nor setc2 were matched... */
		else {
			if(nbuffer==1) printf("%g\t%s\n",time1,comment1);
			printf("%g\t%s\n",aa,templine);
			nbuffer=0;
		}


	}
  if(strcmp(infile,"stdin")!=0) fclose(fpin);

  /* if file ended with a line stored but not printed, print the stored line */
  if(nbuffer==1) printf("%g\t%s\n",time1,comment1);

  if(nmatches==0) fprintf(stderr,"--- Warning [%s]: no matches found for %s followed by %s\n",thisprog,setc1,setc2);


	exit(0);
}
