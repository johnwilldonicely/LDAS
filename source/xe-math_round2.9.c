#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define thisprog "xe-math_round2"
#define TITLE_STRING thisprog" v 9: 15.July.2016 [JRH]"
# define MAXSETCOLS 256

/*
<TAGS>math </TAGS>

v 9: 15.July.2016 [JRH]
	- enable -b "0" to mean "no rounding at all"

v 9: 16.June.2014 [JRH]
	- fixed error in order of precision,value arguments for fprintf - error was masked by compiler ability to interpret second argument as the precision if it was of type int

v 8: 14.April.2014 [JRH]
	- BUGFIX: actually DO need to preallocate memory for line if line is used to store list of columns to round!
		- allocated memory for variable "word" used instead of "line"
	- BUGFIX: revised rounding functions to accurately calculate for negative numbers

v 7: 1.April.2014 [JRH]
	- don't pre-allocate memory for line - unnecessary (let xf_lineread function handle it)
	- remove references to templine (unused)

v 6: 22.May.2013 [JRH]
	- fix to round-down option in function xf_round1_d

v 5: 6.April.2013 [JRH]
	- now uses xf_lineread function to deal with lines of unknown length

v 4: 19.November.2012 [JRH]
	- modified to use function xf_round1_d

v 3: 25.September.2012 [JRH]
	- improved output to eliminate extra tab at end of line

v 2: 24.September.2012 [JRH]
	- increase max line length from 1000 to 10000 to handle large tables of data

 */


/* external functions start */
char *xf_lineread1(char *line, long *maxlinelen, FILE *fpin);
double xf_round1_d(double input, double base, int down);
/* external functions end */

int main(int argc, char *argv[]) {

	/******************************************************************************
	Initialise variables
	******************************************************************************/
	char infile[256],outfile[256],*line=NULL,*templine=NULL,word[1000],*pline,*pcol,*pword;
	int x,y,z,col,decimals;
	int incol[256], ncols;
	long int i,j,k,maxlinelen=0;
	float a,b,c,*data=NULL,mean,sd,var;
	double aa,bb,cc,dd,resultd[64];
	FILE *fpin;
	size_t sizeofchar=sizeof(char);
	/* arguments */
	int dvcol=1,setdown=0;
	double setbase=1.0;

	incol[0]=0;

	/******************************************************************************
	If only one argument (executable's name) print instructions
	******************************************************************************/
	if(argc<2) 	{
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Round columns of data to the nearest base (0.1, 15, 230, etc.)\n");
		fprintf(stderr,"NOTE!! This program is not long-line safe!\n");
		fprintf(stderr,"Non-numeric fields will not be modified\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr," 	%s [infile|stdin] [arguments]\n",thisprog);
		fprintf(stderr,"Valid arguments - defaults in []:\n");
		fprintf(stderr,"	-c: columns to round, comma-separated, max 256 (default: round all)\n");
		fprintf(stderr,"	-b base for rounding to the nearest [%.2f]\n",setbase);
		fprintf(stderr,"	-d round down instead? (0=NO, 1=YES) [%d]\n",setdown);
		fprintf(stderr,"Examples:\n",setbase);
		fprintf(stderr,"	cat datafile.txt | %s stdin -c 1,2,9 -b 0.001\n",thisprog);
		fprintf(stderr,"	%s datafile.txt -b 120\n",thisprog);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/* initialize how many columns are set and the default value (0=unset) */
	ncols=0; for(i=0;i<MAXSETCOLS;i++) incol[i]=0;

	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	sprintf(infile,"%s",argv[1]);
	for(i=2;i<argc;i++) {
		if( *(argv[i]+0) == '-') {
			if((i+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[i]); exit(1);}
			else if(strcmp(argv[i],"-b")==0) 	{ setbase=atof(argv[i+1]); i++;}
			else if(strcmp(argv[i],"-d")==0) 	{ setdown=atoi(argv[i+1]); i++;}
			else if(strcmp(argv[i],"-c")==0) 	{
				strcpy(word,argv[i+1]);
				pword=word;
				for(col=1;(pcol=strtok(pword,","))!=NULL;col++) {
					pword=NULL;
					z=atoi(pcol); if(z>=MAXSETCOLS) {fprintf(stderr,"\n--- Error[%s]: specified column (%d) exceeds maximum (%d)\n\n",thisprog,z,MAXSETCOLS); exit(1);}
					else {
						incol[z]=1;
						ncols++;
			}}}
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[i]); exit(1);}
	}}

	if(setdown!=0&&setdown!=1) {fprintf(stderr,"\n--- Error[%s]: invalid -d option (%d) - must be 0 or 1 \n\n",thisprog,setdown);exit(1);}
	/* for(i=0;i<MAXSETCOLS;i++) printf("%d\n",incol[i]); */

	decimals=(int)log10(1.0/setbase);
	if(setbase>=1) decimals=0;

	/* READ THE INPUT AND GET ROUNDING! */
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}

	while((line=xf_lineread1(line,&maxlinelen,fpin))!=NULL) {
		if(maxlinelen==-1)  {fprintf(stderr,"\n--- Error[%s]: readline function encountered insufficient memory\n\n",thisprog);exit(1);}
		if(setbase==0.0) { printf("%s",line); continue; }
		if(line[0]=='#') continue;
		pline=line;
		for(col=1;(pcol=strtok(pline," ,\t\n"))!=NULL;col++) {
			pline=NULL;
			if(col>1) printf("\t");
			// if specific columns were specified, check if this is one of them
			if(ncols>0) {
				if(sscanf(pcol,"%lf",&aa)!=1 || !isfinite(aa) || incol[col]!=1) printf("%s",pcol);
				else printf("%.*f",decimals,xf_round1_d(aa,setbase,setdown));
			}
			// if no input columns were specified don't check for column-inclusion for rounding
			else {
				if(sscanf(pcol,"%lf",&aa)!=1 || !isfinite(aa)) printf("%s",pcol);
				else printf("%.*f",decimals,xf_round1_d(aa,setbase,setdown));
			}

		} /* END OF LOOP READING TOKENS FOR EACH COLUMN */
		printf("\n");
	}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);

	if(line!=NULL) free(line);
	exit(0);
}
