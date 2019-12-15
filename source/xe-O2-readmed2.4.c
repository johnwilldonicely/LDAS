#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define thisprog "xe-O2-readmed2"
#define TITLE_STRING thisprog" v 4: 12.June.2018 [JRH]"
#define MAXLINELEN 1000
#define MAXLABELS 1000

/*
<TAGS> O2 file </TAGS>

v 4: 12.June.2018 [JRH]
	- bugfix - should not assume 5 data points per line when scanning - use nwords instead

v 4: 5.February.2014 [JRH]
	- allow user to control which section contains data and definition of trial types

v 2: 14.May.2013 [JRH]
	- this version directly reads the MED-ASSOCIATES output file (txt, space-delimited)
*/


/* external functions start */
int xf_stats2_d(double *data, long n, int varcalc, double *result_d);
long *xf_lineparse2(char *line,char *delimiters, long *nwords);
long *xf_lineparse1(char *line,long *nwords);
/* external functions end */

int main (int argc, char *argv[]) {
	/* general variables */
	char infile[256],outfile[256],line[MAXLINELEN],templine[MAXLINELEN],*pline,*pcol;
	long ii,jj,kk,nn;
	int v,w,x,y,z,col,col2,colmatch;
	int sizeofchar=sizeof(char),sizeofshort=sizeof(short),sizeoflong=sizeof(long),sizeofint=sizeof(int),sizeoffloat=sizeof(float),sizeofdouble=sizeof(double);
	float a,b,c,d;
	double aa,bb,cc,dd,result_d[64];
	FILE *fpin,*fpout;
	/* program-specific variables */
	int *trialtype=NULL,*trial=NULL,*trialblock=NULL;
	long *start=NULL,nwords,section=-1,ntrialblocks,ntrials;
	double *data=NULL,*data2=NULL;
	/* arguments */
	char setdatasection[64],setdefsection[64];
	int setnhead=11,setrepeated=0;

	sprintf(setdatasection,"C:");
	sprintf(setdefsection,"Z:");

	/********************************************************************************/
	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	/********************************************************************************/
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Convert a Med-Associates file for amperometry data\n");
		fprintf(stderr,"Assumes:\n");
		fprintf(stderr,"	- input is white-space delimited file\n");
		fprintf(stderr,"	- multiple consecutive delimiters are treated as one\n");
		fprintf(stderr,"	- leading blank-spaces distinguish section labels from data\n");
		fprintf(stderr,"	- input has sections for trial data trial-type definition\n");
		fprintf(stderr,"	- data section has multiple fixed-length blocks of trials\n");
		fprintf(stderr,"	- trials-per-block is defined in the trial-definition section\n");
		fprintf(stderr,"		NOTE: trial definition section known to have one extra entry\n");
		fprintf(stderr,"		NOTE: this extra should always be trial-type zero\n");
		fprintf(stderr,"\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [input] [options]\n",thisprog);
		fprintf(stderr,"	[input]: Med-Associates input file (.txt) or \"stdin\"\n");
		fprintf(stderr,"\n");
		fprintf(stderr,"VALID OPTIONS (defaults in []):\n");
		fprintf(stderr,"	-h : number of header entries in data-section to ignore [%d]\n",setnhead);
		fprintf(stderr,"	-s1 : data section [%s]\n",setdatasection);
		fprintf(stderr,"	-s2 : trial-definition section [%s]\n",setdefsection);
		fprintf(stderr,"	-r : treat data-blocks as repeated measures (output in separate columns [%d]\n",setrepeated);
		fprintf(stderr,"\n");
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s data.txt -s1 \"C:\" -s2 \"Z:\" \n",thisprog);
		fprintf(stderr,"	cat temp.txt | %s stdin -s1 \"C:\" -s2 \"Z:\"\n",thisprog);
		fprintf(stderr,"\n");
		fprintf(stderr,"OUTPUT: [block]	[trial] [trialtype] [data]\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/********************************************************************************/
	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	/********************************************************************************/
	sprintf(infile,"%s",argv[1]);
	for(ii=2;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-h")==0) 	setnhead=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-s1")==0) 	sprintf(setdatasection,"%s",(argv[++ii]));
			else if(strcmp(argv[ii],"-s2")==0) 	sprintf(setdefsection,"%s",(argv[++ii]));
			else if(strcmp(argv[ii],"-r")==0) 	setrepeated=atoi(argv[++ii]);
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
	}}

	/********************************************************************************/
	/* CONVERT THE INPUT FILE  */
	/********************************************************************************/
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
	nn=ntrials=0;
	while(fgets(line,MAXLINELEN,fpin)!=NULL) {

		// parse the line  - assume CSV format
		start = xf_lineparse1(line,&nwords);

		// use contents in first column to define the file section
		if(strlen(line) > 0 ) {
			section = -1;
			if(strcmp(line,setdatasection)==0) { section=1; continue; } // section containing the data for each trial
			if(strcmp(line,setdefsection)==0) { section=2; continue; } // section containing the trial descriptor record
		}

		// read trial-data columns - skip the first word (which just labels the first datum-number, e.g. "10:")
		if(section==1) {
			for(ii=1;ii<nwords;ii++) {
				if((data=(double *)realloc(data,(nn+1)*sizeofdouble))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
				if(sscanf((line+start[ii]),"%lf",&aa)==1 && isfinite(aa)) data[nn]=aa;
				else break;
				nn++;
			}
		}

		// read trial-descriptor
		if(section==2) {
			for(ii=1;ii<nwords;ii++) {
				if((trialtype=(int *)realloc(trialtype,(ntrials+1)*sizeofint))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
				if(sscanf((line+start[ii]),"%d",&x)==1) trialtype[ntrials]=x;
				else break;
				ntrials++;
			}
		}
		// free memory for start in advance of next iteration
		free(start);
		start=NULL;
	}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);

	/********************************************************************************/
	/* PRINT THE OUTPUT */
	/********************************************************************************/
	data += setnhead;
	nn = nn - setnhead;
	ntrials--;
	ntrialblocks=0;
	if(setrepeated==0) {
		printf("block	trial	type	data\n");
		for(ii=jj=0;ii<nn;ii++,jj++) {
			if(jj>=ntrials) { jj=0; ntrialblocks++; }
			printf("%ld	%ld	%d	%g\n",(ntrialblocks+1),(jj+1),trialtype[jj],data[ii]);
		}
	}
	if(setrepeated==1) {
		if((trial=(int *)realloc(trial,(nn+1)*sizeofint))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
		if((trialblock=(int *)realloc(trialblock,(nn+1)*sizeofint))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
		for(ii=jj=0;ii<nn;ii++,jj++) {
			if(jj>=ntrials) { jj=0; ntrialblocks++; }
			trial[ii]=jj;
			trialblock[ii]=ntrialblocks;
		}
		ntrialblocks++;
		printf("trial	type"); for(ii=0;ii<ntrialblocks;ii++) printf("\tblock%ld",(ii+1)); printf("\n");
		for(ii=0;ii<ntrials;ii++) {
			printf("%ld	%d",(ii+1),trialtype[ii]);
			for(jj=0;jj<ntrialblocks;jj++) {
				for(kk=0;kk<nn;kk++) {
					if(trial[kk]==ii && trialblock[kk]==jj) printf("\t%g",data[kk]);
			}}
			printf("\n");
	}}



	if(start!=NULL) free(start);
	if(trialtype!=NULL) free(trialtype);
	if(trial!=NULL) free(trial);
	if(trialblock!=NULL) free(trialblock);
	if(data!=NULL) { data -= setnhead; free(data);}
	exit(0);
}
