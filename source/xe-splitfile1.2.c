#define thisprog "xe-splitfile1"
#define TITLE_STRING thisprog" v 2: 28.November.2018 [JRH]"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAXLINELEN 1000
#define MAXCOLS 16

/*
<TAGS>file screen</TAGS>

v 2: 28.November.2018 [JRH]
	- add elapsed-time criterion

v 2: 3.February.2015 [JRH]
	- add option to split files when time "zero" is encountered (-c 0)
	- default is still to split when timestamp gets smaller instead of larger (-c 1)
*/

/* external functions start */
/* external functions end */


int main (int argc, char *argv[]) {

	/* general and file type variables */
	char line[MAXLINELEN],padline[MAXLINELEN],*pline,*pline2,*pcol,*pcol2,*tempcol,templine[256];
	long i,j,k,nn;
	int w,x,y,z,col,colmatch;
	int sizeofdouble=sizeof(double);
	float a,b,c;
	double aa,bb,cc;
	FILE *fpin,*fpout;
	/* program-specific variables */
	char infile[256],listfile[256],tempfile[256],outfile[256];
	int nout=0,prevgood=0,prevwindow=-1,nblocks=0;
	double time=0.0,interval,*tstart=NULL,*tend=NULL,prev;
	/* command line variables */
	int dcol1=1,dcol2=-1,tcol1=1,tcol2=2;
	double setfreq=-1.0,setzero=0.0,setcrit=-1.0;


	sprintf(tempfile,"temp_%s",thisprog);

	/* PRINT INSTRUCTIONS IF ONLY ONE ARGUMENT */
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"--------------------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"--------------------------------------------------------------------------------\n");
		fprintf(stderr,"Split a file whenever zero is reached or if there is a time discontinuity\n");
		fprintf(stderr,"USAGE: \n");
		fprintf(stderr,"	%s [infile][options]\n",thisprog);
		fprintf(stderr,"		[infile]: file or stdin with a time column and data column(s)\n");
		fprintf(stderr,"\n");
		fprintf(stderr,"VALID OPTIONS (defaults in []):\n");
		fprintf(stderr,"	-t: time column in infile [%d]\n",tcol1);
		fprintf(stderr,"	-c: criterion for split [%g]\n",setcrit);
		fprintf(stderr,"		-1: whenever the timestamp is lower than the previous one\n");
		fprintf(stderr,"		 0: whenever zero is encountered\n");
		fprintf(stderr,"		>0: after this many seconds has elapsed\n");
		fprintf(stderr,"EXAMPLE:\n");
		fprintf(stderr,"	%s a.txt times.txt -t 3 \n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	- a separate file for each block named %s_[block].txt\n",tempfile);
		fprintf(stderr,"\n");
		exit(0);
	}

	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	sprintf(infile,"%s",argv[1]);
	for(i=2;i<argc;i++) {
		if( *(argv[i]+0) == '-') {
			if((i+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[i]); exit(1);}
			else if(strcmp(argv[i],"-t")==0) {tcol1=atoi(argv[i+1]);i++;}
			else if(strcmp(argv[i],"-c")==0) {setcrit=atof(argv[i+1]);i++;}
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[i]); exit(1);}
	}}

	if(setcrit<-1)  {fprintf(stderr,"\n--- Error[%s]: invalid split criterion (-c %f) - must be -1, 0 or >0\n\n",thisprog,setcrit); exit(1);}


	nn= nblocks= 0;
	sprintf(outfile,"%s_%03d.txt",tempfile,nblocks);
	if ((fpout=fopen(outfile,"w"))==NULL) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" could not be opened for writing\n\n",thisprog,outfile);exit(1);}
	printf("\n\tOutput: %s\n",outfile);

	/* READ DATA FILE AND OUTPUT  LINES*/
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
	prev= 0.0; // this is purely set to avoid an uninitialized variable warning

	if(setcrit==-1.0) {
		while(fgets(line,MAXLINELEN,fpin)!=NULL) {
			strcpy(templine,line);
			z=0;
			pline=line;
			for(col=1;(pcol=strtok(pline," ,\t\n"))!=NULL;col++) {
				pline=NULL;
				if(col==tcol1 && sscanf(pcol,"%lf",&aa)==1) z=1;
			}
			if(z==1) {
				if(nn==0) prev= aa;
				if(aa<prev) {
					nblocks++;
					sprintf(outfile,"%s_%03d.txt",tempfile,nblocks);
					if ((fpout=fopen(outfile,"w"))==NULL) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" could not be opened for writing\n\n",thisprog,outfile);exit(1);}
					printf("\tOutput: %s\n",outfile);
				}
				prev= aa;
				nn++;
			}
			fprintf(fpout,"%s",templine);
		}
	}

	if(setcrit==0.0) {
		while(fgets(line,MAXLINELEN,fpin)!=NULL) {
			strcpy(templine,line);
			z=0;
			pline=line;
			for(col=1;(pcol=strtok(pline," ,\t\n"))!=NULL;col++) {
				pline=NULL;
				if(col==tcol1 && sscanf(pcol,"%lf",&aa)==1) z=1;
			}
			if(z==1) {
				if(aa==0.0) {
					nblocks++;
					sprintf(outfile,"%s_%03d.txt",tempfile,nblocks);
					if ((fpout=fopen(outfile,"w"))==NULL) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" could not be opened for writing\n\n",thisprog,outfile);exit(1);}
					printf("\tOutput: %s\n",outfile);
				}
				nn++;
			}
			fprintf(fpout,"%s",templine);
		}
	}

	if(setcrit>0.0) {
		while(fgets(line,MAXLINELEN,fpin)!=NULL) {
			strcpy(templine,line);
			z=0;
			pline=line;
			for(col=1;(pcol=strtok(pline," ,\t\n"))!=NULL;col++) {
				pline=NULL;
				if(col==tcol1 && sscanf(pcol,"%lf",&aa)==1) z=1;
			}
			if(z==1) {
				if(nn==0) prev= aa;
				if(aa<prev) prev= aa;
				if((aa-prev)>=setcrit) {
					nblocks++;
					sprintf(outfile,"%s_%03d.txt",tempfile,nblocks);
					if ((fpout=fopen(outfile,"w"))==NULL) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" could not be opened for writing\n\n",thisprog,outfile);exit(1);}
					printf("\tOutput: %s\n",outfile);
					prev= aa;
				}
				nn++;
			}
			fprintf(fpout,"%s",templine);
		}
	}



	if(strcmp(infile,"stdin")!=0) fclose(fpin);
	printf("\n");
	exit(0);
	}
