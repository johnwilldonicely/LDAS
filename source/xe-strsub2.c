#define thisprog "xe-strsub2"
#define TITLE_STRING thisprog" 28.June.2019 [JRH]"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>


/*
<TAGS>string</TAGS>

28.June.2019 [JRH]
*/


/* external functions start */
char *xf_lineread1(char *line, long *maxlinelen, FILE *fpin);
long *xf_lineparse2(char *line,char *delimiters, long *nwords);
long *xf_parselist1_l(char *line, char *delimiters, long min, long max, long *nitems, char *message);
/* external functions end */

int main (int argc, char *argv[]) {

	/* general variables */
	char *infile=NULL,*line=NULL,*newline=NULL,message[1000];
	long ii,jj,kk,maxlinelen=0;
	FILE *fpin;
	/* program-specific variables */
	char *newword;
	int x,y;
	long nwords=0,*iword=NULL,*listcols=NULL,nlistcols,nwordsm1;
	/* optional arguments */
	char *setnew=NULL,*setinclude=NULL,*setexclude=NULL,*setcols=NULL;
	long setmaxcols=SHRT_MAX;

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<3) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Replace all \"words\" in an input with something else\n");
		fprintf(stderr,"- delimiters (tabs and/or spaces) converted to tabs\n");
		fprintf(stderr,"- no line-length limit\n");
		fprintf(stderr,"- newlines automatically signify start of new words\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [input] [new]\n",thisprog);
		fprintf(stderr,"	[input]: file name or \"stdin\"\n");
		fprintf(stderr,"	[new]: replacement string\n");
		fprintf(stderr,"VALID OPTIONS:\n");
		fprintf(stderr,"	-cols: subtitution only specified column numbers (NULL=all) [%s]\n",setcols);
		fprintf(stderr,"	-maxcols: set maximum column [%ld]\n",setmaxcols);
		fprintf(stderr,"	-in: subtitution only in lines containing this pattern\n");
		fprintf(stderr,"	-ex: exclude subtitution in lines containing this pattern\n");
		fprintf(stderr,"		NOTE: default= both unset (all lines analyzed)\n");
		fprintf(stderr,"		NOTE: -ex will override -in\n");
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s sdatafile \" \" \"_\"\n",thisprog);
		fprintf(stderr,"	cat datafile | %s stdin NAN -ex \"#\" -cols 1,5-9\n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	modified lines, tab-delimited\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	infile= argv[1];
	setnew= argv[2];

	/* READ THE FILENAME AND OPTIONAL ARGUMENTS - including comma-separated list item */
	for(ii=3;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-cols")==0) setcols= argv[++ii];
			else if(strcmp(argv[ii],"-maxcols")==0) setmaxcols= atol(argv[++ii]);
			else if(strcmp(argv[ii],"-in")==0)  setinclude= argv[++ii];
			else if(strcmp(argv[ii],"-ex")==0)  setexclude= argv[++ii];
			else {fprintf(stderr,"\n*** %s [ERROR: invalid command line argument \"%s\"]\n\n",thisprog,argv[ii]); exit(1);}
	}}

	if(setinclude!=NULL) y=1; else y=0;
	if(setexclude!=NULL) x=1; else x=0;

	/* CONVERT THE COLUMN-LIST INTO AN ARRAY OF COLUMN-NUMBERS */
	if(setcols!=NULL) {
		listcols= xf_parselist1_l(setcols,",",0,setmaxcols,&nlistcols,message);
		if(listcols==NULL) {fprintf(stderr,"\b\n\t--- Error [%s] %s\n\n",thisprog,message); exit(1); }
		for(ii=0;ii<nlistcols;ii++) listcols[ii]--;
	}

	/* READ DATA - PARSE INTO WORDS */
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n*** %s [ERROR: file \"%s\" not found]\n\n",thisprog,infile);exit(1);}

	while((line=xf_lineread1(line,&maxlinelen,fpin))!=NULL) {
			if(maxlinelen==-1)  {fprintf(stderr,"\n--- Error[%s]: readline function encountered insufficient memory\n\n",thisprog);exit(1);}

			/* line-inclusion or exclusion criteria */
			if(x==1 && strstr(line,setexclude)!=NULL) { printf("%s",line); continue; }
			if(y==1 && strstr(line,setinclude)==NULL) { printf("%s",line); continue; }

			/* parse the line into tab/space delimited words */
			iword= xf_lineparse2(line,"\t ",&nwords);
			if(nwords<0) {fprintf(stderr,"\n--- Error[%s]: lineparse function encountered insufficient memory\n\n",thisprog);exit(1);};
			nwordsm1= nwords-1;

			/* if there is no column selection, replace all the words with "setnew" */
			if(setcols==NULL) { for(ii=0;ii<nwordsm1;ii++) printf("%s\t",setnew); printf("%s\n",setnew); continue; }

			/* otherwise, check column is in list before proceding */
			else {
				for(ii=0;ii<nwordsm1;ii++) {
					for(jj=kk=0;jj<nlistcols;jj++) { if(listcols[jj]==ii) { kk=1; break; } }
					if(kk==1) printf("%s\t",setnew);
					else      printf("%s\t",(line+iword[ii]));
				}
				for(jj=kk=0;jj<nlistcols;jj++) { if(listcols[jj]==ii) { kk=1; break; } }
				if(kk==1) printf("%s\n",setnew);
				else      printf("%s\n",(line+iword[ii]));
			}
	}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);


END:
	/* FREE MEMORY AND EXIT */
	if(line!=NULL) free(line);
	if(iword!=NULL) free(iword);
	if(listcols!=NULL) free(listcols);
	exit(0);
}
