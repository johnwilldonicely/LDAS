#define thisprog "xe-dbmatch2"
#define TITLE_STRING thisprog" 29.May.2020 [JRH]"
#define MAXDELIM 16
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/*
<TAGS>database</TAGS>

29.May.2020 [JRH]
	- add ability to set the "missing value" string

4.May.2019 [JRH]: complete reworking - currently this program is impractical and unused
	- read database first and store key-value pairs
		- allow any pair of input columns
	- read and process input line-by line
	- improved use of setdelim variable over xe-dbmatch1, though no option for "no delimiter" (awk-like behaviour) is allowed)
	- note that this program also checks that input and key-file is not corrupt - ie. number of columns always matches lines in header
	- note also that blank and comment (#) lines are skipped by default
	- this can be used to replace xs-dbmatch2 if only one value needs to be appended
*/

/* external functions start */
char *xf_lineread1(char *line, long *maxlinelen, FILE *fpin);
long *xf_lineparse1(char *line, long *nwords);
long *xf_lineparse2(char *line,char *delimiters, long *nwords);
char *xf_strescape1(char *line);
char *xf_strcat1(char *string1,char *string2,char *delimiter);
int xf_strselect1(char *input, char delimiter, long *words, long nwords);

/* external functions end */

int main (int argc, char *argv[]) {
	/* general variables */
	char *line=NULL,*pword=NULL;
	long ii,jj,kk,nn,mm,col,maxlinelen=0;
	FILE *fpin,*fpout;
	/* program-specific variables */
	char *allkeys=NULL,*allvals=NULL,delimout[4];
	long *iwords=NULL,*ikeys=NULL,*ivals=NULL,nheader=-1,nwords=0,nkeylines=0;
	long incol=-1,keycol=-1,valcol=-1;
	/* arguments */
	char *infile=NULL,*keyfile=NULL,*setcol=NULL,*setkey=NULL,*setval=NULL,*setdelim=NULL,*setmissing=NULL;
	int setskip=1;

	/********************************************************************************
	PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED
	********************************************************************************/
	if(argc<5) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Read an input and append the matching value from a keyfile\n");
		fprintf(stderr,"- match is performed on a column from the input and the keyfile\n");
		fprintf(stderr,"- the appended value is in a specified column in the keyfile\n");
		fprintf(stderr,"- if there is no matching key, a \"-\" will be appended instead\n");
		fprintf(stderr,"- matches are case-sensitive and must be exact (eg. 3 does not equal 03)\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [input] [col] [keyfile] [val]  [options]\n",thisprog);
		fprintf(stderr,"	[input]: file name or \"stdin\" with key-column to be matched\n");
		fprintf(stderr,"	[col]: input column to match with key\n");
		fprintf(stderr,"	[keyfile]: file with key- and value-columns\n");
		fprintf(stderr,"	[val]: column in keyfile containing values to append\n");
		fprintf(stderr,"VALID OPTIONS:\n");
		fprintf(stderr,"	[-k]: key-column to match in keyfile [ default=[col] ]\n");
		fprintf(stderr,"	[-m]: missing key-match placeholder to append [\"-\"]\n");
		fprintf(stderr,"	[-d]: characters to use as column-delimiters [\"\\t\"]\n");
		fprintf(stderr,"		- max %d characters permitted\n",MAXDELIM);
		fprintf(stderr,"		- the first delimiter is used for the output\n");
		fprintf(stderr,"		- any delimiter in the input marks a new column\n");
		fprintf(stderr,"EXAMPLES: add group-names to a file with only group-no. specified\n");
		fprintf(stderr,"	%s data.txt group names.txt name -d \",\"\n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	input + matching values from keyfile appended on each line\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/********************************************************************************
	READ THE FILENAME AND OPTIONAL ARGUMENTS - including comma-separated list item
	********************************************************************************/
	infile= argv[1];
	setcol= argv[2];
	keyfile= argv[3];
	setval= argv[4];

	for(ii=5;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-k")==0) setkey= argv[++ii];
			else if(strcmp(argv[ii],"-m")==0) setmissing= argv[++ii];
			else if(strcmp(argv[ii],"-d")==0) {
				if((setdelim= malloc(MAXDELIM))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
				snprintf(setdelim,MAXDELIM,"%s",xf_strescape1(argv[++ii]));
			}
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
	}}
	if(setmissing==NULL) setmissing="-";

	/* check validity of arguments */
	if(strcmp(keyfile,"stdin")==0) {fprintf(stderr,"\n--- Error[%s]: keyfile cannot be \"stdin\" - specify a file-name\n\n",thisprog);exit(1);}
	if(setkey==NULL) setkey=setcol;
	if(setdelim==NULL) {
		if((setdelim= malloc(MAXDELIM))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
		sprintf(setdelim,"\t");
	}
	sprintf(delimout,"%c",setdelim[0]);


	/********************************************************************************/
	/* STORE KEY-VALUE PAIRS */
	/********************************************************************************/
	if((fpin=fopen(keyfile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: key-file \"%s\" not found\n\n",thisprog,keyfile);exit(1);}
	nn= mm= nkeylines= 0;
	while((line= xf_lineread1(line,&maxlinelen,fpin))!=NULL) {
		if(maxlinelen==-1)  {fprintf(stderr,"\n--- Error[%s]: readline function encountered insufficient memory\n\n",thisprog);exit(1);}
		mm++; // just used for error-reporting
 		/* skip blank or commented lines */
		if(setskip==1 && (line[0]=='#'||strlen(line)<2)) continue;
		/* parse the line */
		iwords= xf_lineparse2(line,setdelim,&nwords);

		/* if it's the header-line, determine the key-column */
		if(++nn==1) {
			/* record how many columns there should be */
			nheader= nwords;
			/* determine the key-column and value-column to match */
			for(ii=0;ii<nwords;ii++) {
				if(strcmp(setkey,line+iwords[ii])==0) keycol=ii;
				if(strcmp(setval,line+iwords[ii])==0) valcol=ii;
			}

			// ??? NEXT STEP - REPLACE THE CODE ABOVE WITH A MULTI-KEY SEARCH AND STORE THE LIST OF KEYCOLS
			// ?? KEYCOLS SHOULD BE INITIALISED TO -1 - TEST AFTER SEARCH
			// /* look for matches between words and keys */
			// for(jj=0;jj<nwords;jj++) {
			// 	pword= (line+iwords[jj]);
			// 	for(kk=0;kk<nkeys;kk++) if(strcmp(key[kk],pword)==0) keycol[kk]= jj;
			// }


			if(keycol==-1) {fprintf(stderr,"\n--- Error[%s]: no key-column matching \"%s\" found in %s\n\n",thisprog,setkey,keyfile);exit(1);}
			if(valcol==-1) {fprintf(stderr,"\n--- Error[%s]: no value-column matching \"%s\" found in %s\n\n",thisprog,setval,keyfile);exit(1);}
			continue;
		}

		/* for all other lines, store the key and value, if the appropriate columns are present */
		if(nwords!=nheader) {fprintf(stderr,"\n--- Error[%s]: corrupt key line %ld - number of columns (%ld) does not match number of headers (%ld) in %s\n\n",thisprog,mm,nwords,nheader,keyfile);exit(1);}
		allkeys= xf_strcat1(allkeys,line+iwords[keycol],setdelim);
		allvals= xf_strcat1(allvals,line+iwords[valcol],setdelim);

		// REPLACE THE ABOVE CODE WITH STORAGE OF A SELECTED VERSION OF EACH LINE AS IN THIS EXAMPLE
			// char input[]= "zero,one,two,three,four,five,siz,seven\n";
			// long ii,keep[]= {1,3,5};
			// ii= xf_strselect1(input,',',keep,3);
			// printf("input=%s\n",input);

		// NOTE HOWEVER! WE NEED A SPECIAL DELIMITER TO GO BETWEEN EACH SET OF VALUES!
		// BEST SOLUTION:
		// - process allkeys as above, and parse afterwards as always
		// - unparse the line (replace all nulls before the end with setdelim)
		// - use strselect to reduce the line
		// - calculate required storage - do realloc()
		// - add the "selected" line to the allvals array


		nkeylines++;
	}
	fclose(fpin);
	if(nkeylines==0) {fprintf(stderr,"\n--- Error[%s]: no key-value pairs in %s\n\n",thisprog,keyfile);exit(1);}
	//TEST: printf("nkeylines=%ld\nallkeys=%s\nallvals=%s\n",nkeylines,allkeys,allvals);



	/********************************************************************************/
	/* PARSE THE MULTI-LINE KEY AND VALUE STRINGS
	/********************************************************************************/
	ikeys= xf_lineparse2(allkeys,setdelim,&nn);
	ivals= xf_lineparse2(allvals,setdelim,&mm);
	//TEST:for(ii=0;ii<nkeylines;ii++) printf("%ld  %s  %s\n",ii,allkeys+ikeys[ii],allvals+ivals[ii]);



	/********************************************************************************/
	/* PROCESS THE INPUT & APPEND APPROPRIATE VALUES (OR DUMMY-VALUE) TO EACH LINE */
	/********************************************************************************/
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
	nn= mm= 0;
	while((line=xf_lineread1(line,&maxlinelen,fpin))!=NULL) {
		if(maxlinelen==-1)  {fprintf(stderr,"\n--- Error[%s]: readline function encountered insufficient memory\n\n",thisprog);exit(1);}
		mm++; // just used for error-reporting
		/* skip blank or commented lines */
		if(setskip==1 && (line[0]=='#'||strlen(line)<2)) continue;
		/* parse the line */
		iwords= xf_lineparse2(line,setdelim,&nwords);
		/* if it's the header-line, determine the key-column */
		if(++nn==1) {
			/* record how many columns there should be */
			nheader= nwords;
			/* determine the input-column to match */
			for(ii=0;ii<nwords;ii++) if(strcmp(setcol,line+iwords[ii])==0) incol=ii;
			if(incol==-1) {fprintf(stderr,"\n--- Error[%s]: no input-column matching \"%s\" found in %s\n\n",thisprog,setcol,infile);exit(1);}
			/* output header line, including a column-header for the appended value */
			for(ii=0;ii<nwords;ii++) printf("%s%s",line+iwords[ii],setdelim); printf("%s\n",setval);
			continue;
		}

		/* for all other lines, append the appropriate value if the required input-column is present */
		if(nwords!=nheader) {fprintf(stderr,"\n--- Error[%s]: corrupt input line %ld - number of columns (%ld) does not match number of headers (%ld) in %s\n\n",thisprog,mm,nwords,nheader,infile);exit(1);}
		/* print the input line */
		for(ii=0;ii<nwords;ii++) printf("%s%s",line+iwords[ii],setdelim);
		/* get pointer to the input-column-to-match */
		pword= line+iwords[incol];
		/* find matching keys - the value for the last matching key will be the one used */
		for(kk=-1,ii=0;ii<nkeylines;ii++) if(strcmp(pword,allkeys+ikeys[ii])==0) kk=ii;
		if(kk!=-1) printf("%s\n",allvals+ivals[kk]);
		else printf("%s\n",setmissing);
	}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);


	if(line!=NULL) free(line);
	if(iwords!=NULL) free(iwords);
	exit(0);

	}
