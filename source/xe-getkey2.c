#define thisprog "xe-getkey2"
#define TITLE_STRING thisprog" v 1: 4.November.2016 [JRH]"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
<TAGS>database</TAGS>
v 4: 8.September.2015 [JRH]
	- built on xe-getkey, but use new version of xf_lineparse1 so quoted key values can be preserved

*/


/* external functions start */
char *xf_lineread1(char *line, long *maxlinelen, FILE *fpin);
long *xf_lineparse1(char *line,long *nwords);
/* external functions end */

int main (int argc, char *argv[]) {

	char infile[1000],keyword[256],*line=NULL,*pcol,*pline;
	char *delimiters;
	long ii,jj,kk,col,maxlinelen=0,nwords,*start=NULL;
	FILE *fpin,*fpout;
	/* arguments */
	int setdelimiters=0,setmatch=2;


	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<3) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Detect a key's value when the pair are white-space separated\n");
		fprintf(stderr,"- similar to xe-getkey1, but this program won't break quoted text\n");
		fprintf(stderr,"- in other words, place values containing spaces in quotes\n");
		fprintf(stderr,"- for shell input this requires double-quoting values, e.g. \'\"a b c\"\'\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [input] [key] [options]\n",thisprog);
		fprintf(stderr,"		[input]: file name or \"stdin\"\n");
		fprintf(stderr,"		[key]: the key for which the value is required\n");
		fprintf(stderr,"VALID OPTIONS:\n");
		fprintf(stderr,"	-m: key match mode, 1=contains, 2=exact [%d]\n",setmatch);
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s data.txt RATE\n",thisprog);
		fprintf(stderr,"	echo name Joe greet \'\"good morning\"\' | xe-getkey2 stdin greet\n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	key value (the next word on the line after the key)\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	sprintf(infile,"%s",argv[1]);
	sprintf(keyword,"%s",argv[2]);
	for(ii=3;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-d")==0) { setdelimiters=1; delimiters=(argv[++ii]); }
			else if(strcmp(argv[ii],"-m")==0) setmatch=atoi(argv[++ii]);
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
	}}
	if(setdelimiters==0) delimiters=" \t";
	if(setmatch!=1&&setmatch!=2) {fprintf(stderr,"\n--- Error[%s]: invalid match mode (-m %d): must be 1 or 2\n",thisprog,setmatch); exit(1);}


	/* READ THE INPUT AND OUPUT MATCHING KEYWORD VALUES */
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}

	if(setmatch==1) {
		while((line=xf_lineread1(line,&maxlinelen,fpin))!=NULL) {
			if(maxlinelen==-1)  {fprintf(stderr,"\n--- Error[%s]: readline function encountered insufficient memory\n\n",thisprog);exit(1);}
			start= xf_lineparse1(line,&nwords);
			for(ii=0;ii<nwords;ii++) {
				if(strstr(line+start[ii],keyword)!=NULL) {
					if(ii<nwords-1) printf("%s\n",line+start[ii+1]);
	}}}}
	if(setmatch==2) {
		while((line=xf_lineread1(line,&maxlinelen,fpin))!=NULL) {
			if(maxlinelen==-1)  {fprintf(stderr,"\n--- Error[%s]: readline function encountered insufficient memory\n\n",thisprog);exit(1);}
			start= xf_lineparse1(line,&nwords);
			for(ii=0;ii<nwords;ii++) {
				if(strcmp(line+start[ii],keyword)==0) {
					if(ii<nwords-1) printf("%s\n",line+start[ii+1]);
	}}}}



	if(strcmp(infile,"stdin")!=0) fclose(fpin);

	if(line!=NULL) free(line);
	if(start==NULL) free(start);

	exit(0);
	}
