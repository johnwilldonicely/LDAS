#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define thisprog "xe-dbmatch1"
#define TITLE_STRING thisprog" v 5: 15.January.2019 [JRH]"

/*
<TAGS>file database string</TAGS>

v 5: 6.September.2019 [JRH]
	- show error if requested XML section is not found
v 5: 15.January.2019 [JRH]
	- deal with output-column that doesn't exist on a given line
v 5: 3.November.2018 [JRH]
	- bugfix: -cn 1 option - keycolin was being reset to -1
	- bugfix: if line had too few columns, match could be against first word on the line
v 5: 18.October.2016 [JRH]
	- add non-match options
v 5: 28.March.2016 [JRH]
	- allow prog to only include an XML block
v 5: 11.January.2016 [JRH]
	- bugfix: when specifying column numbers (instead of labels) by setting -cn 1, it is now assumed there is no header line to omit
v 5: 6.July.2015 [JRH]
	- ignore blank lines (causes segmentation fault) and lines beginning with "#"
v 4: 22.June.2015 [JRH]
	- bugfix: only detect output-column if output column was actually defined
	- add ability to define a list of matches - any one of which will satisfy a match
v 3: 14.June.2015 [JRH]
	- added option to output a specific column
	- added value-matching option contains|exact (-m 1|2)
v 2: 1.April.2014 [JRH]
	- don't pre-allocate memory for line - unnecessary (let xf_lineread function handle it)
	- bugfix: allocate extra byte of memory for line & templine
*/

/* external functions start */
char *xf_lineread1(char *line, long *maxlinelen, FILE *fpin);
long *xf_lineparse1(char *line, long *nwords);
long *xf_lineparse2(char *line,char *delimiters, long *nwords);
char *xf_strescape1(char *line);
/* external functions end */

int main (int argc, char *argv[]) {
	/* general variables */
	char *line=NULL,*templine=NULL,word[256],message[1000];
	long ii,jj,kk,nn,nchars=0,maxlinelen=0;
	int v,w,x,y,z;
	float a,b,c,d;
	double aa,bb,cc,dd,ee,result_d[64];
	FILE *fpin,*fpout;
	size_t iii,jjj,kkk,nnn,mmm,sizeofchar=sizeof(char);

	/* program-specific variables */
	char xmlstart[256],xmlstop[256];
	char *words=NULL;
	int *iword=NULL,xmlgood=1,xmlfound=0;
	long *start=NULL,*vstart=NULL,nwords=0,nvalues=0,prevmaxlinelen,keycolin,keycolout;
	/* arguments */
	char *infile,*setxml=NULL,*setkeyin=NULL,*setkeyout=NULL,*setvalues;
	int setcolnum=0,setdelimiters=0,setomithead=0,setmatch=2;
	char delimiters[256],delimout[4];


	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<4) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Extract database lines if a column matches/non-matches a pattern\n");
		fprintf(stderr,"- assumes database file in which first line contains column labels\n");
		fprintf(stderr,"- can also be used for files with no column labels (see -cn option)\n");
		fprintf(stderr,"- blank lines and lines beginning with \"#\" will be ignored\n");
		fprintf(stderr,"- an XML tag can be specified defining a section contiaining the data\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [input] [column] [pattern]\n",thisprog);
		fprintf(stderr,"	[input]: file name or \"stdin\"\n");
		fprintf(stderr,"	[column]: column label (or number) containing value to match\n");
		fprintf(stderr,"	[patterns]: comma-separated patterns to detect in column (see -m)\n");
		fprintf(stderr,"			- at least one pattern must match \n");
		fprintf(stderr,"VALID OPTIONS:\n");
		fprintf(stderr,"	[-xml]: XML section containing header-line and data [%s]\n",setxml);
		fprintf(stderr,"	[-m]: pattern match mode [%d]\n",setmatch);
		fprintf(stderr,"		 1= contains at least one pattern\n");
		fprintf(stderr,"		 2= exact match with at least one pattern\n");
		fprintf(stderr,"		-1= contains none of the patterns\n");
		fprintf(stderr,"		-2= exact match with none of the patterns\n");
		fprintf(stderr,"	[-cn]: treat [column] as column-number (0=NO, 1=YES) [%d]\n",setcolnum);
		fprintf(stderr,"		- if set, assumes no header line\n");
		fprintf(stderr,"	[-o]: omit outputting header-line (0=NO, 1=YES) [%d]\n",setomithead);
		fprintf(stderr,"	[-oc]: output column (see also -cn) [default=all, \"\\n\" if missing]\n");
		fprintf(stderr,"	[-d]: characters to use as column-delimiters [unset]\n");
		fprintf(stderr,"		if unset:\n");
		fprintf(stderr,"		- white-space (blanks,tabs) are used\n");
		fprintf(stderr,"		- multiple consecutive delims are treated as one\n");
		fprintf(stderr,"		- suitable for reading files without \"empty\" columns\n");
		fprintf(stderr,"		if set:\n");
		fprintf(stderr,"		- behaviour is similar to the linux \"cut\" command\n");
		fprintf(stderr,"		- any matching delim in the input marks a new column\n");
		fprintf(stderr,"		- multiple consecutive delims signify \"empty\" columns\n");
		fprintf(stderr,"		- suitable for reading CSV files, for example\n");
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s table_cells.txt subject 02 -oc DOB -cn 0\n",thisprog);
		fprintf(stderr,"	%s table_cells.txt 1 dog,cat -oc 4 -cn 1\n",thisprog);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}


	/********************************************************************************
	READ THE FILENAME AND OPTIONAL ARGUMENTS
	********************************************************************************/
	infile=argv[1];
	setkeyin=argv[2];
	setvalues=argv[3];
	for(ii=4;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-m")==0) 	setmatch=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-cn")==0) 	setcolnum=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-o")==0) 	setomithead=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-oc")==0) 	setkeyout=argv[++ii];
			else if(strcmp(argv[ii],"-xml")==0) 	setxml=argv[++ii];
			else if(strcmp(argv[ii],"-d")==0) 	{ setdelimiters=1; sprintf(delimiters,"%s",xf_strescape1(argv[++ii])); }
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
	}}
	if(setcolnum!=0&&setcolnum!=1) {fprintf(stderr,"\n--- Error[%s]: invalid -n (%d) : must be 0 or 1\n\n",thisprog,setcolnum); exit(1);}
	if(setmatch!=1&&setmatch!=2&&setmatch!=-1&&setmatch!=-2) {fprintf(stderr,"\n--- Error[%s]: invalid -m (%d) : must be -1,1 -2, or 2\n\n",thisprog,setmatch); exit(1);}
	if(setdelimiters==0) sprintf(delimout,"\t");
	else if(setdelimiters==1) sprintf(delimout,"%c",delimiters[0]);

	keycolin= keycolout= -1;

	/* BREAK MATCH VALUES INTO SEPARATE COMPONENTS */
	vstart= xf_lineparse2(setvalues,",",&nvalues);

// sample code for converting string to upper-case
//char *ptr = setvalue; for(ptr=setvalue;*ptr;ptr++) if (*ptr >= 97 && *ptr <= 122) *ptr-=32;
	if(setcolnum==1) {
		keycolin= atoi(setkeyin)-1;
		if(keycolin<0) {fprintf(stderr,"\n--- Error[%s]: -cn converts %s to a number <0 \n\n",thisprog,setkeyin);exit(1);}
		if(setkeyout!=NULL) {
			keycolout= atoi(setkeyout)-1;
			if(keycolout<0) {fprintf(stderr,"\n--- Error[%s]: -cn converts %s to a number <0 \n\n",thisprog,setkeyout);exit(1);}
		}
	}

	/********************************************************************************
	MAKE XML START-STOP FLAGS IF REQUIRED - SET STARTING OUTPUT FLAG TO 0
	********************************************************************************/
	if(setxml!=NULL) {
		xmlgood=0;
		xmlfound=0;
		snprintf(xmlstart,256,"<%s>",setxml);
		snprintf(xmlstop,256,"</%s>",setxml);
	}

	/********************************************************************************
	READ THE DATABASE FILE
	********************************************************************************/
	nn=0;
	prevmaxlinelen=maxlinelen;

	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}

	while((line=xf_lineread1(line,&maxlinelen,fpin))!=NULL) {

		if(line==NULL)  {fprintf(stderr,"\n--- Error[%s]: readline function encountered insufficient memory\n\n",thisprog);exit(1);}

		/* MAKE A TEMPORARY COPY OF THE LINE - ALLOCATE ADDITIONAL MEMORY IF LONGEST LINE LENGTH HAS INCREASED */
		if(setkeyout==NULL) {
			if(maxlinelen>prevmaxlinelen) {
					if((templine=(char *)realloc(templine,(maxlinelen+1)*sizeofchar))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
			}
			strcpy(templine,line);
		}
		prevmaxlinelen=maxlinelen;

		/* SKIP LINES OUTSIDE XML SECTION IF REQUIRED */
		if(setxml!=NULL) {
			if(strstr(line,xmlstop)!=NULL)       { xmlgood=0; continue; }
			else if(strstr(line,xmlstart)!=NULL) { xmlgood=1; xmlfound=1; continue; }
			else if(xmlgood==0) continue;
		}
		//printf("%ld %d %s",n,xmlgood,line); continue;

		/* BREAK UP THE ORIGINAL LINE USING NULL CHARACTERS AT DELIMITERS, AND FIND INDICES TO THE START OF EACH "WORD" */
		if(setdelimiters==0) start= xf_lineparse1(line,&nwords);
		else start= xf_lineparse2(line,delimiters,&nwords);

		/* SKIP BLANK LINES AND LINES BEGINNING WITH "#" */
		if(nwords<1 || line[0]=='#') continue;

		/* FOR FIRST GOOD LINE, DETECT THE INPUT KEY-COLUMN AND OUTPUT KEY-COLUMN - EXIT WITH ERROR IF KEY IS NOT FOUND */
		if(nn==0 && setcolnum==0) {
			keycolin=-1;
			keycolout=-1;
			for(ii=0;ii<nwords;ii++) {
				if(strcmp(setkeyin,line+start[ii])==0) keycolin=ii;
				if(setkeyout!=NULL){
					if(strcmp(setkeyout,line+start[ii])==0 && setkeyout!=NULL) keycolout=ii;
				}
			}
			if(keycolin<0) {fprintf(stderr,"\n--- Error[%s]: no column labelled \"%s\" in %s\n\n",thisprog,setkeyin,infile);exit(1);}
			if(keycolout<0 && setkeyout!=NULL) {fprintf(stderr,"\n--- Error[%s]: no column labelled \"%s\" in %s\n\n",thisprog,setkeyout,infile);exit(1);}
			if(setomithead==0) {
				if(setkeyout==NULL) printf("%s",templine);
				else printf("%s\n",line+start[keycolout]);
			}
		}

		/* OTHERWISE IF NOT THE FIRST LINE, OR COLUMNS ARE SPECIFIED BY NUMBER, TEST FOR VALUE IN THE KEY-COLUMN */
		else {
			//TEST:	fprintf(stderr,"keycolin:%ld nwords:%ld result:%s\n",keycolin,nwords,line+start[keycolin]); continue;
			if(keycolin>=nwords) continue;

			if(setmatch==1) { // PARTIAL MATCH
				for(ii=0;ii<nvalues;ii++) {
					if(strstr(line+start[keycolin],setvalues+vstart[ii])!=NULL) {
						if(setkeyout==NULL) printf("%s",templine);
						else if(nwords>keycolout) printf("%s\n",line+start[keycolout]);
						else printf("\n");
						continue;
			}}}
			else if(setmatch==2) { // EXACT MATCH
				for(ii=0;ii<nvalues;ii++) {
					if(strcmp(line+start[keycolin],setvalues+vstart[ii])==0) {
						if(setkeyout==NULL) printf("%s",templine);
						else if(nwords>keycolout) printf("%s\n",line+start[keycolout]);
						else printf("\n");
						continue;
			}}}
			else if(setmatch==-1) { // PARTIAL NON-MATCH
				z=0; for(ii=0;ii<nvalues;ii++) if(strstr(line+start[keycolin],setvalues+vstart[ii])!=NULL) z=1;
				if(z==0) {
					if(setkeyout==NULL) printf("%s",templine);
					else if(nwords>keycolout) printf("%s\n",line+start[keycolout]);
					else printf("\n");
					continue;
			}}
			else if(setmatch==-2) { // EXACT NON-MATCH
				z=0; for(ii=0;ii<nvalues;ii++) if(strcmp(line+start[keycolin],setvalues+vstart[ii])==0) z=1;
				if(z==0) {
					if(setkeyout==NULL) printf("%s",templine);
					else if(nwords>keycolout) printf("%s\n",line+start[keycolout]);
					else printf("\n");
					continue;
			}}
		}
		nn++;
	}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);

	if(setxml!=NULL && xmlfound==0) {fprintf(stderr,"\n--- Error[%s]: no XML section \"%s\" found in %s\n\n",thisprog,setxml,infile);exit(1);}


	if(line!=NULL) free(line);
	if(templine!=NULL) free(templine);
	if(start==NULL) free(start);
	if(vstart==NULL) free(vstart);

	exit(0);

	}
