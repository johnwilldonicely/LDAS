#define thisprog "xe-normrow2"
#define TITLE_STRING thisprog" 7.April.2019 [JRH]"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

/*
<TAGS>stats</TAGS>

7.April.2019 [JRH] - improvement on previous versions of normrow
	- dynamic memory allocation for lines and goodcol
	- use of non-destructive line-parsing
	- use of norm function instead of inline processing
	- fixed output of redundant tab at end of every line
	- update variable-naming to use latest conventions


*/

/* external functions start */
char *xf_lineread1(char *line, long *maxlinelen, FILE *fpin);
long *xf_lineparse2(char *line,char *delimiters, long *nwords);
long xf_norm3_d(double *data,long ndata,int normtype,long start,long stop,char *message);

/* external functions end */

int main(int argc, char *argv[]) {

	/******************************************************************************
	Initialise variables
	******************************************************************************/
	char *line=NULL;
	int v,w,x,y,z;
	long ii,jj,kk,nn,maxlinelen,prevlinelen;
	float a,b,c;
	double aa,bb,cc,result_d[64];
	FILE *fpin;

	/* program-specific variables */
	char message[256];
	long *iword=NULL,*goodcol=NULL,nwords,nwordsmax=0,nlines,ncheck;
	double *data1=NULL;
	size_t sizeofgoodcol=sizeof(*goodcol), sizeofdata1=sizeof(*data1);

	/* arguments */
	char *infile=NULL;
	int setnorm=1;
	long sethead=0,setskip=0,setstart=0,setstop=-1;

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc==1) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Normalize data within each row of input\n");
		fprintf(stderr," - non-numeric and non-finite values will be ignored\n");
		fprintf(stderr," - accepted input delimiters: space,tab,comma\n");
		fprintf(stderr," - output will be tab-delimited\n");
		fprintf(stderr,"USAGE: %s [infile|stdin] [options]\n",thisprog);
		fprintf(stderr,"VALID OPTIONS:\n");
		fprintf(stderr,"	-head: non-comment lines at top of file to pass unaltered [%ld]\n",sethead);
		fprintf(stderr,"	-skip: number of columns to skip (preserve) [%ld]\n",setskip);
		fprintf(stderr,"	-norm: normalization type: [%d]\n",setnorm);
		fprintf(stderr,"		-1: no normalization \n");
		fprintf(stderr,"		 0: 0-1 range\n");
		fprintf(stderr,"		 1: z-scores (see start/stop)\n");
		fprintf(stderr,"		 2: difference from first valid sample (see start)\n");
		fprintf(stderr,"		 3: difference from mean (see start/stop) \n");
		fprintf(stderr,"		 4: ratio of mean (see start/stop)\n");
		fprintf(stderr,"	-start: start of normalization range (-1= first valid) [%ld]\n",setstart);
		fprintf(stderr,"	-stop:  end of normalization range (-1= last valid) [%ld]\n",setstop);
		fprintf(stderr,"		NOTE: start/stop are samples-after-skip\n");
		fprintf(stderr,"		NOTE: stop = sample just AFTER the last to be included\n");
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	cat infile.txt | %s stdin -n 1\n",thisprog);
		fprintf(stderr,"	%s infile.txt -n 0 -invalid -1\n",thisprog);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/********************************************************************************
	READ THE FILENAME AND OPTIONAL ARGUMENTS
	********************************************************************************/
	infile= argv[1];
	for(ii=2;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-head")==0) sethead= atol(argv[++ii]);
			else if(strcmp(argv[ii],"-skip")==0) setskip= atol(argv[++ii]);
			else if(strcmp(argv[ii],"-norm")==0) setnorm= atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-start")==0) setstart= atol(argv[++ii]);
			else if(strcmp(argv[ii],"-stop")==0) setstop= atol(argv[++ii]);
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
	}}
	if(setskip<0) {fprintf(stderr,"\n--- Error[%s]: invalid -skip (%ld) - must be >=0\n\n",thisprog,setskip); exit(1);}
	if(setnorm<-1||setnorm>4) {fprintf(stderr,"\n--- Error[%s]: -n must be -1 or 0-4\n\n",thisprog,setnorm); exit(1);}

	/******************************************************************************
	READ THE DATA AND NORMALIZE EACH LINE (ROW)
	******************************************************************************/
	nwordsmax= maxlinelen= prevlinelen= 0;
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
	nlines=0;
	while((line=xf_lineread1(line,&maxlinelen,fpin))!=NULL) {
		if(maxlinelen==-1)  {fprintf(stderr,"\n--- Error[%s]: readline function encountered insufficient memory\n\n",thisprog);exit(1);}

		nlines++;
		if(line[0]=='#' || nlines<=sethead) { printf("%s",line); continue; }

		/* PARSE THE LINE */
		iword= xf_lineparse2(line," ,\t",&nwords);
		if(nwords<0) {fprintf(stderr,"\n--- Error[%s]: lineparse function encountered insufficient memory\n\n",thisprog);exit(1);};

		/* DYNAMICALLY ALLOCATE MEMORY FOR DATA - INCLUDING SKIPPED COLUMNS */
		if(nwords>nwordsmax) {
			nwordsmax= nwords;
			data1= realloc(data1,(nwordsmax+1)*sizeofdata1);
			goodcol= realloc(goodcol,(nwordsmax+1)*sizeofgoodcol);
			if(data1==NULL || goodcol==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
		}

		/* STORE THE DATA FOR THIS LINE, STARTING FROM SETSKIP */
		for(ii=setskip;ii<nwords;ii++) {
			if(sscanf(line+iword[ii],"%lf",&aa)==1 && isfinite(aa)) { data1[ii]= aa; goodcol[ii]= 1; }
			else { data1[ii]= NAN; goodcol[ii]= 0; }
		}

		/* PERFORM THE NORMALIZATION */
		ncheck=0;
		if(setnorm>=0) {
			ncheck= xf_norm3_d((data1+setskip),(nwords-setskip),setnorm,setstart,setstop,message);
			if(ncheck==-2) { fprintf(stderr,"\n\t--- %s/%s\n\n",thisprog,message); exit(1); }
			if(ncheck==-1) fprintf(stderr,"\t*** %s/%s\n",thisprog,message);
		}

		/* OUTPUT ADJUSTED SCORES, OR ORIGINAL FIELDS IF NOT NUMBERS OR INVALID VALUE */
		if(ncheck>=0) {
			for(ii=0;ii<nwords;ii++) {
				if(ii>0) printf("\t"); /* print a tab separator for all but the first field */
				if(ii<setskip) printf("%s",(line+iword[ii]));
				else printf("%lf",data1[ii]);
			}
			printf("\n");
		}
		else {
			for(ii=0;ii<nwords;ii++) {
				if(ii>0) printf("\t"); /* print a tab separator for all but the first field */
				if(ii<setskip) printf("%s",(line+iword[ii]));
				else printf("NAN");
			}
			printf("\n");
		}
	}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);

	/* FREE(DATA); */
	if(line!=NULL) free(line);
	if(data1!=NULL) free(data1);
	if(goodcol!=NULL) free(goodcol);
	exit(0);
}
