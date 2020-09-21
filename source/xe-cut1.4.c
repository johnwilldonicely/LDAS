#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define thisprog "xe-cut1"
#define TITLE_STRING thisprog" v 4: 11.November.2018 [JRH]"
#define MAXLINELEN 1000

/*
<TAGS>database screen</TAGS>

v 4: 5.July.2019 [JRH]
	- add option to just count the "words" (-c) on each line - useful for testing files with "empty" columns
v 4: 11.November.2018 [JRH]
	- bugfix: missing columns now properly represented by delimiters in output
v 4: 22.September.2017 [JRH]
	- bugfix: now outputs a "-" if a specified column is missing on a given line
v 4: 24.November.2016 [JRH]
	- bugfix: numeric column-id option (-n 1) was omitting first line
		- should assume that there is no header row - all rows output
v 4: 18.October.2016 [JRH]
	- add option for numeric column definition
	- add long-line support
v 4: 23.May.2016 [JRH]
	- add option to skip comments and blank lines (-s)
	- update long int variable naming conventions
v 4: 15.March.2013 [JRH]
	- add option to omit outputting the header line
v 3: 18.February.2013 [JRH]
v 2: 14.February.2013 [JRH]
	- use new function lineparse1 to parse line into words
	- note that missing words (eg. two successive tabs) will disrupt word-numbering
	- missing columns on a given line now replaced by "-"
*/


/* external functions start */
char *xf_lineread1(char *line, long *maxlinelen, FILE *fpin);
long *xf_lineparse1(char *line, long *nwords);
long *xf_lineparse2(char *line,char *delimiters, long *nwords);
char *xf_strescape1(char *line);
/* external functions end */

int main (int argc, char *argv[]) {
	/* general variables */
	char *line=NULL,*pword;
	long ii,jj,kk,mm,nn,maxlinelen=MAXLINELEN;
	int v,w,x,y,z,col,colmatch;
	float a,b,c,d;
	double aa,bb,cc,dd;
	FILE *fpin,*fpout;
	/* program-specific variables */
	char **key=NULL;
	long *keycol=NULL,*start=NULL,nwords,nkeys,nkeysm1,keycolmax;
	/* arguments */
	char *infile,*setkeys,delimiters[MAXLINELEN],delimout[4];
	int setdelimiters=0,setomithead=0,setskip=0,setnumeric=0,setcount=0;

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<3) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Extract columns from input based on labels in the first line\n");
		fprintf(stderr,"	- the order of columns can be rearranged\n");
		fprintf(stderr,"	- multiple copies of a column can be output\n");
		fprintf(stderr,"	- for duplicated labels, only the last column matching is output\n");
		fprintf(stderr,"	- the line-count will be preserved (blank lines are output)\n");
		fprintf(stderr,"	- missing columns will be replaced by a delimiter\n");
		fprintf(stderr,"\nUSAGE: %s [input] [labels] [options]\n",thisprog);
		fprintf(stderr,"	[input]: file name or \"stdin\"\n");
		fprintf(stderr,"	[labels]: comma-separated list of column labels\n");
		fprintf(stderr,"\nVALID OPTIONS (defaults in []):\n");
		fprintf(stderr,"	[-n]: labels are numeric (1) instead of defined by header (0) [%d]\n",setnumeric);
		fprintf(stderr,"	[-s]: skip #-comments and blank lines (0=NO 1=YES) [%d]\n",setskip);
		fprintf(stderr,"		NOTE: \"#\" must be first character on the line\n");
		fprintf(stderr,"		NOTE: blank lines cannot contain spaces or tabs\n");
		fprintf(stderr,"	[-o]: omit outputting header-line (0=NO, 1=YES) [%d]\n",setomithead);
		fprintf(stderr,"	[-d]: characters to use as column-delimiters (unset by default)\n");
		fprintf(stderr,"		if unset:\n");
		fprintf(stderr,"		- white-space (blanks,tabs) are used\n");
		fprintf(stderr,"		- multiple consecutive delimiters are treated as one delimiter\n");
		fprintf(stderr,"		- suitable for reading files without \"empty\" columns\n");
		fprintf(stderr,"		if set:\n");
		fprintf(stderr,"		- behaviour is similar to the linux \"cut\" command\n");
		fprintf(stderr,"		- any matching delimiter in the input marks a new column\n");
		fprintf(stderr,"		- multiple consecutive delimiters signify \"empty\" colmns\n");
		fprintf(stderr,"		- suitable for reading CSV files, for example\n");
		fprintf(stderr,"	[-c]: just count the words on each line (0=NO 1=YES) [%d]\n",setcount);
		fprintf(stderr,"\nEXAMPLES:\n");
		fprintf(stderr,"	%s data.txt name,phone,address\n",thisprog);
		fprintf(stderr,"	cat temp.txt | %s stdin phone,name,name -d \"\\t \"\n",thisprog);
		fprintf(stderr,"\nOUTPUT:\n");
		fprintf(stderr,"	the labelled columns, in the order and number specified\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	infile= argv[1];
	setkeys= argv[2];
	for(ii=3;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-n")==0) setnumeric= atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-s")==0) setskip= atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-o")==0) setomithead= atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-d")==0)  {
				setdelimiters=1;
				sprintf(delimiters,"%s",xf_strescape1(argv[++ii]));
			}
			else if(strcmp(argv[ii],"-c")==0) setcount= atoi(argv[++ii]);
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
	}}

	if(setomithead!=0 && setomithead!=1) { fprintf(stderr,"\n--- Error [%s]: invalid -o [%d] must be 0 or 1\n\n",thisprog,setomithead);exit(1);}
	if(setskip!=0 && setskip!=1) { fprintf(stderr,"\n--- Error [%s]: invalid -s [%d] must be 0 or 1\n\n",thisprog,setskip);exit(1);}
	if(setnumeric!=0 && setnumeric!=1) { fprintf(stderr,"\n--- Error [%s]: invalid -n [%d] must be 0 or 1\n\n",thisprog,setnumeric);exit(1);}
	if(setdelimiters==0) sprintf(delimout,"\t");
	else if(setdelimiters==1) sprintf(delimout,"%c",delimiters[0]);
	if(setcount!=0 && setcount!=1) { fprintf(stderr,"\n--- Error [%s]: invalid -c [%d] must be 0 or 1\n\n",thisprog,setcount);exit(1);}

	/* BUILD THE LIST OF KEYWORDS */
	/* key is an array of pointers to portions of setkeys, and hence to the list of keywords stored in argv[] */
	if((key=(char **)realloc(key,(strlen(setkeys)*sizeof(char *))))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
	/* build a list if indices to the start of each word in the list, and convert the commas to NULLS ('\0') */
	nkeys=0; start= xf_lineparse2(setkeys,",",&nkeys);
	/* point each key to the portion of setkeys containing a keyword */
	for(jj=0;jj<nkeys;jj++) key[jj]=&setkeys[start[jj]];

	/* ASSIGN SUFFICIENT MEMORY FOR KEY COLUMNS */
	if((keycol=(long *)realloc(keycol,(nkeys)*sizeof(long)))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
	for(ii=0;ii<nkeys;ii++) keycol[ii]=-1;

	/* IF COLUMN DEFINITION IS NUMERIC, CONVERT LABELS TO NUMBERS... */
	if(setnumeric==1) for(ii=0;ii<nkeys;ii++) keycol[ii]= atol(setkeys+start[ii])-1;

	//TEST: for(ii=0;ii<nkeys;ii++) printf("key_%d:%s:\n",ii,setkeys+start[ii]);exit(0);

	/* READ THE INPUT */
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
	nn=0;
	while((line=xf_lineread1(line,&maxlinelen,fpin))!=NULL) {
		if(maxlinelen==-1)  {fprintf(stderr,"\n--- Error[%s]: readline function encountered insufficient memory\n\n",thisprog);exit(1);}

 		/* SKIP BLANK OR COMMENTED LINES */
		if(setskip==1){ if(line[0]=='#' || strlen(line)<2) continue; }
		
		/* IF THIS IS THE FIRST LINE, LOOK FOR THE LABELS (KEYS) */
		if(nn++==0 && setnumeric==0) {
			/* break up the original line using NULL characters at delimiters, and find indices to the start of each "word" */
			if(setdelimiters==0) start= xf_lineparse1(line,&nwords);
			else start= xf_lineparse2(line,delimiters,&nwords);
			/* just count the words */
			if(setcount==1) { printf("%ld\n",nwords); continue; }
			/* look for matches between words and keys */
			for(ii=0;ii<nwords;ii++) {
				pword= (line+start[ii]);
				for(jj=0;jj<nkeys;jj++) if(strcmp(key[jj],pword)==0) keycol[jj]= ii;
			}
			/* readjust key columns - drop if it doesn't exist - exit if no key columns were found */
			mm=nkeys; kk=nkeys-1; for(ii=0;ii<nkeys;ii++) { if(keycol[ii]<0) { mm--; for(jj=ii;jj<kk;jj++) { keycol[jj]=keycol[(jj+1)]; }}}
			nkeys=mm; if(nkeys<1) exit(0);
			/* output a new header line containing the labels that matched keys */
			if(setomithead==0) {
				printf("%s",line+start[keycol[0]]);
				for(ii=1;ii<nkeys;ii++) printf("%s%s",delimout,line+start[keycol[ii]]);
				printf("\n");
		}}

		/* FOR ALL OTHER LINES, OUTPUT THE APPROPRIATE COLUMNS */
		else {
			/* break up line using NULL characters at delimiters, and find indices to the start of each "word" */
			if(setdelimiters==0) start= xf_lineparse1(line,&nwords);
			else start= xf_lineparse2(line,delimiters,&nwords);
			/* just count the words */
			if(setcount==1) { printf("%ld\n",nwords); continue; }
			/* output the appropriate columns */
			if(nwords>0) {
				/* handle the first key - do not print preceeding delimiter */
				jj= keycol[0]; // key-column (zero-offset)
				kk= start[jj]; // actual start-position in line[], -1 if not found
				if(kk>=0 && jj<nwords) printf("%s",line+kk); // if key exists, print it - no preceeding delimiter
				/* handle the subsequent keys */
				for(ii=1;ii<nkeys;ii++) {
					jj= keycol[ii];
					kk= start[jj];
					if(kk>=0 && jj<nwords) printf("%s%s",delimout,line+kk);
					else printf("%s",delimout);
				}
			}
			printf("\n");
		}
	}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);

	if(line!=NULL) free(line);
	if(keycol!=NULL) free(keycol);
	if(start!=NULL) free(start);

	exit(0);
	}
