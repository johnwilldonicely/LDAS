#define thisprog "xe-dbmatch2"
#define TITLE_STRING thisprog" 25.Jan.2021 [JRH]"
#define MAXDELIM 16
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/*
<TAGS>database</TAGS>

25.Jan.2021 [JRH]
	- bugfix: now skips empty fields in input-2 (containing values to be appended)
	 	- previously this could cause crashes

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
	char *allkeys=NULL,*allvals=NULL,*valheader=NULL,*psetval=NULL,delimout[4];
	long *iwords=NULL,*ikeys=NULL,*ivals=NULL,nheader=-1,nwords=0,nkeylines=0,linelen;
	long *isetval=NULL,*valcol=NULL,nsetval=0;
	long incol=-1,keycol=-1,xvalcol=-1;
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
		fprintf(stderr,"Read input-1 and append the corresponding values from a input-2\n");
		fprintf(stderr,"- match is performed on a column (key) from both files\n");
		fprintf(stderr,"- if there is no matching key, a \"-\" will be appended instead\n");
		fprintf(stderr,"- matches are case-sensitive & must be exact (e.g. 3 doesn't match 03)\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [in1] [col] [in2] [val]  [options]\n",thisprog);
		fprintf(stderr,"	[in1]: the file (or \"stdin\") to be added to\n");
		fprintf(stderr,"	[col]: in1 column-name holding the keys\n");
		fprintf(stderr,"	[in2]: file containing values to be appended\n");
		fprintf(stderr,"	[vals]: in2 column-names (CSV) to append\n");
		fprintf(stderr,"VALID OPTIONS:\n");
		fprintf(stderr,"	[-k]: key-column to match in in2 [ default=[col] ]\n");
		fprintf(stderr,"	[-m]: missing key-match placeholder to append [\"-\"]\n");
		fprintf(stderr,"	[-d]: characters to use as column-delimiters [\"\\t\"]\n");
		fprintf(stderr,"		- max %d characters permitted\n",MAXDELIM);
		fprintf(stderr,"		- any delimiter in the input marks a new column\n");
		fprintf(stderr,"		- the first delimiter is used for the output\n");
		fprintf(stderr,"EXAMPLES: add age and height to a file containing names and sex\n");
		fprintf(stderr,"	%s names.txt NAME stats.txt AGE,HEIGHT -d \",\"\n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	in1 + matching values from in2 appended on each line\n");
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

	/* PARSE SETVAL INTO INDIVIDUAL VALUE-WORDS, AND RESERVE MEMORY FOR ASSOCIATED COLUMNS  */
	isetval= xf_lineparse2(setval,",",&nsetval);
	if(nsetval<1) {fprintf(stderr,"\n--- Error[%s]: no value-columns defined\n\n",thisprog);exit(1);}
	valcol= realloc(valcol,nsetval*sizeof(*valcol));
	if(valcol==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
	//TEST for(ii=0;ii<nsetval;ii++) printf("%ld\t%s\tcolumn: %ld\n",ii,(setval+isetval[ii]),valcol[ii]); exit(0);

	/********************************************************************************/
	/* STORE KEY-VALUE PAIRS */
	/********************************************************************************/
	if((fpin=fopen(keyfile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: key-file \"%s\" not found\n\n",thisprog,keyfile);exit(1);}
	nn= mm= nkeylines= 0;
	while((line= xf_lineread1(line,&maxlinelen,fpin))!=NULL) {
		if(maxlinelen==-1)  {fprintf(stderr,"\n--- Error[%s]: readline function encountered insufficient memory\n\n",thisprog);exit(1);}
		linelen= strlen(line);
		mm++; // just used for error-reporting
 		if(setskip==1 && (line[0]=='#'||strlen(line)<2)) continue; // skip blank or commented lines */
		else nn++; // actual record-number
		/* parse the line */
		iwords= xf_lineparse2(line,setdelim,&nwords);
		/* if it's the header-line, determine the value-columns, and keep a value-header */
		if(nn==1) {
			nheader= nwords; // record how many columns there should be
			/* determine the key-column to match */
			for(ii=0;ii<nwords;ii++) if(strcmp(setkey,line+iwords[ii])==0) keycol=ii;
			if(keycol==-1) {fprintf(stderr,"\n--- Error[%s]: no key-column matching \"%s\" found in %s\n\n",thisprog,setkey,keyfile);exit(1);}
			/* build the array of value-columns to match */
			for(jj=0;jj<nsetval;jj++) {
				valcol[jj]=-1;
				psetval= setval+isetval[jj];
				for(ii=0;ii<nwords;ii++) if(strcmp(psetval,line+iwords[ii])==0) valcol[jj]= ii;
				if(valcol[jj]==-1) {fprintf(stderr,"\n--- Error[%s]: no value-column matching \"%s\" found in %s\n\n",thisprog,psetval,keyfile);exit(1);}
			}
			/* unparse the line (replace all nulls before the end with the primary setdelim) */
			for(ii=0;ii<linelen;ii++) if(line[ii]=='\0') line[ii]= setdelim[0];
			/* use strselect to reduce the line to the set values of interest, delimited */
			ii= xf_strselect1(line,setdelim[0],valcol,nsetval);
			/* copy this selected line to valheader for use with output */
			valheader= realloc(valheader,linelen*sizeof(*valheader));
			if(valheader==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
			strncpy(valheader,line,linelen);
			continue;
		}
		/* for all other lines, store the key and value, if the appropriate columns are present */
		if(nwords!=nheader) {fprintf(stderr,"\n--- Error[%s]: corrupt keyfile line %ld - number of columns (%ld) does not match number of headers (%ld) in %s\n\n",thisprog,mm,nwords,nheader,keyfile);exit(1);}
		/* make sure the keycol is not an empty field */
		if(strlen(line+iwords[keycol])==0) continue;
		/* build the list of keys, make it tab-delimited */
		allkeys= xf_strcat1(allkeys,line+iwords[keycol],"\n");
		/* now as for the header line, unparse the line and select the valid columns */
		for(ii=0;ii<linelen;ii++) if(line[ii]=='\0') line[ii]= setdelim[0];
		ii= xf_strselect1(line,setdelim[0],valcol,nsetval);
		/* add the "selected" line to the allvals array, using newline as the delimiter */
		allvals= xf_strcat1(allvals,line,"\n");
		/* increment the counter for the number of non-header lines in the key-file */
		nkeylines++;
	}
	fclose(fpin);
	if(nkeylines==0) {fprintf(stderr,"\n--- Error[%s]: no key-value pairs in %s\n\n",thisprog,keyfile);exit(1);}
	//TEST: printf("nkeylines=%ld\nallkeys=%s\nallvals=%s\n",nkeylines,allkeys,allvals);
	//TEST: for(jj=0;jj<nsetval;jj++) printf("valcol[%ld]= %ld\n",jj,valcol[jj]); exit(0);

	/* PARSE THE MULTI-LINE KEY AND VALUE STRINGS - REPLACE THE NEWLINES WITH NULLS */
	ikeys= xf_lineparse2(allkeys,"\n",&nn);
	ivals= xf_lineparse2(allvals,"\n",&mm);
	//TEST:for(ii=0;ii<nkeylines;ii++) printf("%ld  %s  %s\n",ii,allkeys+ikeys[ii],allvals+ivals[ii]); exit(0);


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
			for(ii=0;ii<nwords;ii++) printf("%s%s",line+iwords[ii],setdelim);
			printf("%s\n",valheader);
			continue;
		}

		/* for all other lines, append the appropriate value if the required input-column is present */
		if(nwords!=nheader) {fprintf(stderr,"\n--- Error[%s]: corrupt input line %ld - number of columns (%ld) does not match number of headers (%ld) in %s\n\n",thisprog,mm,nwords,nheader,infile);exit(1);}
		/* print the input line */
		for(ii=0;ii<nwords;ii++) printf("%s%s",line+iwords[ii],setdelim);
		/* get pointer to the input-column-to-match */
		pword= line+iwords[incol];
		/* find matching keys - the value for the last matching key will be the one used */
		kk= -1;
		for(ii=0;ii<nkeylines;ii++) if(strcmp(pword,allkeys+ikeys[ii])==0) kk=ii;
		if(kk!=-1) printf("%s\n",allvals+ivals[kk]);
		else printf("%s\n",setmissing);
	}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);


	if(line!=NULL) free(line);
	if(iwords!=NULL) free(iwords);
	if(isetval!=NULL) free(isetval);
	if(valcol!=NULL) free(valcol);
	if(valheader!=NULL) free(valheader);
	exit(0);

	}
