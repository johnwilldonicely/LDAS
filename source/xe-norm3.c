#define thisprog "xe-norm3"
#define TITLE_STRING thisprog" v 1: 19.April.2019 [JRH]"
#define MAXLINELEN 1000

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
<TAGS> stats </TAGS>

v 1: 5.February.2021 [JRH]
	- make id1 and id2 double-precision float, for more flexibility in naming of "groups"

v 1: 19.April.2019 [JRH]
	- first version
*/

/* external functions start */
char *xf_lineread1(char *line, long *maxlinelen, FILE *fpin);
long *xf_lineparse2(char *line,char *delimiters, long *nwords);
long xf_norm3_d(double *data,long ndata,int normtype,long start,long stop,char *message);
/* external functions end */

void internal_print4(char *line,long *iword,long setid1,long setid2,long setrep,long setval) {
	printf("%s\t%s\t%s\t%s\n", (line+iword[setid1]),(line+iword[setid2]),(line+iword[setrep]),(line+iword[setval]));
	return;
}

int main (int argc, char *argv[]) {

	/* general variables */
	char outfile[256],*line=NULL,*templine=NULL,word[256],*pline,*pcol,message[MAXLINELEN];
	long int ii,jj,kk,ll,mm,nn,nbad,nchars=0,maxlinelen=0,prevlinelen=0;
	int v,w,x,y,z,col,colmatch;
	float a,b,c,d,result_f[64];
	double aa,bb,cc,dd,ee,result_d[64];
	FILE *fpin,*fpout;
	/* program-specific variables */
	char *words=NULL;
	int sizeofid1,sizeofid2,sizeofrep,sizeofval,startfound,stopfound;
	long *iword=NULL,nwords;
	long nlines,start,stop,indexa,indexb;
	double *datid1=NULL,*datid2=NULL,*datrep=NULL,*datval=NULL,previd1,previd2;

	/* arguments */
	char *infile=NULL;
	int setnorm=3,setverb=0;
	long setid1=1,setid2=2,setrep=3,setval=4,sethead=0;
	double setn1=0.0,setn2=1.0;


	/********************************************************************************
	PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED
	********************************************************************************/
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Normalise repeated-measures data, preserving grouping identifiers\n");
		fprintf(stderr,"- typically used to normalize subject x group x time data\n");
		fprintf(stderr,"- works on 4 input columns:\n");
		fprintf(stderr,"- 	id1: identifier (float, typically subject)\n");
		fprintf(stderr,"- 	id2: identifier (float, typically group)\n");
		fprintf(stderr,"- 	rep: repeated-measure (float, typically time)\n");
		fprintf(stderr,"- 	val: value to be normalized (float)\n");
		fprintf(stderr,"NOTE! assumes data are sorted by id1,id2 and rep (see below)\n");
		fprintf(stderr,"USAGE: %s [in] [options]\n",thisprog);
		fprintf(stderr,"	[in]: input file name or \"stdin\"\n");
		fprintf(stderr,"VALID OPTIONS: defaults in []\n");
		fprintf(stderr,"	-head: non-comment lines at top of file to pass unaltered [%ld]\n",sethead);
		fprintf(stderr,"	-id1: col-no. for id#1 (float), e.g. subject [%ld]\n",setid1);
		fprintf(stderr,"	-id2: col-no. for id#2 (float), e.g. group [%ld]\n",setid2);
		fprintf(stderr,"	-rep: col-no. for repeated category (float), e.g. time [%ld]\n",setrep);
		fprintf(stderr,"	-val: col-no. for values (float) [%ld]\n",setval);
		fprintf(stderr,"	-norm: normalization type: [%d]\n",setnorm);
		fprintf(stderr,"		-1: no normalization \n");
		fprintf(stderr,"		 0: 0-1 range\n");
		fprintf(stderr,"		 1: z-scores (see -n1/-n2)\n");
		fprintf(stderr,"		 2: difference from first valid sample (see -n1)\n");
		fprintf(stderr,"		 3: difference from mean (see -n1/-n2) \n");
		fprintf(stderr,"		 4: ratio of mean (see -n1/-n2)\n");
		fprintf(stderr,"	-n1: beginning of normalization zone (units) [%g]\n",setn1);
		fprintf(stderr,"	-n2: end of normalization zone (units) [%g]\n",setn2);
		fprintf(stderr,"	-verb: verbose output (0=NO 1=YES 999=DEBUG) [%d]\n",setverb);
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s data.txt -t 1\n",thisprog);
		fprintf(stderr,"	cat temp.txt | %s stdin -t 3\n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	four columns od data : id1,id2,rep, and val(normalised)\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/********************************************************************************
	READ THE FILENAME AND OPTIONAL ARGUMENTS - including comma-separated list item
	********************************************************************************/
	infile= argv[1];
	for(ii=2;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-id1")==0)  setid1= atol(argv[++ii]);
			else if(strcmp(argv[ii],"-id2")==0)  setid2= atol(argv[++ii]);
			else if(strcmp(argv[ii],"-rep")==0)  setrep= atol(argv[++ii]);
			else if(strcmp(argv[ii],"-val")==0)  setval= atol(argv[++ii]);
			else if(strcmp(argv[ii],"-norm")==0) setnorm= atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-n1")==0)   setn1= atof(argv[++ii]);
			else if(strcmp(argv[ii],"-n2")==0)   setn2= atof(argv[++ii]);
			else if(strcmp(argv[ii],"-head")==0) sethead= atol(argv[++ii]);
			else if(strcmp(argv[ii],"-verb")==0) setverb= atoi(argv[++ii]);
			else {fprintf(stderr,"\n--- Error [%s]: invalid command line argument [%s]\n\n",thisprog,argv[ii]); exit(1);}
	}}
	if(setnorm<-1||setnorm>4) {fprintf(stderr,"\n--- Error[%s]: -nnorm (%d) : must be -1 or 0-4\n\n",thisprog,setnorm); exit(1);}
	if(setverb!=0 && setverb!=1 && setverb != 999) { fprintf(stderr,"\n--- Error [%s]: invalid -verb [%d] must be 0,1, or 999\n\n",thisprog,setverb);exit(1);}

	setid1--;
	setid2--;
	setrep--;
	setval--;

	/********************************************************************************
	STORE DATA - ASSUME WE DON'T KNOW THE LENGTH OF EACH INPUT LINE
	********************************************************************************/
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}

	sizeofid1= sizeof(*datid1);
	sizeofid2= sizeof(*datid2);
	sizeofrep= sizeof(*datrep);
	sizeofval= sizeof(*datval);

	nn= nlines= 0;
	while((line=xf_lineread1(line,&maxlinelen,fpin))!=NULL) {
		if(maxlinelen==-1)  {fprintf(stderr,"\n--- Error[%s]: readline function encountered insufficient memory\n\n",thisprog);exit(1);}
		/* parse the line */
		iword= xf_lineparse2(line,"\t",&nwords);
		if(nwords<0) {fprintf(stderr,"\n--- Error[%s]: lineparse function encountered insufficient memory\n\n",thisprog);exit(1);};
		/* preserve header and leading blank lines if required */
		nlines++;
		if(sethead>0) {
			if(line[0]=='#' || nlines<=sethead) { internal_print4(line,iword,setid1,setid2,setrep,setval); if(strlen(line)>1) mm++; continue; }
		}
		/* make sure required columns are present */
		if(nwords<setrep || nwords<setval) continue;
		/* make sure content in x- and y-columns is numeric */
		if(sscanf(line+iword[setid1],"%lf",&aa)!=1 || !isfinite(aa)) {fprintf(stderr,"\n--- Error[%s]: id1 column contains non-numeric values\n\n",thisprog);exit(1);};
		if(sscanf(line+iword[setid2],"%lf",&bb)!=1 || !isfinite(bb)) {fprintf(stderr,"\n--- Error[%s]: id2 column contains non-numeric values\n\n",thisprog);exit(1);};
		if(sscanf(line+iword[setrep],"%lf",&cc)!=1 || !isfinite(cc)) cc= NAN;
		if(sscanf(line+iword[setval],"%lf",&dd)!=1 || !isfinite(dd)) dd= NAN;
		/* dynamically allocate memory */
		datid1= realloc(datid1,(nn+1)*sizeofid1);
		datid2= realloc(datid2,(nn+1)*sizeofid2);
		datrep= realloc(datrep,(nn+1)*sizeofrep);
		datval= realloc(datval,(nn+1)*sizeofval);
		if(datid1==NULL || datid2==NULL || datrep==NULL || datval==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		/* store values */
		datid1[nn]= aa;
		datid2[nn]= bb;
		datrep[nn]= cc;
		datval[nn]= dd;
		/* increment data-counter */
		nn++;
	}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);
	//TEST for(ii=0;ii<nn;ii++) printf("%ld	%ld	%lf	%lf\n",datid1[ii],datid2[ii],datrep[ii],datval[ii]);

	/* SKIP ALL THIS IF NO NORMALIZATION IS REQUIRED  */
	if(setnorm<0) goto OUTPUT;

	/********************************************************************************
	NORMALIZE
	********************************************************************************/
	/* initializa the block-start & stop */
	previd1= datid1[0];
	previd2= datid2[0];
	start=-1;
	stop=-1;
	indexa= 0;
	indexb= nn;

	for(ii=0;ii<=nn;ii++) { // we need this loop to go to nn

		/* PROCESS THE DATA WHENEVER SUBJECT OR GROUP CHANGE, OR IF THIS IS THE END OF THE INPUT */
		if(datid1[ii]!=previd1 || datid2[ii]!=previd2 || ii==nn) {

			indexb= ii;    /* set the hard-stop for this block of data */
			mm= 0;         /* initialize the within-block counter */
			start= -1;     /* start by default is invalid */
			stop=  -1;     /* stop by default is invalid */
			//TEST:for(jj=indexa;jj<indexb;jj++) printf("%ld\t%ld\t%.3f\t%.3f\n",datid1[jj],datid2[jj],datrep[jj],datval[jj]); printf("\n");

			/* SCROLL THOUGH THE BLOCK TO FIND START & STOP INDICES FOR THE NORMALIZATION FUNCTION */
			for(jj=indexa;jj<indexb;jj++) { /* indexa is inherited from the previous iterration, or from initialization */
				aa= datrep[jj];
				/* find the index to the repeated-measures value first corresponding with setn1 */
				if(aa>=setn1 && aa<setn2 && start<0) start= mm;
				/* find the index to the repeated-measures value first corresponding with setn2 */
				if(aa>=setn2 && stop<0) stop= mm;
				/* increment the within-block counter - this will serve as the block-size on completion of the loop */
				mm++;
				//TEST:	printf("rep=%g\tstart=%ld\tstop=%ld\n",datrep[jj],start,stop);
			}

			/* CHECK THAT NORMALIZATION START AND STOP WERE DEFINED - USE FAKE START UNDER CERTAIN CONDITIONS */
			if(start<0 && setnorm==0) start= 0; /* allow for no valid start if normalization= 0-1 range */
			if(stop==-1) stop= indexb;
			/* NOW NORMALIZE THE VALUES INSIDE THIS BLOCK */
			if(start>=0) {
				kk= xf_norm3_d((datval+indexa),(indexb-indexa),setnorm,start,stop,message);
				if(kk==-2) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }
				if(kk==-1 && setverb>0) fprintf(stderr,"\t*** %s/%s\n",thisprog,message);
			}
			else { for(jj=indexa;jj<indexb;jj++) datval[jj]= NAN; }

			/* BREAK IF WE HAVE GONE PAST THE LAST SAMPLE */
			if(ii==nn) break;

			/* RESET VARIABLES */
			indexa= indexb;
			previd1= datid1[ii];
			previd2= datid2[ii];

			/* ROLL BACK ii SO THE CURRENT ROW GET'S RE-TESTED FOR BEGINNING A NEW BLOCK */
			ii--;
		}
	}

	/********************************************************************************
	OUTPUT:
	********************************************************************************/
OUTPUT:
	for(ii=0;ii<nn;ii++) {
		printf("%g\t%g\t%g\t%lf\n",datid1[ii],datid2[ii],datrep[ii],datval[ii]);
	}

	/********************************************************************************/
	/* CLEANUP AND EXIT */
	/********************************************************************************/
END:
	if(line!=NULL) free(line);
	if(iword!=NULL) free(iword);
	if(datid1!=NULL) free(datid1);
	if(datid2!=NULL) free(datid2);
	if(datrep!=NULL) free(datrep);
	if(datval!=NULL) free(datval);
	exit(0);
}
