#define thisprog "xe-getsequence1"
#define TITLE_STRING thisprog" v 1.6: 25.September.2012 [JRH]"
#define MAXLINELEN 1000
#define MAXWORDS 64

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
<TAGS>database string</TAGS>
Changes:
v 1.6: 17.November.2018 [JRH]
	- update variable naming for consistency

v 1.6: 25.September.2012 [JRH]
	- bug fix: previously, 2-word sequences (eg. A B) were detected repeatedly if subsequent entries were word-2 (eg A B B B )
	- this was because the contiguous-sequence section became active if either x==0 or y==0 - now correctly happens only if y==0

v 1.5: 11.June.2012 [JRH]
	- bug fix: NULL characters now copied directly from argv[] to make sure each label is properly terminated

v 1.4: 24.November.2011 [JRH]
 	- report of total sequences now goes to standard output
	- outputting to stderr meant insertion into random parts of the file when all output was directed to a file using &>

v 1.3: 16.November.2011 [JRH]
	- bugfix: now returns an error if only one element of a sequence is specified
	- all completed sequences are now preceeded by a blank line, unless contiguous with the preceeding sequence
		- previously no blanks were generated if the first and last elements of the sequence were distinct
	- now reports to stderr the total number of sequences detected

v 1.2: 26.July.2011 [JRH]
	- bugfix: "contains" option will now correctly trigger "back2back" mode if
	           either the first or last words are substrings of the other
	- insert a blank line before sequences that are not contiguous with the previous sequence (shared first/last member)
	- this will allow identification of discrete sequence blocks
*/


/* external functions start */
/* external functions end */

int main (int argc, char *argv[]) {
	/* general variables */
	char *infile=NULL,line[MAXLINELEN],templine[MAXLINELEN],word[256],*matchstring=NULL,*pline,*pcol;
	long ii,jj,kk,nn;
	int v,w,x,y,z,col,colmatch,sizeofchar=sizeof(char);
	float a,b,c,d;
	double aa,bb,cc,dd;
	FILE *fpin,*fpout;
	/* program-specific variables */
	char *labelstring=NULL,*label[MAXWORDS];
	char buffer[MAXLINELEN*MAXWORDS];
	int lenlabelstring=0,nlabels=0,nbuffer=0,prevfail=1,nsequences=0;
	/* arguments */
	char setmatchword[MAXLINELEN]="exact\0";
	int setcol=1,setmatch=1;

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<5) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Extract lines of data in which a word-sequence is found in a given column \n");
		fprintf(stderr,"That is, a column on line must match the correct item in the sequence\n");
		fprintf(stderr,"Start & end words can be the same, allowing contiguous sequences\n");
		fprintf(stderr,"Individual elements in contiguous sequences will only be output once.\n");
		fprintf(stderr,"Non-contiguous sequences will be separated by a space\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [input] [col] [mode] [list]\n",thisprog);
		fprintf(stderr,"	[input]: file name or \"stdin\"\n");
		fprintf(stderr,"	[col]: column containing the words of interest\n");
		fprintf(stderr,"	[mode]: match mode: \"contains\" or \"exact\"\n");
		fprintf(stderr,"	[list]: a sequence of up to %d words to match\n",MAXWORDS);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	Lines for which the set column matches the sequence\n");
		fprintf(stderr,"	Last output line reads \"# total_sequences: \"[total]\n");
		fprintf(stderr,"EXAMPLE: if this is the input file...\n");
		fprintf(stderr,"	1	dog\n");
		fprintf(stderr,"	2	cat\n");
		fprintf(stderr,"	3	pig\n");
		fprintf(stderr,"	4	cow\n");
		fprintf(stderr,"	Then executing this command...\n");
		fprintf(stderr,"		%s data.txt 2 exact  cat pig cow\n",thisprog);
		fprintf(stderr,"	Will produce this output...\n");
		fprintf(stderr,"	2	cat\n");
		fprintf(stderr,"	3	pig\n");
		fprintf(stderr,"	4	cow\n");
		fprintf(stderr,"	# total_sequences: 1\n");
		fprintf(stderr,"NOTE:\n");
		fprintf(stderr,"	Normal redirection to file excludes the total_sequences report\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/* READ THE FILENAME */
	infile=argv[1];

	/* READ THE INPUT COLUMN */
	if(sscanf(argv[2],"%d",&setcol)!=1) {fprintf(stderr,"\n--- Error[%s]: second argument (%s) must be numerical\n\n",thisprog,argv[2]);exit(1);}

	/* READ THE MATCH MODE */
	sprintf(setmatchword,"%s",argv[3]);
	if(strcmp(setmatchword,"exact")==0) setmatch=1;
	else if(strcmp(setmatchword,"contains")==0) setmatch=2;
	else {fprintf(stderr,"\n--- Error[%s]: \"%s\" is not a valid option for [match]. Must be \"exact\" or \"contains\"\n\n",thisprog,setmatchword);exit(1);}


	/* BUILD A SEQUENCE OF WORDS TO MATCH (LABELS) - STORE IN SINGLE LONG STRING ACCESSED BY POINTERS */
	for(ii=4;ii<argc;ii++) {
		/* allocate memory for expanded labelstring */
		x=strlen(argv[ii]);
		labelstring=(char *)realloc(labelstring,((lenlabelstring+x+2)*sizeofchar));
		if(labelstring==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		/* set pointer to start position (currently, the end of the labels string) */
		label[nlabels]=labelstring+lenlabelstring;
		/* add new word (including terminating NULL) to end of labelstring - do not overwrite preceding NULL */
		for(kk=0;kk<=x;kk++) labelstring[lenlabelstring+kk]=*(argv[ii]+kk);
		/* update length */
		lenlabelstring+=(x+1);
		/* incriment nlabels with check */
		nlabels++; if(nlabels>=MAXWORDS) {fprintf(stderr,"\n--- Error[%s]: maximum number of labels (%d) exceeded\n\n",thisprog,MAXWORDS);exit(1);};
	}

	if(nlabels<2) {fprintf(stderr,"\n--- Error[%s]: list must contain at least 2 items\n\n",thisprog,argv[2]);exit(1);}

	/* READ THE DATA */
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
	nn=0;
	prevfail=1;
	while(fgets(line,MAXLINELEN,fpin)!=NULL) {
		if(line[0]=='#') continue;
		nn++;
		strcpy(buffer+(nbuffer*MAXLINELEN),line);
		strcpy(templine,line);
		pline=line;
		x=y=1; // if a match has been found x or y will be zero, depending on if it matches the next item or the first item

		/* SCAN THE LINE AND ANALYZE THE COLUMN OF INTEREST */
		for(col=1;(pcol=strtok(pline," ,\t\n"))!=NULL;col++) {
			pline=NULL;
			if(col==setcol) {
				/* if looking for exact match - 0 denotes success */
				if(setmatch==1) {
					x=strcmp(pcol,label[nbuffer]);  // does the word match the next-in-list?  (0=yes)
					y=strcmp(pcol,label[0]);	// does the word match first-in-list? (0=yes)
				}
				/* if looking to match a sub-string - 0 denotes success */
				if(setmatch==2) {
					x=0;if(x=strstr(pcol,label[nbuffer])==NULL) x=1; // does the word match the next-in-list? (1=no)
					y=0;if(y=strstr(pcol,label[0])==NULL) y=1;  	 // does the word match first-in-list? (1=no)
				}
				/* if word matches next item in list, incriment buffer counter */
				if(x==0) nbuffer++;
				/* otherwise, if a premature re-start of the sequence, copy to position zero and set buffer counter to one */
				else if(y==0) { strcpy(buffer+0,templine); nbuffer=1; prevfail=1;}
				/* otherwise, if a sequence has started (nbuffer>0) and this is a non-match, reset and remember last started sequence failed */
				else if(nbuffer>0) { nbuffer=0; prevfail=1;}
				break; /* don't bother scanning the rest of the line */
			}
		}

		/* IF ALL THE ELEMENTS OF THE SEQUENCE ARE FOUND, OUTPUT THE BUFFER */
		if(nbuffer==nlabels)	{
			nsequences++;
			/* if start- and end-words are the same, y will be zero */
			if(y==0) {
				if(prevfail==1) printf("\n%s",buffer+(0*MAXLINELEN));
				for(ii=1;ii<nlabels;ii++) printf("%s",buffer+(ii*MAXLINELEN));
				strcpy(buffer+0,templine);
				nbuffer=1;
				prevfail=0;
			}
			else {
				printf("\n");
				for(ii=0;ii<nlabels;ii++) printf("%s",buffer+(ii*MAXLINELEN));
				nbuffer=0;
				prevfail=0;
			}
		}
	}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);

	printf("# total_sequences: %d\n",nsequences);

	free(labelstring);
	exit(0);
	}
