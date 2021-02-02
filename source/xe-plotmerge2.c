#define thisprog "xe-plotmerge2"
#define TITLE_STRING thisprog" 2.February.2021 [JRH]"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
<TAGS>plot</TAGS>

2.February.2021 [JRH]
	- allow omission of the filename-header on each page
	- also required making a file list of arguments NOT beginning with a hyphen

v 1: 20.March.2019 [JRH]
	- just pass-through if only one document is to be "merged"
v 1: 22.May.2018 [JRH]
	- add document page-size specification for multi-page documents
v 1: 6.November.2017 [JRH]
*/

/* external functions start */
char *xf_lineread1(char *line, long *maxlinelen, FILE *fpin);
/* external functions end */

int main (int argc, char *argv[]) {
	/* general variables */
	char *line=NULL,message[256];
	long ii,nn,maxlinelen=0;
	double aa,bb,cc;
	FILE *fpin;
	/* program-specific variables */
	long nfiles=0, *findex=NULL;
	/* arguments */
	char *infile=NULL;
	int sethead=1;


	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Merge postscript files into a single multi-page document\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"    %s [list] [options]\n",thisprog);
		fprintf(stderr,"    [list]: list of file names (space-seperated) to merge\n");
		fprintf(stderr,"VALID OPTIONS (default in [])...\n");
		fprintf(stderr,"    -head: include a file-name page-header (0=NO 1=YES) [%d]\n",sethead);
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s a.ps b.ps c.ps  -head 1\n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	A single multi-paged postscript document, sent to stdout\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}


	/********************************************************************************
	READ THE FILENAME AND OPTIONAL ARGUMENTS
	********************************************************************************/
	infile= argv[1];
	for(ii=1;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-head")==0)  sethead= atoi(argv[++ii]);
			else {fprintf(stderr,"\n--- Error [%s]: invalid command line argument [%s]\n\n",thisprog,argv[ii]); exit(1);}
		}
		/* otherwise add it to the file list */
		else {
			findex= realloc(findex,(nfiles+1)*sizeof(long));
			if(findex==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
			findex[nfiles]= ii;
			nfiles++;
		}
	}
	if(sethead!=0 && sethead!=1) {fprintf(stderr,"\n\a--- Error[%s]: illegal -nohead (%d), must be either 0 or 1\n\n",thisprog,sethead);exit(1);}
	//for(ii=0;ii<nfiles;ii++) printf("%ld: %ld: %s\n",ii,findex[ii],argv[findex[ii]]); exit(0);

	/********************************************************************************
	START THE MERGE
	********************************************************************************/

	/* print a postcript header if there are two or more files  */
	if(nfiles>1) {
		printf("%%!PS-Adobe-2.0\n");
		printf("%%%%Pages: %ld\n",nfiles);
		printf("%%%%DocumentMedia: A4 595 842 80 white ( )\n");
		printf("\n");
	}
	nn=0;
	for(ii=0;ii<nfiles;ii++) {
		nn++;
		infile= argv[findex[ii]];
		if(nfiles>1) {
			printf("%%%%Page: %ld\n",nn);
			printf("/Helvetica findfont 12.00 scalefont setfont\n");
			if(sethead==1) printf("0 830 moveto (%s) show\n",infile);
		}
		if(sethead==1) printf("0 830 moveto (%s) show\n",infile);
		if((fpin=fopen(infile,"r"))==NULL) { fprintf(stderr,"\n\t\b*--- Error[%s]: file \"%s\" not found\n",thisprog,infile);exit(1);}
		while((line=xf_lineread1(line,&maxlinelen,fpin))!=NULL) {
			if(maxlinelen==-1)  {fprintf(stderr,"\n--- Error[%s]: readline function encountered insufficient memory\n\n",thisprog);exit(1);}
			printf("%s",line);
		}
		fclose(fpin);
	}

	if(line!=NULL) free(line);
	exit(0);
}
