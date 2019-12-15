#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define thisprog "xe-strxmlparse1"
#define TITLE_STRING thisprog" v 6: 10.October.2018 [JRH]"
#define MAXWORDLEN 256

/*
<TAGS>string</TAGS>

v 6: 18.January.2019 [JRH]
	- bugfix: avoid string-copies and additional memory allocation - shift indices instead

v 6: 14.October.2018 [JRH]
	- combine ability to detect multiple blocks with nested-key detection
	- use strdup instead of strcpy for index1incing

v 5: 10.October.2018 [JRH]
	- allow detection of multiple blocks matching a particular key
	- use comma-delimited sets of keys
	- include option to include tags in output

v 5: 21.May.2016 [JRH]
	- switch to new long-int version of strstr1

v 1: 11.May.2016 [JRH]
	- replace (unreliable) everything-concatenation code with "tried and true" xf_strcat function

v 1: 28.March.2016 [JRH]
	- removed trailing NULL from sprint commands
	- make output stdout instead of stderr

v 1: 17.November.2014 [JRH]
	- first version
*/


/* external functions start */
char *xf_lineread1(char *line, long *maxlinelen, FILE *fpin);
long *xf_lineparse2(char *line,char *delimiters, long *nwords);
char *xf_strcat1(char *string1,char *string2,char *delimiter);
char *xf_strncat1(char *string1,char *string2,size_t nn,char *delimiter);
long xf_strstr1(char *haystack,char *needle,int setcase, int firstlast);
/* external functions end */


int main (int argc, char *argv[]) {

	/* general variables */
	char *line=NULL;
	long ii,jj,kk,nn;
	int v,w,x,y,z;
	float a,b,c,d;
	double aa,bb,cc,dd,ee;
	FILE *fpin,*fpout;

	/* program-specific variables */
	char *everything=NULL;
	char key1[MAXWORDLEN],key2[MAXWORDLEN];
	long *keyindex=NULL,maxlinelen=256,nkeys,keysfound,index1,index2,start,stop,lentot,lenblock,index1inc;

	/* arguments */
	char *infile=NULL,*setkeys=NULL,setok=0;

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<3) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Parse XML input - extracts content between key tags\n");
		fprintf(stderr,"Assumes values are bounded by <[key]> and </[key]>\n");
		fprintf(stderr,"	- example: <name>John Smith</name>\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [input] [key] [options]\n",thisprog);
		fprintf(stderr,"	[input]: file name or \"stdin\" containing XML content\n");
		fprintf(stderr,"	[key]: XML key (or nested list) to extract\n");
		fprintf(stderr,"		- specify nesting using a CSV list of keys\n");
		fprintf(stderr,"		- do not include the brackets as these will be added\n");
		fprintf(stderr,"		- multiple matches allowed\n");
		fprintf(stderr,"VALID OPTIONS: defaults in []\n");
		fprintf(stderr,"	-ok: output key tags for matching blocks (0=NO 1=YES) [%d]\n",setok);
		fprintf(stderr,"		- for lowest nest-level only\n");
		fprintf(stderr,"		- allows output to be further parsed\n");
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s data.xml group1,name\n",thisprog);
		fprintf(stderr,"\n");
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	all content between the each match for each listed key(s)\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}


	/********************************************************************************
	READ THE FILENAME, KEYS, AND OPTIONAL ARGUMENTS
	********************************************************************************/
	infile= argv[1];
	setkeys= argv[2];
	for(ii=3;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-ok")==0)  setok= atoi(argv[++ii]);
			else {fprintf(stderr,"\n*** %s [ERROR: invalid command line argument \"%s\"]\n\n",thisprog,argv[ii]); exit(1);}
	}}
	/* check validity of options */
	if(setok!=0 && setok!=1) { fprintf(stderr,"\n--- Error [%s]: invalid -out [%d] must be 0 or 1\n\n",thisprog,setok);exit(1);}
	/* parse the key list with keyindex */
	keyindex= xf_lineparse2(setkeys,",",&nkeys);

	/********************************************************************************
	READ THE INPUT AND BUILD "EVERYTHING" (A SINGLE STRING HOLDING THE ENTIRE INPUT)
	********************************************************************************/
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
	while((line=xf_lineread1(line,&maxlinelen,fpin))!=NULL) {
		if(maxlinelen==-1)  {fprintf(stderr,"\n--- Error[%s]: readline function encountered insufficient memory\n\n",thisprog);exit(1);}
		/* concatenate line to everything */
		everything= xf_strcat1(everything,line,"");
	}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);
	lentot= strlen(everything);
	//TEST: printf("everything=***%s***\nlen=%ld\n",everything,lentot);


	/********************************************************************************
	SCAN FOR THE XML BLOCKS
	********************************************************************************/
	index1= 0;
	while(index1<lentot){
		start=stop= -1;
		index1inc= 0;
		index2= index1;
		/* drill down into nested keys by changing start-index (index2) and using NULLs to limit scans */
		for(kk=0;kk<nkeys;kk++) {
			snprintf(key1,MAXWORDLEN,"<%s>",setkeys+keyindex[kk]);
			snprintf(key2,MAXWORDLEN,"</%s>",setkeys+keyindex[kk]);
			start= xf_strstr1((everything+index2),key1,1,1);
			stop=  xf_strstr1((everything+index2),key2,1,1);
			/* stop search if key not found */
			if(start<0||stop<0||(stop-start)<1) { start= stop= -1; break; }
			/* adjust to include/exclude key-tags in output */
			if(setok==0) start+= strlen(key1);
			if(setok==1) stop+= strlen(key2);
			/* if this is the first key, index1inc is just past the null about to be inserted in everything */
			if(kk==0) {
				index1inc= index2+stop+1;
				if(setok==0) index1inc+= strlen(key1);
			}
			/* insert a null at the end of the top-level block */
			everything[index2+stop]='\0';
			/* update the new start-position - only scan for deeper blocks  within current block */
			index2+= start;
			if(index2>=lentot) break;
		}
		if(kk<nkeys) break;
		if(index2<lentot) printf("%s\n",(everything+index2));
		if(index2>=lentot) break;
		index1+= index1inc;
	}

END:
	/* CLEANUP AND EXIT */
	if(line!=NULL) free(line);
	if(everything!=NULL) free(everything);
	if(keyindex!=NULL) free(keyindex);
	exit(0);
}
