#define thisprog "xe-getkey"
#define TITLE_STRING thisprog" v 4: 8.September.2015 [JRH]"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
<TAGS>database</TAGS>
v 4: 28.September.2018 [JRH]
	- allow outputting of first match only (-f option)
	- restrict word search to nwords-1 (never try to match the last word on each line)
	- update variable names

v 4: 8.September.2015 [JRH]
	- added ability to match sub-strings

v 3: 6.July.2015 [JRH]
	- bugfix: variable "line" DOES need to be initialized to NULL, however - on some platforms

v 2: 1.April.2014 [JRH]
	- don't pre-allocate memory for line - unnecessary (let xf_lineread function handle it)

v 1: 19.February.2014
	- update to use line-length independent read function xf_lineread1
	- update to use thread-safe xf_lineparse2 function to determine "words"
	- change default delimiters to omit commas and newlines
	- add ability to specify a different delimiter set
*/


/* external functions start */
char *xf_lineread1(char *line, long *maxlinelen, FILE *fpin);
long *xf_lineparse2(char *line,char *delimiters, long *nwords);
/* external functions end */

int main (int argc, char *argv[]) {

	char infile[1000],keyword[256],*line=NULL,*pcol,*pline;
	char *delimiters;
	long ii,jj,kk,col,maxlinelen=0,nwords,*start=NULL;
	FILE *fpin,*fpout;
	/* arguments */
	int setdelimiters=0,setmatch=2,setfirst=0;


	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<3) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Output the value following a keyword on each line\n");
		fprintf(stderr,"- assumes words on each line are white-space delimited unless -d is set\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [input] [key] [options]\n",thisprog);
		fprintf(stderr,"		[input]: file name or \"stdin\"\n");
		fprintf(stderr,"		[key]: the keyword to be matched (any word on the line)\n");
		fprintf(stderr,"VALID OPTIONS:\n");
		fprintf(stderr,"	-d: specify alternative delimiter(s)  [default is \" \\t\"]\n");
		fprintf(stderr,"	-m: keyword match mode, 1=contains, 2=exact [%d]\n",setmatch);
		fprintf(stderr,"	-f: output first match only (0=NO 1=YES) [%d]\n",setfirst);
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s data.txt RATE\n",thisprog);
		fprintf(stderr,"	cat temp.txt | %s stdin PHONE -d \":\"\n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	keyword value (the next word on the line after the keyword)\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	sprintf(infile,"%s",argv[1]);
	sprintf(keyword,"%s",argv[2]);

	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	sprintf(infile,"%s",argv[1]);
	sprintf(keyword,"%s",argv[2]);
	for(ii=3;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-d")==0) { setdelimiters=1; delimiters=(argv[++ii]); }
			else if(strcmp(argv[ii],"-m")==0) setmatch=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-f")==0) setfirst=atoi(argv[++ii]);
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
	}}
	if(setdelimiters==0) delimiters=" \t";
	if(setmatch!=1&&setmatch!=2) {fprintf(stderr,"\n--- Error[%s]: invalid match mode (-m %d): must be 1 or 2\n",thisprog,setmatch); exit(1);}
	if(setfirst!=0&&setfirst!=1) {fprintf(stderr,"\n--- Error[%s]: invalid first mode (-f %d): must be 0 or 1\n",thisprog,setfirst); exit(1);}


	/* READ THE INPUT AND OUPUT MATCHING KEYWORD VALUES */
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}

	if(setmatch==1) {
		while((line=xf_lineread1(line,&maxlinelen,fpin))!=NULL) {
			if(maxlinelen==-1)  {fprintf(stderr,"\n--- Error[%s]: readline function encountered insufficient memory\n\n",thisprog);exit(1);}
			start= xf_lineparse2(line,delimiters,&nwords);
			jj= nwords-1;
			kk=0;
			for(ii=0;ii<jj;ii++) {
				if(strstr(line+start[ii],keyword)!=NULL) {
					printf("%s\n",line+start[ii+1]);
					kk++;
				}
			}
			if(setfirst==1 && kk>0) break;
		}
	}

	if(setmatch==2) {
		while((line=xf_lineread1(line,&maxlinelen,fpin))!=NULL) {
			if(maxlinelen==-1)  {fprintf(stderr,"\n--- Error[%s]: readline function encountered insufficient memory\n\n",thisprog);exit(1);}
			start= xf_lineparse2(line,delimiters,&nwords);
			jj= nwords-1;
			kk=0;
			for(ii=0;ii<jj;ii++) {
				if(strcmp(line+start[ii],keyword)==0) {
					printf("%s\n",line+start[ii+1]);
					kk++;
				}
			}
			if(setfirst==1 && kk>0) break;
		}
	}



	if(strcmp(infile,"stdin")!=0) fclose(fpin);

	if(line!=NULL) free(line);
	if(start==NULL) free(start);

	exit(0);
	}
