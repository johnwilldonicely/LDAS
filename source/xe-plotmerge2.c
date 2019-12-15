#define thisprog "xe-plotmerge2"
#define TITLE_STRING thisprog" v 1: 20.March.2019 [JRH]"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
<TAGS>plot</TAGS>

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
	FILE *fpin;
	char *infile=NULL;

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Merge postscript files into a single multi-page document\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [file list]\n",thisprog);
		fprintf(stderr,"	[file list]: list of file names to merge\n");
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s a.ps b.ps c.ps \n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	A single multi-paged postscript document, sent to stdout\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/* READ THE FILENAMES AND OPTIONAL ARGUMENTS - DYNAMICALLY AND EFFICIENTLY ALLOCATE MEMORY FOR NEW FILENAMES */
	if(argc>2) {
		printf("%%!PS-Adobe-2.0\n");
		printf("%%%%Pages: %ld\n",(long)(argc-1));
		printf("%%%%DocumentMedia: A4 595 842 80 white ( )\n");
		printf("\n");
	}
	nn=0;
	for(ii=1;ii<argc;ii++) {
		nn++;
		infile= argv[ii];
		if(argc>2) {
			printf("%%%%Page: %ld\n",nn);
			printf("/Helvetica findfont 12.00 scalefont setfont\n");
			printf("0 830 moveto (%s) show\n",infile);
		}
		printf("0 830 moveto (%s) show\n",infile);
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
