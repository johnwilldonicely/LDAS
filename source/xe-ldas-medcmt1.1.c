#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define thisprog "xe-ldas-medcmt1"
#define TITLE_STRING thisprog" v 1: 24.September.2012 [JRH]"
#define MAXLINELEN 1000
#define MAXLABELS 1000

/* <TAGS> file MED </TAGS> */
/*
*/


/* external functions start */
int xf_stats2_d(double *data, long n, int varcalc, double *result_d);
/* external functions end */

int main (int argc, char *argv[]) {
	/* general variables */
	char infile[256],outfile[256],line[MAXLINELEN],templine[MAXLINELEN],word[256];
	char *pline1,*pline2,*pcol1,*pcol2,*saveptr1,*saveptr2;
	long int i,j,k,n;
	int v,w,x,y,z,col,colmatch;
	int sizeofchar=sizeof(char),sizeofshort=sizeof(short),sizeoflong=sizeof(long),sizeofint=sizeof(int),sizeoffloat=sizeof(float),sizeofdouble=sizeof(double);
	float a,b,c,d; 
	double aa,bb,cc,dd, result_d[64];
	FILE *fpin=NULL,*fpout=NULL;
	/* program-specific variables */ 
	char *words=NULL;
	int *iword=NULL;
	int lenwords=0,nwords=0,*count,grp,bin,bintot,setrange=0,colx=1,coly=2;
	double *xdat=NULL,*ydat=NULL;

	char yymmdd[16],subjname[64];
	int col2,getdate=0,getsubj=0,start=0;
	/* arguments */
	char setblock[256];
	float setmult=0.1;

	sprintf(outfile,"");
	sprintf(setblock,"K");

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Output event-comments from a Med-Associates file\n");
		fprintf(stderr,"Assumes the input file has subject headers and blocks of [time].[event]\n");
		fprintf(stderr,"- a colon separates labels from header values and data\n");
		fprintf(stderr,"- data blocks are separated by letter-IDs (A: B: C: etc.)\n");
		fprintf(stderr,"- each data line begins with the index-number of the first datum\n");
		fprintf(stderr,"- time is the part of the datum before the decimal\n");
		fprintf(stderr,"- event type (eg 100) comes after the decimal. 000 indicates no event\n");
		fprintf(stderr,"- sample input file...\n");
		fprintf(stderr,"\n");
		fprintf(stderr,"	Start Date: 04/26/12\n");
		fprintf(stderr,"	Subject: 9\n");
		fprintf(stderr,"	Experiment: CAR_002\n");
		fprintf(stderr,"	Group: Box\n");
		fprintf(stderr,"	Box: 1\n");
		fprintf(stderr,"	A:\n");
		fprintf(stderr,"	     0:      965.100       46.100      109.100      135.000\n");
		fprintf(stderr,"	     4:        0.000\n");
		fprintf(stderr,"	C:\n");
		fprintf(stderr,"	     0:       20.100       26.200       33.100       26.200\n");
		fprintf(stderr,"	     4:       33.100       33.200       25.100        0.000\n");
		fprintf(stderr,"	     8:        0.000        0.000        0.000        0.000\n");
		fprintf(stderr,"\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [input] [options]\n",thisprog);
		fprintf(stderr,"		[input]: Med-Associates input file or \"stdin\"\n");
		fprintf(stderr,"\n");
		fprintf(stderr,"VALID OPTIONS (defaults in []):\n");
		fprintf(stderr,"	-b(lock) to output (a single capital letter) [%s]\n",setblock);
		fprintf(stderr,"	-t(time) multiplier, as time may not be seconds [%g]\n",setmult);
		fprintf(stderr,"		e.g. if time is in seconds, set to 1\n");
		fprintf(stderr,"		e.g. if time is in tenths of seconds, set to 0.1\n");
		fprintf(stderr,"		e.g. if time is in tens of seconds, set to 10\n");
		fprintf(stderr,"\n");
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s data.txt -b A\n",thisprog);
		fprintf(stderr,"	cat temp.txt | %s stdin -b K -t 10\n",thisprog);
		fprintf(stderr,"\n");
		fprintf(stderr,"OUTPUT: multiple files called [sub]-[yymmdd].med2  (e.g. 009-001231.med2)\n");
		fprintf(stderr,"	1st column: time in seconds\n");
		fprintf(stderr,"	2nd column: event label (eg. 100, 200, etc)\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	sprintf(infile,"%s",argv[1]);
	for(i=2;i<argc;i++) {
		if( *(argv[i]+0) == '-') {
			if((i+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[i]); exit(1);}
			else if(strcmp(argv[i],"-b")==0) 	{ sprintf(setblock,"%s",argv[i+1]); i++;}
			else if(strcmp(argv[i],"-t")==0) 	{ setmult=atof(argv[i+1]); i++;}
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[i]); exit(1);}
	}}

	/* STORE DATA METHOD 3 - newline delimited input  from user-defined columns */
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
	n=0; 
	while(fgets(line,MAXLINELEN,fpin)!=NULL) {
		pline1=line; colmatch=2; // number of columns to match
		getdate=0;
		getsubj=0;
		for(col=1;(pcol1=strtok_r(pline1,":\n\r",&saveptr1))!=NULL;col++) {
			pline1=NULL; 
			
			if(col==1) {
				if(strcmp(pcol1,"Start Date")==0) getdate=1;
				if(strcmp(pcol1,"Subject")==0) getsubj=1;
				if(strlen(pcol1)<2) { if(strcmp(pcol1,setblock)==0) start=1; else start=0; }
			}
			
			if(col==2) {
				if(getdate==1) sprintf(yymmdd,"%c%c%c%c%c%c",pcol1[7],pcol1[8],pcol1[1],pcol1[2],pcol1[4],pcol1[5]);
				if(getsubj==1) {
					if(strlen(outfile)>0) fclose(fpout);
					sprintf(outfile,"%03d-%s.med2",atoi(pcol1),yymmdd); 
					if((fpout=fopen(outfile,"w"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" could not be opened for writing\n\n",thisprog,outfile);exit(1);}
					fprintf(fpout,"time\tflag\n");
				}
				
				if(start==1) {
					pline2=pcol1;
					z=0;
					for(col2=1;(pcol2=strtok_r(pline2," .\t\n\r",&saveptr2))!=NULL;col2++) {
						pline2=NULL;
						if(z==0) aa=setmult*atoi(pcol2);
						if(z==1) { k=atoi(pcol2); if(k>0) fprintf(fpout,"%.3f\t%s\n",aa,pcol2);}
						if(++z>1)z=0;
					}
				} // END OF IF(START==1)
			}
		}
		n++;
	}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);
	if(strlen(outfile)>0) fclose(fpout);
	
	exit(0);
	}

