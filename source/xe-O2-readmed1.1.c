#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define thisprog "xe-O2-readmed1"
#define TITLE_STRING thisprog" v 1: 5.February.2014 [JRH]"
#define MAXLINELEN 1000
#define MAXLABELS 1000

/*
<TAGS> O2 file </TAGS>

*/


/* external functions start */
int xf_stats2_d(double *data, long n, int varcalc, double *result_d);
long *xf_lineparse2(char *line,char *delimiters, long *nwords);
long *xf_lineparse1(char *line,long *nwords);
/* external functions end */

int main (int argc, char *argv[]) {
	/* general variables */
	char outfile[256],line[MAXLINELEN],templine[MAXLINELEN],*pline,*pcol;
	long int i,j,k,n;
	int v,w,x,y,z,col,col2,colmatch;
	int sizeofchar=sizeof(char),sizeofshort=sizeof(short),sizeoflong=sizeof(long),sizeofint=sizeof(int),sizeoffloat=sizeof(float),sizeofdouble=sizeof(double);
	float a,b,c,d;
	double aa,bb,cc,dd,result_d[64];
	FILE *fpin,*fpout;
	/* program-specific variables */
	char cexpt[256],cdate[32],cgroup[256];
	int *trialtype=NULL,*trial=NULL,*trialblock=NULL,subject=0,headerdone=0;
	long *start=NULL,*lindex=NULL,*eindex=NULL,nwords,nelements,nlabels,section,ntrialblocks,ntrials;
	double *data=NULL,*data2=NULL;
	/* arguments */
	char *infile, *setdatasection,*setlabels,*setelements;
	int setrow=0;

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<4) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Convert a Med-Associates file for amperometry data\n");
		fprintf(stderr,"- read subject/date/group/experiment information\n");
		fprintf(stderr,"- output a single data block of particular element numbers\n");
		fprintf(stderr,"- define labels for chosen elements\n");
		fprintf(stderr,"Assumes:\n");
		fprintf(stderr,"	- input is white-space delimited\n");
		fprintf(stderr,"	- multiple consecutive delimiters are treated as one\n");
		fprintf(stderr,"	- leading blank-spaces distinguish section labels from data\n");
		fprintf(stderr,"	- input has sections for trial data trial-type definition\n");
		fprintf(stderr,"	- data section has multiple fixed-length blocks of trials\n");
		fprintf(stderr,"\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [input] [section] [elements] [labels] [options]\n",thisprog);
		fprintf(stderr,"	[input]: Med-Associates input file (.txt) or \"stdin\"\n");
		fprintf(stderr,"	[section]: section containing data\n");
		fprintf(stderr,"	[elements]: comma-separated list of elements in [section] to output\n");
		fprintf(stderr,"	[labels]: comma-separated list of labels for [elements]\n");
		fprintf(stderr,"\n");
		fprintf(stderr,"VALID OPTIONS (defaults in []):\n");
		fprintf(stderr,"	-r : output on a single row (0=NO, 1=YES)[%d]\n",setrow);
		fprintf(stderr,"\n");
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s data.txt   A:  3,5  ntrials,nhead\n",thisprog);
		fprintf(stderr,"\n");
		fprintf(stderr,"OUTPUT: [experiment] [subject] [date] [group] [data]\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}


	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	infile= argv[1];
	setdatasection= argv[2];
	setelements= argv[3];
	setlabels= argv[4];

	for(i=2;i<argc;i++) {
		if( *(argv[i]+0) == '-') {
			if((i+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[i]); exit(1);}
			else if(strcmp(argv[i],"-r")==0) 	setrow=atoi(argv[++i]);
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[i]); exit(1);}
	}}

	eindex= xf_lineparse2(setelements,",",&nelements);
	lindex= xf_lineparse2(setlabels,",",&nlabels);

	if(nelements!=nlabels) {fprintf(stderr,"\n--- Error[%s]: number of elements (%d) does not match number of labels (%d)\n\n",thisprog,nelements,nlabels); exit(1);}

/* TEST: for(i=0;i<nelements;i++) printf("%s	%s\n",setelements+eindex[i],setlabels+lindex[i]); */

	/* CONVERT THE INPUT FILE  */
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
	n=ntrials=0;
	section=0;
	while(fgets(line,MAXLINELEN,fpin)!=NULL) {

		// parse the line  - assume CSV format
		start = xf_lineparse1(line,&nwords);

		// use contents in first column to define the file section
		if(strlen(line) > 0 ) {
			if(strcmp(line,setdatasection)==0) { section=1; continue; } // section containing the data for each trial
			else section = -1;
		}

		// read header (non-data) section
		if(section==-1 && headerdone==0) {
			if(strstr("Experiment:",line)!=NULL) {
				sprintf(cexpt,"%s",line+start[1]);
			}
			if(strstr("Subject:",line)!=NULL) {
				subject=atoi(line+start[1]);
			}
			if(strstr("Start Date:",line)!=NULL) {
				sprintf(cdate,"%s",line+start[2]);
			}
			if(strstr("Group:",line)!=NULL) {
				sprintf(cgroup,"%s",line+start[1]);
				headerdone=1;
			}
		}

		// read trial-data columns : skip the first column (0) which contains the index for the first item on each line
		if(section==1) {
			for(i=1;i<6;i++) {
				if((data=(double *)realloc(data,(n+1)*sizeofdouble))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
				if(sscanf((line+start[i]),"%lf",&aa)==1 && isfinite(aa)) data[n]=aa;
				else break;
				n++;
		}}
		// free memory for start in advance of next iteration
		free(start);
		start=NULL;
	}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);


	// PRINT THE OUTPUT
	if(setrow==1) {
		printf("expt	sub	date	grp");
		for(i=0;i<nelements;i++) {
			printf("	%s",setlabels+lindex[i],data[i]);
		}
		printf("\n");
		printf("%s	%03d	%s	%s",cexpt,subject,cdate,cgroup);
		for(i=0;i<nelements;i++) {
			x=atoi(setelements+eindex[i]);
			printf("	%g",data[i]);
		}
		printf("\n");
	}
	else if(setrow==0) {
		printf("experiment %s\nsubject %03d\ndate %s\ngroup %s\n",cexpt,subject,cdate,cgroup);
		printf("\n");
		for(i=0;i<nelements;i++) {
			x=atoi(setelements+eindex[i]);
			printf("%s %g\n",setlabels+lindex[i],data[i]);
		}
	}

exit(0);



	if(start!=NULL) free(start);
	if(trialtype!=NULL) free(trialtype);
	if(trial!=NULL) free(trial);
	if(eindex!=NULL) free(eindex);
	if(lindex!=NULL) free(lindex);

	exit(0);
}
