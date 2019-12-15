#define thisprog "xe-getkeyline1"
#define TITLE_STRING thisprog" v 1: 12.January.2018 [JRH]"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
<TAGS>database</TAGS>
v 1: 12.January.2018 [JRH]
*/


/* external functions start */
char *xf_lineread1(char *line, long *maxlinelen, FILE *fpin);
long *xf_lineparse2(char *line,char *delimiters, long *nkeys);

/* external functions end */

int main (int argc, char *argv[]) {

	char *infile=NULL,*keywords=NULL,*line=NULL;
	char *delimiters;
	long ii,jj,kk,col,maxlinelen=0,nkeys,*start=NULL;
	FILE *fpin,*fpout;
	/* program-specific variables */
	int lineout=0;
	long nout=0;
	char *pkey;
	/* arguments */
	int setdelimiters=0,setomit=0;
	long setnout=-1;


	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<3) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Output lines following a key-line containing patterns\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [input] \"[patterns]\" [options]\n",thisprog);
		fprintf(stderr,"		[input]: file name or \"stdin\"\n");
		fprintf(stderr,"		[patterns]: CSV list of patterns to match\n");
		fprintf(stderr,"VALID OPTIONS:\n");
		fprintf(stderr,"	-o: omit the key-line from output (0=NO 1=YES) [%d]\n",setomit);
		fprintf(stderr,"	-n: number of lines after keyline to output (-1=ALL) [%ld]\n",setnout);
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s data.txt RATE,VALUE -o 1 \n",thisprog);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	infile= argv[1];
	keywords= argv[2];
	for(ii=3;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-o")==0) setomit=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-n")==0) setnout=atol(argv[++ii]);
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
	}}
	if(setdelimiters==0) delimiters=" \t";
	if(setomit!=0&&setomit!=1) {fprintf(stderr,"\n--- Error[%s]: invalid -o (%d): must be 0 or 1\n",thisprog,setomit); exit(1);}
	if(setnout<-1) {fprintf(stderr,"\n--- Error[%s]: invalid -n (%d): must be -1, 0, or >0\n",thisprog,setnout); exit(1);}

	/* PARSE THE KEYWORDS */
	start= xf_lineparse2(keywords,",",&nkeys);
	//TEST:	for(ii=0;ii<nkeys;ii++) { pkey= keywords+start[ii] ; printf("%ld: %s\n",ii,pkey); }


	/* READ THE INPUT AND OUPUT MATCHING KEYWORD VALUES */
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}

	while((line=xf_lineread1(line,&maxlinelen,fpin))!=NULL) {
		if(maxlinelen==-1)  {fprintf(stderr,"\n--- Error[%s]: readline function encountered insufficient memory\n\n",thisprog);exit(1);}
		if(lineout==0) {
			/* count the number of keys matched on the line */
			kk=nkeys;
			for(ii=0;ii<nkeys;ii++) {
				pkey= keywords+start[ii];
				if(strstr(line,pkey)!=NULL) kk--;
			}
			/* if all keys were identified... */
			if(kk==0) {
				if(setomit==0) printf("%s",line);
				if(setnout==0) break;
				lineout= 1;
			}
		}
		else {
			printf("%s",line);
			if(++nout==setnout) break;
		}
	}

	if(strcmp(infile,"stdin")!=0) fclose(fpin);

	if(line!=NULL) free(line);
	if(start==NULL) free(start);

	exit(0);
	}
