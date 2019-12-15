#define thisprog "xe-math_round1"
#define TITLE_STRING thisprog"v 12: 15.July.2016 [JRH]"

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

/*
<TAGS>math </TAGS>

v 12: 15.July.2016 [JRH]
	- enable -b "0" to mean "no rounding at all"

v 12: 4.November.2015 [JRH]
	- fixed rounding for negative numbers by function xf_round1_2 to avoid over-reresentation of zero

v 11: 17.February.2015 [JRH]
	- enabled operation on all columns (-cy -1)

v 10: 16.June.2014 [JRH]
	- fixed error in order of precision,value arguments for fprintf - error was masked by compiler ability to interpret second argument as the precision if it was of type int
	- speed improvement: remove unnecessary cast of decimals to int at output stage

v 9: 1.April.2014 [JRH]
	- don't pre-allocate memory for line - unnecessary (let xf_lineread function handle it)
	- omit references to templine (unusued)

v 8: 6.April.2013 [JRH]
	- now uses xf_lineread function to deal with lines of unknown length

v 7: 19.November.2012 [JRH]
	- modified to use function xf_round1_d

v 6: 25.September.2012 [JRH]
	- improved output to eliminate extra tab at end of line

v 5: 13.July.2012 [JRH]
	- bugfix: improved rounding of negative numbers - avoids numbers either side of zero being averaged to the same number
		- subtract base from number before dividing by base
		- add a tiny number to all values (0.000000001) to ensure accurate results

v 4: 8.May.2012 [JRH]
	- added option to round down to nearest number instead of rounding up for numbers > mid-way between two categories

v 3: 30.April.2012 [JRH]
 	- bugfix - did not previously round to numbers >1 properly
 */

/* external functions start */
char *xf_lineread1(char *line, long *maxlinelen, FILE *fpin);
double xf_round1_d(double input, double base, int down);
/* external functions end */


int main(int argc, char *argv[]) {
	/******************************************************************************
	Initialise variables
	******************************************************************************/
	char infile[256],outfile[256],*line=NULL,*templine=NULL,word[256],*pline,*pcol;
	int x,y,z,col,decimals;
	long i,j,k,maxlinelen=0;
	float a,b,c,*data=NULL,mean,sd,var;
	double aa,bb,cc,dd,resultd[64];
	FILE *fpin;
	size_t sizeofchar=sizeof(char);
	/* arguments */
	int dvcol=1,setdown=0,setoriginal=0;;
	double setbase=1.0;

	/******************************************************************************
	IF ONLY ONE ARGUMENT (EXECUTABLE'S NAME) PRINT INSTRUCTIONS
	******************************************************************************/
	if(argc<2) 	{
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Round a column of data to the nearest base (0.1, 15, 230, etc.)\n");
		fprintf(stderr,"Non-numeric fields will not be modified\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr," 	%s [infile|stdin] [arguments]\n",thisprog);
		fprintf(stderr,"Valid arguments - defaults in []:\n");
		fprintf(stderr,"	-cy column containing data (0=none, -1=all)z[%d]\n",dvcol);
		fprintf(stderr,"	-b base for rounding to the nearest [%.2f]\n",setbase);
		fprintf(stderr,"	-d round down instead? (0=NO, 1=YES) [%d]]\n",setdown);
		fprintf(stderr,"	-o output original values as well (0=NO, 1=YES) [%d]\n",setoriginal);
		fprintf(stderr,"Examples:\n");
		fprintf(stderr,"	cat datafile.txt | %s stdin -cy 1 -b 0.001\n",thisprog);
		fprintf(stderr,"	%s datafile.txt -cy 2 -b 120\n",thisprog);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	sprintf(infile,"%s",argv[1]);
	for(i=2;i<argc;i++) {
		if( *(argv[i]+0) == '-') {
			if((i+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[i]); exit(1);}
			else if(strcmp(argv[i],"-cy")==0) 	{ dvcol=atoi(argv[i+1]); i++;}
			else if(strcmp(argv[i],"-b")==0) 	{ setbase=atof(argv[i+1]); i++;}
			else if(strcmp(argv[i],"-d")==0) 	{ setdown=atoi(argv[i+1]); i++;}
			else if(strcmp(argv[i],"-o")==0) 	{ setoriginal=atoi(argv[i+1]); i++;}
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[i]); exit(1);}
	}}

	if(setdown!=0&&setdown!=1) {fprintf(stderr,"\n--- Error[%s]: invalid -d option (%d) - must be 0 or 1 \n\n",thisprog,setdown);exit(1);}
	if(setoriginal!=0&&setoriginal!=1) {fprintf(stderr,"\n--- Error[%s]: invalid -o option (%d) - must be 0 or 1 \n\n",thisprog,setoriginal);exit(1);}

	decimals=(int)log10(1.0/setbase);
	if(setbase>=1.0) decimals=0;

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
			if(col==dvcol||dvcol==-1) {
				if(setoriginal==0) {
					if(sscanf(pcol,"%lf",&aa)!=1 || !isfinite(aa)) printf("%s",pcol);
					else printf("%.*f",decimals,xf_round1_d(aa,setbase,setdown));
				}
				else {
					if(sscanf(pcol,"%lf",&aa)!=1 || !isfinite(aa)) printf("%s	%s",pcol,pcol);
					else printf("%s	%.*f",pcol,decimals,xf_round1_d(aa,setbase,setdown));
				}
			}
			else printf("%s",pcol);
		}
		printf("\n");
	}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);

	if(line!=NULL) free(line);
	exit(0);
}
