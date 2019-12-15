#define thisprog "xe-norm2"
#define TITLE_STRING thisprog" v 11: 14.January.2019 [JRH]"

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
/* requirements for getting process-id */
#include <sys/types.h>
#include <unistd.h>


#define MAXLINELEN 1000

/*
<TAGS>stats</TAGS>

v 11: 14.January.2019 [JRH]
	- fix errors associated with invalid normalization data - now depends on the type of normalization

v 11: 6.May.2016 [JRH]
	- make sure that in most error-cases the temporary file is removed before exit

v 11: 4.April.2016 [JRH]
	- improve auto-setting of "start" and "stop"
	- remove redundant variable colcount
	- remove option to normalize to ratio of first valid sample (too unreliable)
	- add option to normalize to ratio of mean defined by start-stop range - commonly used in science

v 11: 22.May.2015 [JRH]
	- add reverse-Fisher transform (-n 6)
	- log transform is now -n 7

v 10: 19.May.2015 [JRH]
	- add control of decimal precision for output

v 9: 1.December.2014 [JRH]
	- bugfix: setting setnorm to -1 if user defines subtractor and divisor for normalization had effect of later setting them to zero and 1 respectively
	- bugfix in invalid options message
	- additional option to specify a start/stop sample-number for normalization
		- default= 0 to N

v 8: 31.October.2014 [JRH]
	- add option to not normalize at all

v 7: 10.April.2014 [JRH]
	- temp file name now incorporates procress id to avoid overwriting don multiple calls to program, or serial piping
	- now alerts user to maximum line length in instructions

v 6: 18.May.2013 [JRH]
	- add log(10)-normalization to capabilities

v 2.5: 30.April.2013 [JRH]
	- add option to apply the Fisher transform for Pearson's R-values

v 2.4: 6.September.2011 [JRH]
	- bugfix - program exits if there is no data to normalize #include <unistd.h>


v 2.3: JRH, 10 May 2011
	- add normalization to mean
	- make sure NaN and Inf are treated as invalid values
v 2.2: JRH, 1 May 2011
	- add extra normalization option - normalize to first sample, difference or ratio
	- add transformation option (manually set value to add to and multiply by data)

/* external functions start */
void xf_fishertransform2_d(double *data, long n, int type);
void xf_fishertransformrev2_d(double *data, long n, int type);
/* external functions end */

int main(int argc, char *argv[]) {

	/* general variables */
	char infile[256],outfile[256],line[MAXLINELEN],*pline,*pcol;
	long int i,j,k,n;
	int v,w,x,y,z,col,colmatch;
	int sizeofshort=sizeof(short),sizeofdouble=sizeof(double);
	double aa,bb,cc,dd;
	FILE *fpin,*fpout;
	/* program-specific variables */
	short *datagood=NULL, setstdin=0, setinvalid=0;
	int status=0;
	long goodcount=0,N,start,stop;
	double *data=NULL,min,max,sum=0.0,sumofsq=0.0,norm1=0,norm2=1;
	/* arguments */
	int setp=-1,datacol=1,setnorm=1,settrans=0;
	double sub=0.0,div=1.0;
	double invalid=12345.6789;
	long setstart=-1,setstop=-1;

	/* IF ONLY ONE ARGUMENT (EXECUTABLE'S NAME) PRINT INSTRUCTIONS */
	if(argc==1) 	{
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"Normalize or transform data in a specified column\n");
		fprintf(stderr,"Other columns and comment lines and will be output unaltered\n");
		fprintf(stderr,"Maxmimum line length = %d characters\n",MAXLINELEN);
		fprintf(stderr,"Non-numbers in the column will be output unaltered but do not affect calculations\n");
		fprintf(stderr,"Values matching the invalid value will also be output as \"-\"\n");
		fprintf(stderr,"If input is piped to the program, input is sent to a temporary file\n");
		fprintf(stderr,"This allows reconstruction of the input, normalizing only the column of interest\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr," 	%s [input] [options]\n",thisprog);
		fprintf(stderr,"REQUIED ARGUMENTS:\n");
		fprintf(stderr,"	[input]: a filename or \"stdin\"\n");
		fprintf(stderr,"VALID OPTIONS:\n");
		fprintf(stderr,"	-cy column containing data [%d]\n",datacol);
		fprintf(stderr,"	-p output decimal precision (-1=auto (%%f), 0=auto (%%g), >0=precision) [%d]\n",setp);
		fprintf(stderr,"	-n normalization type: [%d]\n",setnorm);
		fprintf(stderr,"		-1: no normalization \n");
		fprintf(stderr,"		 0: 0-1 range\n");
		fprintf(stderr,"		 1: z-scores (uses start/stop)\n");
		fprintf(stderr,"		 2: difference from first valid sample (uses start)\n");
		fprintf(stderr,"		 3: difference from mean (uses start/stop) \n");
		fprintf(stderr,"		 4: ratio of mean (uses start/stop)\n");
		fprintf(stderr,"		 5: apply Fisher transform for Pearson's \"r\"\n");
		fprintf(stderr,"		 6: apply reverse Fisher transform for Pearson's \"r\"\n");
		fprintf(stderr,"		 7: log-transform input (log base-10)\n");
		fprintf(stderr,"	-start: start of normalization range (-n options 1-4) [%ld]\n",setstart);
		fprintf(stderr,"	-stop:  end of normalization range (-n options 1-4) [%ld]\n",setstop);
		fprintf(stderr,"		NOTE: stop = sample just AFTER the last to be included\n");
		fprintf(stderr,"		-1 = auto (first valid sample, respectively\n");
		fprintf(stderr,"			= first valid sample for start\n");
		fprintf(stderr,"			= last valid sample +1 for stop\n");
		fprintf(stderr,"	-invalid value to be ignored (unset by default)\n");
		fprintf(stderr,"	-sub define value to subtract from each datum (unset by default)\n");
		fprintf(stderr,"	-div define value to divide each datum by (unset by default)\n");
		fprintf(stderr,"		NOTE: setting -sub or -div overrides -n option\n");
		fprintf(stderr,"		NOTE: if only -sub was set, -div=1\n");
		fprintf(stderr,"		NOTE: if only -div was set, -sub=0\n");
		fprintf(stderr,"To preserve input/output line count, remove comments & blank lines first\n");
		fprintf(stderr,"Invalid values and non-normal numbers (NaN, Inf) will be padded with \"-\"\n");
		fprintf(stderr,"Examples:\n");
		fprintf(stderr,"	cat infile.txt | %s stdin -cy 2 -n 1\n",thisprog);
		fprintf(stderr,"	%s infile.txt -n 0 -cy 2 -invalid 999\n",thisprog);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/********************************************************************************/
	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	/********************************************************************************/
	sprintf(infile,"%s",argv[1]);
	for(i=2;i<argc;i++) {
		if( *(argv[i]+0) == '-') {
			if((i+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[i]); exit(1);}
			else if(strcmp(argv[i],"-cy")==0) datacol=atoi(argv[++i]);
			else if(strcmp(argv[i],"-n")==0)  setnorm=atoi(argv[++i]);
			else if(strcmp(argv[i],"-p")==0)  setp=atoi(argv[++i]);
			else if(strcmp(argv[i],"-start")==0) setstart=atol(argv[++i]);
			else if(strcmp(argv[i],"-stop")==0)  setstop=atol(argv[++i]);
			else if(strcmp(argv[i],"-sub")==0)  { sub=atof(argv[++i]);settrans=1;}
			else if(strcmp(argv[i],"-div")==0)  { div=atof(argv[++i]);settrans=1;}
			else if(strcmp(argv[i],"-invalid")==0)  { invalid=atof(argv[++i]);setinvalid=1;}
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[i]); exit(1);}
	}}

	/********************************************************************************/
	/* CHECK FOR INVALID VALUES */
	/********************************************************************************/
	if(setnorm<-1 || setnorm>7) {fprintf(stderr,"\n--- Error[%s]: -n [%d] must be between -1 and 7\n\n",thisprog,setnorm); exit(1);}
	if(setstart<-1) {fprintf(stderr,"\n--- Error[%s]: -start [%ld] must be -1 or >0\n\n",thisprog,setstart); exit(1);}
	if(setstop<-1)  {fprintf(stderr,"\n--- Error[%s]: -stop [%ld] must be -1 or >0\n\n",thisprog,setstop); exit(1);}

	// CREATE VARIABLE TO REPRESENT SPECIFICATION OF STDIN
	if(strcmp(infile,"stdin")==0) setstdin=1;

	// DEFINE OUTPUT FILE NAME (ONLY USED IF INPUT IS PIPED IN)
	sprintf(outfile,"temp_%s_%d",thisprog,getpid());
	fpout= stdout;


	/********************************************************************************/
	/* STORE DATA IN MEMORY */
	/********************************************************************************/
	if(setstdin==1) {
		fpin=stdin;
		if(setnorm!=-1) { // only open output file if data is NOT meant to simply be passed through
			if((fpout=fopen(outfile,"w"))==0) {fprintf(stderr,"\n--- Error[%s]: could not open \"%s\" for writing\n\n",thisprog,outfile);exit(1);}
	}}
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: input file \"%s\" not found\n\n",thisprog,infile);exit(1);}

	N=-1; // line counter - start at -1 so increment can go at top of read loop
	while(fgets(line,MAXLINELEN,fpin)!=NULL) {
		if(setnorm==-1){
			 printf("%s",line);
			 continue;
		}
		N++; // so, zero at first iteration of loop
		if(setstdin==1) fprintf(fpout,"%s",line);
		if(line[0]=='#') {
			data=(double *)realloc(data,(N+1)*sizeofdouble);if(data==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
			datagood=(short *)realloc(datagood,(N+1)*sizeofshort);if(datagood==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
			data[N]=invalid; datagood[N]=0; continue;
		}
		pline=line;
		for(col=1;(pcol=strtok(pline," ,\t\n"))!=NULL;col++) {
			pline=NULL;
			if(col==datacol) {
				// store every value, but nullify non-numbers or invalid values
				z=sscanf(pcol,"%lf",&aa);
				data=(double *)realloc(data,(N+1)*sizeofdouble);if(data==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
				datagood=(short *)realloc(datagood,(N+1)*sizeofshort);if(datagood==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}

				if(z!=1) {
					data[N]=invalid;
					datagood[N]=-1;
				}
				else if(!isfinite(aa) || (setinvalid==1 && aa==invalid)) {
					data[N]=invalid;
					datagood[N]=0;
				}
				else { // only do this if a non-invalid number was successfully read (z=1)
					data[N]=aa;
					datagood[N]=1;
					goodcount++;
				}
	}}}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);
	else if(setnorm!=-1) fclose(fpout);

	/* exit here if data was simply passed through */
	if(setnorm==-1) exit(0);

	/* check - was there any good data in the specified column? */
	if(goodcount<1) {fprintf(stderr,"\n--- Error[%s]: infile %s has no valid data in column %d\n\n",thisprog,infile,datacol);if(setstdin==1) remove(outfile);exit(1);}

	/* update n so this reflects the total number of lines, not the highest index number */
	N++;



	/********************************************************************************/
	/* AUTO DEFINE AND/OR CHECK START/STOP VALUES */
	/********************************************************************************/
	if(setstart==-1) {for(start=i=0;i<N;i++) if(datagood[i]==1) {start=i; break;}}
	else {
		start=setstart;
		if(start>=N) {fprintf(stderr,"\n--- Error[%s]: -start [%ld] must be less than the total number of samples [%ld]\n\n",thisprog,start,N);if(setstdin==1) remove(outfile);exit(1);}
	}
	if(setstop==-1) {for(stop=i=(N-1);i>=0;i--) if(datagood[i]==1) {stop=(i+1); break;}}
	else {
		stop=setstop;
		if(stop>N) {fprintf(stderr,"\n--- Error[%s]: -stop [%ld] exceeds total number of samples samples [%ld]\n\n",thisprog,stop,N);if(setstdin==1) remove(outfile);exit(1);}
		/* note no need to check that the stop value is valid */
	}
	if(start>=stop) {fprintf(stderr,"\n--- Error[%s]: -start [%ld] and -stop [%ld] result in an empty start-stop set [%ld-%ld]\n\n",thisprog,setstart,setstop,start,stop);if(setstdin==1) remove(outfile); exit(1);}
	// fprintf(stderr,"start=%ld	stop=%ld\n",start,stop);



	/********************************************************************************/
	/* CALCULATE CORRECTION FACTORS TO BE APPLIED TO EACH DATA POINT */
	/********************************************************************************/
	n=0; /* counter for good values in normalization zone */

	if(settrans==1) {
		norm1=sub; /* 0.0 by default if only div was defined */
		norm2=div; /* 1.0 by default if only sub was defined */
	}
	/* range 0-1 */
	else if(setnorm==0) {
		min=max= 0.0; /// just to avoid initialization warning on compile
		for(i=0;i<N;i++) if(datagood[i]==1) {min=max=data[i]; break;}
		for(i=0;i<N;i++) if(datagood[i]==1) {if(data[i]<min) min=data[i];if(data[i]>max) max=data[i]; }
		norm1=min;
		if(max>min) norm2= max-min;
		else norm2=1.0;
	}
	/* Z-score (start-stop) */
	else if(setnorm==1) {
		for(i=start;i<stop;i++) if(datagood[i]==1) {sum+=data[i]; sumofsq+=data[i]*data[i]; n++;}
		if(n==0) {fprintf(stderr,"\n--- Error[%s]: no valid data in normalization period\n\n",thisprog);if(setstdin==1) remove(outfile);exit(1);}
		norm1= sum/n;
		if(n>1) norm2= sqrt((double)((n*sumofsq-(sum*sum))/(n*(n-1)))); // norm2=standard deviation
		else norm2=1.0; // norm1=mean, norm2=stdev
	}
	/* difference from first valid sample (start) */
	else if(setnorm==2) {
		norm2=1;
		if(!isfinite(data[start])) {fprintf(stderr,"\n--- Error[%s]: normalization sample is invalid\n\n",thisprog);if(setstdin==1) remove(outfile);exit(1);}
		for(i=0;i<N;i++) if(datagood[i]==1) { norm1= data[start]; break; }
	}
	/* difference from mean (start-stop) */
	if(setnorm==3) {
		norm1=0.0; norm2=1; n=0;
		for(i=start;i<stop;i++) if(datagood[i]==1) { norm1+=data[i]; n++;}
		if(n==0) {fprintf(stderr,"\n--- Error[%s]: no valid data in normalization period\n\n",thisprog);if(setstdin==1) remove(outfile);exit(1);}
		norm1/=(double)n;
	}
	/* ratio of mean (start-stop) */
	else if(setnorm==4) {
		norm1=0.0; norm2=0; n=0;
		for(i=start;i<stop;i++) if(datagood[i]==1) { norm2+=data[i]; n++;}
		if(n==0) {fprintf(stderr,"\n--- Error[%s]: no valid data in normalization period\n\n",thisprog);if(setstdin==1) remove(outfile);exit(1);}
		norm2/=(double)n;
	}
	/* Fisher transform for correlations */
	if(setnorm==5) {
		norm1=0.0; norm2=1; n=0;
		xf_fishertransform2_d(data,N,1);
	}
	/* reverse Fisher transform for correlations */
	if(setnorm==6) {
		norm1=0.0; norm2=1; n=0;
		xf_fishertransformrev2_d(data,N,1);
	}
	/* log transform  */
	if(setnorm==7) {
		norm1=0.0; norm2=1;
		for(i=0;i<N;i++) data[i] = log10(data[i]);
	}

	//TEST:fprintf(stderr,"norm1=%g	norm2=%g\n",norm1,norm2);

	/********************************************************************************/
	/* RE-READ THE FILE AND OUTPUT NORMALIZED VALUES (OR "-") FOR SELCTED COLUMN */
	/********************************************************************************/
	n=-1; // new line-count variable
	if(setstdin==1) sprintf(infile,"%s",outfile); // if temporary file was made, this becomes the infile
	if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
	while(fgets(line,MAXLINELEN,fpin)!=NULL) {
		n++;
		if(line[0]=='#') {printf("%s",line); continue;}
		pline=line;
		for(col=1;(pcol=strtok(pline," ,\t\n"))!=NULL;col++) {
			pline=NULL;
			if(col==datacol) {
				if(datagood[n]==1) {
					aa=(data[n]-norm1)/norm2;
					if(setp>0) printf("%.*f\t",setp,aa);
					else if(setp==0) printf("%g\t",aa);
					else printf("%f\t",aa);
				}
				else if(datagood[n]==0) printf("-\t"); // if invalid value was found
				else if(datagood[n]==-1) printf("%s\t",pcol); // if non-numeric field was found
			}
			else printf("%s\t",pcol);
		}
		printf("\n");
	}
	fclose(fpin);

	free(data);
	free(datagood);

	if(setstdin==1) remove(outfile);
	exit(status);
}
