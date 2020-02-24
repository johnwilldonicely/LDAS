#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define thisprog "xe-cofiring"
#define TITLE_STRING thisprog" v 1.2: 18.April.2016 [JRH]"
#define MAXLINELEN 1000

/*
<TAGS>signal_processing dt.spikes</TAGS>

v 1.2: 18.April.2016 [JRH]
	- update correlate function
	- remove some unnecessary variable definitions

v 1.2: 14.August.2012 [JRH]
	- bugfix - changed use of fscanf to read data with fgets/sscanf, to avoid problems related to "-" and "."
*/



/* external functions start */
int xf_correlate_i(int *x, int *y, long nn, int setinv, double *result, char *message);
float xf_prob_F(float F, int df1,int df2);
/* external functions end */

int main (int argc, char *argv[]) {
	/* general variables */
	char infile[256],outfile[256],line[MAXLINELEN],*pline,*pcol,message[256];
	long int i,j,k;
	int v,w,x,y,z,n,col,colmatch,sizeofint=sizeof(int),sizeofdouble=sizeof(double);
	float a,b,c,d;
	double aa,bb,cc,dd,resultd[64];
	FILE *fpin,*fpout;
	/* program-specific variables */
	char spikefile[256],listfile[256];
	int *spikeid=NULL,*listA=NULL,*listB=NULL,nspikes=0,nlist=0;
	int nwindows,cellidA,cellidB,*wincountA=NULL,*wincountB=NULL;
	double *spiketime=NULL,timestart,timeend,timerange,winstart,winend;
	/* arguments */
	int setmincount=0;
	double setwinsize=0.1;

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<3) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Calculate co-firing in a cell-pair using a cell-pair list-file\n");
		fprintf(stderr,"- usage:\n");
		fprintf(stderr,"	%s [data] [listfile] [arguments]\n",thisprog);
		fprintf(stderr,"	[data]: filename or \"stdin\" of format [time] [cell-id]\n");
		fprintf(stderr,"	[listfile]: file listing cell-id pairs (two numbers per line)\n");
		fprintf(stderr,"- valid arguments (defaults in []) ...\n");
		fprintf(stderr,"	-w size of window, seconds [%g]\n",setwinsize);
		fprintf(stderr,"	-ms minimum spikes (any cell) in a window to allow inclusion [%d]\n",setmincount);
		fprintf(stderr,"- examples:\n");
		fprintf(stderr,"	%s data.txt list.txt\n",thisprog);
		fprintf(stderr,"	cat data.txt | %s stdin list.txt\n",thisprog);
		fprintf(stderr,"- output:\n");
		fprintf(stderr,"	1st column: lower limit of each bin\n");
		fprintf(stderr,"	2nd column: value (eg. counts) in that bin\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	sprintf(spikefile,"%s",argv[1]);
	sprintf(listfile,"%s",argv[2]);
	for(i=3;i<argc;i++) {
		if( *(argv[i]+0) == '-') {
			if(i>=argc) break;
			else if(strcmp(argv[i],"-w")==0) 	{ setwinsize=atof(argv[i+1]); i++;}
			else if(strcmp(argv[i],"-ms")==0) 	{ setmincount=atoi(argv[i+1]); i++;}
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[i]); exit(1);}
	}}


	/* STORE SPIKE TIME AND CELLID DATA */
	if(strcmp(spikefile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(spikefile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,spikefile);exit(1);}
	nspikes=0;
	while(fgets(line,MAXLINELEN,fpin)!=NULL) {
		if(sscanf(line,"%lf %d",&aa,&x)!=2) continue;
		spiketime=(double *)realloc(spiketime,(nspikes+1)*sizeofdouble);
		spikeid=(int *)realloc(spikeid,(nspikes+1)*sizeofint);
		if(spiketime==NULL || spikeid==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
		spiketime[nspikes]=aa; spikeid[nspikes]=x; nspikes++;
	}
	if(strcmp(spikefile,"stdin")!=0) fclose(fpin);
	timestart=spiketime[0];
	timeend=spiketime[nspikes-1];
	timerange=timeend-timestart;


	/* STORE CELL-PAIR LIST */
	if((fpin=fopen(listfile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,listfile);exit(1);}
	// count lines in list
	nlist=0; while(fgets(line,MAXLINELEN,fpin)!=NULL) nlist++; rewind(fpin);
	// allocate memory
	listA = (int *) realloc(listA,(nlist+1)*sizeof(int));
	listB = (int *) realloc(listB,(nlist+1)*sizeof(int));
	if(listA==NULL || listB==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
	nlist=0;
	while(fgets(line,MAXLINELEN,fpin)!=NULL) {
		if(sscanf(line,"%d %d",&x,&y)==2) {
			listA[nlist]=x;
			listB[nlist]=y;
			nlist++;
		}
	}
	fclose(fpin);

	/* DEFINE COUNT-ARRAYS FOR TIME-WINDOWS */
	nwindows=(int)(timerange/setwinsize); // a little of end of file will not be counted, but all windows will be the same size
	wincountA = (int *) realloc(wincountA,(nwindows+1)*sizeof(int));
	wincountB = (int *) realloc(wincountB,(nwindows+1)*sizeof(int));
	if(wincountA==NULL || wincountB==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}

	// CALCULATE FIRING IN TIME WINDOWS FOR EACH CELL PAIR (i=CELLPAIR, j=SPIKENUMBER)
	for(i=0;i<nlist;i++) {
		cellidA=listA[i];cellidB=listB[i];						// speed up processing by defining cellids to be matched
		for(k=0;k<nwindows;k++) wincountA[k]=wincountB[k]=0;	// reset spike-counts for each window
		for(j=0;j<nspikes;j++) {								// now populate the windows
			z=(int)((spiketime[j]-timestart)/setwinsize); //  formula to determine the window-number
			if(z>nwindows) {
				printf("Error!!: window %d is greater than nwindows %d\n",z,nwindows);
				printf("%d	%g	%g	%g\n",spikeid[j],spiketime[j],timestart,timeend);
				exit(1);
			}
			if(spikeid[j]==cellidA) wincountA[z]++ ;
			if(spikeid[j]==cellidB) wincountB[z]++ ;
		}

		// if total spike count in a window < set minimum, set values to invalid
		for(k=0;k<nwindows;k++) if((wincountA[k]+wincountB[k])<setmincount) wincountA[k]=wincountB[k]=-1;

		/* correlate spike-counts in the windows - no need to check status - must be >4 windows */
		xf_correlate_i(wincountA, wincountB, nwindows, -1, resultd, message);

		for(k=0;k<nwindows;k++) printf("%ld\t%d\t%d\n",k,wincountA[k],wincountB[k]);

		printf("%d\t%d\t%f\n",cellidA,cellidB,resultd[1]);
	}

	free(spiketime); free(spikeid);
	free(listA); free(listB);
	free(wincountA);
	free(wincountB);

	exit(0);
	}
