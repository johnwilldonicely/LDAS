#define thisprog "xe-stripcomments"
#define TITLE_STRING thisprog" v 1: 26.January.2018 [JRH]"
#define MAXLINELEN 1000

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
<TAGS>string database</TAGS>
v 1: 26.January.2018 [JRH]
	- remove comments from a file
	- based on xe-progdep
*/


/* external functions start */
char *xf_lineread1(char *line, long *maxlinelen, FILE *fpin);
/* external functions end */

int main (int argc, char *argv[]) {
	/* general variables */
	char *infile=NULL,*line=NULL;
	int x,y,z;
	long ii,jj,kk,nn,mm,maxlinelen=0;
	FILE *fpin;
	/* program-specific variables */
	long linecount=0,linestart=0;
	char prev=' ';
	int skip=0;
	/* arguments */
	int settype=1;

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Remove comments from a file: shell, C/C++, or both types\n");
		fprintf(stderr,"	- all text between /* and */ \n");
		fprintf(stderr,"	- everything after the first # or // on a given line\n");
		fprintf(stderr,"- ignores comment-markers contained within quotes\n");
		fprintf(stderr,"- a blank line will remain if required, to preserve the linecount\n");
		fprintf(stderr,"USAGE: %s [input] [options]\n",thisprog);
		fprintf(stderr,"	[input]: file name or \"stdin\"\n");
		fprintf(stderr,"VALID OPTIONS: defaults in []\n");
		fprintf(stderr,"	-type: type of comments to remove [%d]\n",settype);
		fprintf(stderr,"		0= none\n");
		fprintf(stderr,"		1= # (shell single-line)\n");
		fprintf(stderr,"		2= /* */  or // (C and C++ style block or single-line)\n");
		fprintf(stderr,"		3= both 1 and 2\n");
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s xs-myscript\n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	- the original input with comments removed\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/********************************************************************************
	READ THE FILENAME AND OPTIONAL ARGUMENTS
	********************************************************************************/
	infile=argv[1];

	for(ii=2;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-type")==0) 	settype=atoi(argv[++ii]);
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
	}}
	if(settype<1||settype>3) {fprintf(stderr,"\n--- Error[%s]: invalid -type (%d) : must be 1 2 or 3\n\n",thisprog,settype); exit(1);}


	/* READ THE FILE LINE-BY LINE LOOKING FOR WORDS SATRING WITH xs- xe- or xf_ */
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
	skip=0;
	linecount=0;
	ii=jj=kk=0;

	/* OPTION 0: NO COMMENT REMOVAL */
	if(settype==0) {
		while((line=xf_lineread1(line,&maxlinelen,fpin))!=NULL) printf("%s",line);
	}

	/* OPTION 1: SHELL-STYLE #-COMMENTS, INVALIDATES EVERYTHING AFTER THE # */
	if(settype==1) {
		while((line=xf_lineread1(line,&maxlinelen,fpin))!=NULL) {
			if(skip==3) skip=0;
		 	nn= strlen(line);
			for(ii=0;ii<nn;ii++) {
				/* handle single-quotes - may signal start or end of quote */
				if(line[ii]=='\'' && prev!='\\' && (skip==0 || skip==1)) { if(skip==1) skip=0; else { skip=1;continue;}}
				/* handle double-quotes - may signal start or end of quote  */
				if(line[ii]=='\"' && prev!='\\' && (skip==0 || skip==2)) { if(skip==2) skip=0; else { skip=2;continue;}}
				/* handle hash-symbols - marks the end of a line */
				if(line[ii]=='#' && skip==0) { skip=3; line[ii]='\n'; line[ii+1]='\0'; nn=ii+1;	break; }
			}
			printf("%s",line);
		}
	}



	/* OPTION 2: C/C++-STYLE COMMENTS */
	if(settype==2) {
		while((line=xf_lineread1(line,&maxlinelen,fpin))!=NULL) {
			if(skip==4) skip=0;
		 	nn= strlen(line);
			jj=0;
			for(ii=0;ii<nn;ii++) {
				/* handle single-quotes - may signal start or end of quote */
				if(line[ii]=='\'' && prev!='\\' && (skip==0 || skip==1)) { if(skip==1) skip=0; else { skip=1;continue;}}
				/* handle double-quotes - may signal start or end of quote  */
				if(line[ii]=='\"' && prev!='\\' && (skip==0 || skip==2)) { if(skip==2) skip=0; else { skip=2;continue;}}
				/* handle c++ style double-slash - marks the end of a line */
		  		if(ii>0 && line[ii]=='/' && prev=='/' && skip==0) { skip=4; line[ii]='\n'; line[ii+1]='\0'; nn=ii+1; break; }
				/* handle c-style slash-asterix comments */
				if(ii>0 && line[ii]=='*' && prev=='/' && skip==0) { skip=5; linestart=linecount; jj=ii-1; continue; }
		  		if(ii>0 && line[ii]=='/' && prev=='*' && skip==5) {
					skip=6;
					/* if comment ends on the same line, remove the comment section.*/
					if(linecount==linestart) {
						for(kk=ii+1;kk<nn;kk++) line[jj++]=line[kk];
						line[jj]='\0'; ii=jj-1;	continue;
					}
					/* otherwise invalidate the current line up to this point */
					else { line+=(ii+1); nn-=(ii+1); ii=-1;continue; }
				}
				prev= line[ii];
			}
			/* if an unterminated comment was found, terminate at the position of the beginning */
			if(skip==5) { line[jj]='\n'; line[jj+1]='\0';}
			printf("%s",line);
			if(skip==6) skip=0;
			linecount++;
		}
	}


	/* OPTION 3: SHELL AND C/C++-STYLE COMMENTS */
	if(settype==3) {
		while((line=xf_lineread1(line,&maxlinelen,fpin))!=NULL) {
			if(skip==3||skip==4) skip=0;
		 	nn= strlen(line);
			for(ii=0;ii<nn;ii++) {
				/* handle single-quotes - may signal start or end of quote */
				if(line[ii]=='\'' && prev!='\\' && (skip==0 || skip==1)) { if(skip==1) skip=0; else { skip=1;continue;}}
				/* handle double-quotes - may signal start or end of quote  */
				if(line[ii]=='\"' && prev!='\\' && (skip==0 || skip==2)) { if(skip==2) skip=0; else { skip=2;continue;}}
				/* handle hash-symbols - marks the end of a line */
				if(line[ii]=='#' && skip==0) { skip=3; line[ii]='\n'; line[ii+1]='\0'; nn=ii+1;	break; }
				/* handle c++ style double-slash - marks the end of a line */
		  		if(ii>0 && line[ii]=='/' && prev=='/' && skip==0) { skip=4; line[ii]='\n'; line[ii+1]='\0'; nn=ii+1; break; }
				/* handle c-style slash-asterix comments */
				if(ii>0 && line[ii]=='*' && prev=='/' && skip==0) { skip=5; linestart=linecount; jj=ii-1; continue; }
		  		if(ii>0 && line[ii]=='/' && prev=='*' && skip==5) {
					skip=6;
					/* if comment ends on the same line, remove the comment section.*/
					if(linecount==linestart) {
						for(kk=ii+1;kk<nn;kk++) line[jj++]=line[kk];
						line[jj]='\0'; ii=jj-1;	continue;
					}
					/* otherwise invalidate the current line up to this point */
					else { line+=(ii+1); nn-=(ii+1); ii=-1;continue; }
				}
				prev= line[ii];
			}
			/* if an unterminated comment was found, terminate at the position of the beginning */
			if(skip==5) { line[jj]='\n'; line[jj+1]='\0';}
			printf("%s",line);
			if(skip==6) skip=0;
			linecount++;
		}
	}




	if(strcmp(infile,"stdin")!=0) fclose(fpin);
	exit(0);
	}
