#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>   /* needed for time() function */
#include <unistd.h>	/* needed for getpid() function */

#define thisprog "xe-makedata2"
#define TITLE_STRING thisprog" v 1: 23.January.2017 [JRH]"
#define MAXLINELEN 1000
#define MAXLABELS 1000

/*
<TAGS>synthetic_data</TAGS>
NOTE: CURENTLY INCLUDES EXCTRA LEGACY CODE? INCOMPLETE??

v 1: 23.January.2017 [JRH]
	- add option to include data for timestamp corresponding to [dur]

v 1: 30.September.2015 [JRH]
	- minor bugfixes for compliance with Ubuntu
		-
v 0: 21.January.2015 [JRH]
*/




/* external functions start */
double xf_rand1_d(double setmax);
/* external functions end */

int main (int argc, char *argv[]) {
	/* general variables */
	char infile[256],outfile[256],line[MAXLINELEN],templine[MAXLINELEN],word[256],*pline,*pcol;
	long int i,j,k,n;
	size_t ii,jj,kk;
	unsigned long setseed;
	int v,w,x,y,z,col,colmatch;
	int sizeofchar=sizeof(char),sizeofshort=sizeof(short),sizeoflong=sizeof(long),sizeofint=sizeof(int),sizeoffloat=sizeof(float),sizeofdouble=sizeof(double);
	float a,b,c,d;
	double aa,bb,cc,dd, result_d[64];
	FILE *fpin,*fpout;
	/* program-specific variables */
	long nsamps=0,halfsamps,cyclelength,eventstart,eventend;
	double nyquist,mytime,signal,noise,twopi=2.0*M_PI,newfreq,temppulse,temppulsesd,tempamp,tempnoise;
	/* arguments */
	int settype=0,seteventtype=0,setgaussian=0,setend=0,setout=1;
	long eventint,eventdur;
	double setdur,setrate,setoff=0.0;
	double setpulse1=1.0,setpulsesd1=0.0,setamp1=1.0,setnoise1=0.0;
	double setpulse2=-1.0,setpulsesd2=-1.0,setamp2=-1.0,setnoise2=-1.0;
	double seteventint=0.0,seteventdur=0.0;
	double setpulseint=-1.0;

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<3) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Create a fake data signal at a given frequency with added white noise\n");
		fprintf(stderr,"Includes option to create periodic changes to the signal (events)\n");
		fprintf(stderr,"\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [dur] [rate] [options]\n",thisprog);
		fprintf(stderr,"	[dur]: length of output, in seconds (e.g. 10)\n");
		fprintf(stderr,"	[rate]: sampling rate in samples/second (e.g. 24000))\n");
		fprintf(stderr,"\n");
		fprintf(stderr,"VALID OPTIONS (defaults in []):\n");
		fprintf(stderr,"	-p: pulse duration (s) [%g]\n",setpulse1);
		fprintf(stderr,"	-i: pulse interval (s) [%g]\n",setpulseint);
		fprintf(stderr,"	-fsd: standard deviation (Hz) around the main frequency [%g]\n",setpulsesd1);
		fprintf(stderr,"		NOTE: for some combinations of -f and rate, it may not\n");
		fprintf(stderr,"		be possible to produce a cycle at exacly frequency -f,\n");
		fprintf(stderr,"		especially as -f exceeds 1/10 the sample rate. This\n");
		fprintf(stderr,"		can be corrected by increasing sampling rate or -vf.\n");
		fprintf(stderr,"	-a: amplitude multiplier for signal (base = -1 to 1)  [%g]\n",setamp1);
		fprintf(stderr,"	-n: amplitude multiplier for noise (base value is 0-1) [%g]\n",setnoise1);
		fprintf(stderr,"	-g: noise distribution (0=uniform, 1=gaussian) [%d]\n",setgaussian);
		fprintf(stderr,"		NOTE: generated using Box-Muler method\n");
        		fprintf(stderr,"		NOTE: for Gaussian noise, -n determines the std.deviation\n");
		fprintf(stderr,"	-t: signal type [%d]\n",settype);
		fprintf(stderr,"		0: half-sine\n");
		fprintf(stderr,"		1: square-wave\n");
		fprintf(stderr,"	-end: also output data for [dur] timestamp (0=NO 1=YES)  [%d]\n",setend);
		fprintf(stderr,"	-o: output format: 1=<data>, 2=<time><tab><data> [%d]\n",setout);
		fprintf(stderr,"\n");
		fprintf(stderr,"OPTIONS FOR DEFINING PERIODIC EVENTS:\n");
		fprintf(stderr,"(changes in frequency, frequency std.dev., amplitude or noise\n");
		fprintf(stderr,"	-et: event type (0=none, 1=complete each cycle, 2=force change) [%d]\n",seteventtype);
		fprintf(stderr,"		NOTE: event type must be >0 for any events to be generated\n");
		fprintf(stderr,"		NOTE: -et 1 may cause variation (<1 cycle) in event times\n");
		fprintf(stderr,"		NOTE: -et 2 may cause abrupt changes at event boundaries\n");
		fprintf(stderr,"	-ed: event duration, in seconds [%f]\n",seteventdur);
		fprintf(stderr,"	-ei: interval between events, in seconds [%f]\n",seteventint);
		fprintf(stderr,"	-ep: signal frequency during event (-1:no change)[%g]\n",setpulse2);
		fprintf(stderr,"	-epsd: std.dev of freq. durng event (-1:no change)[%g]\n",setpulsesd2);
		fprintf(stderr,"	-ea: amplitude during event (-1:no change)[%g]\n",setamp2);
		fprintf(stderr,"	-en: noise level during event (-1:no change) [%g]\n",setnoise2);
		fprintf(stderr,"\n");
		fprintf(stderr,"EXAMPLE: 10s @24KHz of 4Hz signal, amplitude x2 for 1s every 5s:\n");
		fprintf(stderr,"	%s 10 24000 -f 4 -n 0 -et 1 -ei 5 -ed 1 -ea 2\n",thisprog);
		fprintf(stderr,"\n");
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	1st column: mytime (seconds)\n");
		fprintf(stderr,"	2nd column: fake signal = attenuation*(sinewave-noise)\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/********************************************************************************/
	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	/********************************************************************************/
	setdur=atof(argv[1]);
	setrate=atof(argv[2]);
	for(i=3;i<argc;i++) {
		if( *(argv[i]+0) == '-') {
			if((i+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[i]); exit(1);}
			else if(strcmp(argv[i],"-g")==0) 	setgaussian=atoi(argv[++i]);
			else if(strcmp(argv[i],"-t")==0) 	settype=atoi(argv[++i]);
			else if(strcmp(argv[i],"-o")==0) 	setout=atoi(argv[++i]);
			else if(strcmp(argv[i],"-end")==0) 	setend=atoi(argv[++i]);

			else if(strcmp(argv[i],"-p")==0) 	setpulse1=atof(argv[++i]);
			else if(strcmp(argv[i],"-i")==0) 	setpulseint=atof(argv[++i]);
			else if(strcmp(argv[i],"-fsd")==0) 	setpulsesd1=atof(argv[++i]);
			else if(strcmp(argv[i],"-a")==0) 	setamp1=atof(argv[++i]);
			else if(strcmp(argv[i],"-n")==0) 	setnoise1=atof(argv[++i]);

			else if(strcmp(argv[i],"-et")==0) 	seteventtype=atoi(argv[++i]);
			else if(strcmp(argv[i],"-ei")==0) 	seteventint=atof(argv[++i]);
			else if(strcmp(argv[i],"-ed")==0) 	seteventdur=atof(argv[++i]);
			else if(strcmp(argv[i],"-ep")==0) 	setpulse2=atof(argv[++i]);
			else if(strcmp(argv[i],"-epsd")==0) setpulsesd2=atof(argv[++i]);
			else if(strcmp(argv[i],"-ea")==0) 	setamp2=atof(argv[++i]);
			else if(strcmp(argv[i],"-en")==0) 	setnoise2=atof(argv[++i]);
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[i]); exit(1);}
	}}

	/********************************************************************************/
	/* CHECK THE ARGUMENTS */
	/********************************************************************************/
	nyquist=setrate/2.0;

	if(setdur<0.0) {fprintf(stderr,"\n--- Error[%s]: invalid duration (%g) - must be >0\n\n",thisprog,setdur); exit(1);}
	if(setrate<=0.0) {fprintf(stderr,"\n--- Error[%s]: invalid sample rate (%g) - must be >0\n\n",thisprog,setrate); exit(1);}
	if(setend!=0 && setend!=1) {fprintf(stderr,"\n--- Error[%s]: invalid end switch (-end %d) - must be 0 or 1\n\n",thisprog,setend); exit(1);}
	if(setpulse1<0.0) {fprintf(stderr,"\n--- Error[%s]: invalid signal frequency (-f %g) - must be >0\n\n",thisprog,setpulse1); exit(1);}
	if(setpulsesd1<0.0) {fprintf(stderr,"\n--- Error[%s]: invalid signal frequency std.dev (-fsd %g) - must be >0\n\n",thisprog,setpulsesd1); exit(1);}
	if(setnoise1<0.0) {fprintf(stderr,"\n--- Error[%s]: invalid noise (-n %g) - must be >= 0\n\n",thisprog,setnoise1); exit(1);}
	if(setgaussian!=0 && setgaussian!=1) {fprintf(stderr,"\n--- Error[%s]: invalid gaussian switch (-g %d) - must be 0 or 1\n\n",thisprog,setgaussian); exit(1);}
	if(setamp1<0.0) {fprintf(stderr,"\n--- Error[%s]: invalid amplitude (-a %g) - must be >= 0 \n\n",thisprog,setamp1); exit(1);}
	if(settype<0 || settype>4) {fprintf(stderr,"\n--- Error[%s]: invalid type (-t %d) - must be 0-4\n\n",thisprog,settype); exit(1);}
	if(setoff<0.0 || setoff>180.0) {fprintf(stderr,"\n--- Error[%s]: invalid offset (-p %g) - must be 0-180\n\n",thisprog,setoff); exit(1);}
	if(setout<1 || setout>2) {fprintf(stderr,"\n--- Error[%s]: invalid output format (-o %d) - must be 1 or 2\n\n",thisprog,setout); exit(1);}
	if(seteventdur<(2.0/setpulse2)) {fprintf(stderr,"\n--- Error[%s]: event duration (-ed %g) must be at lest 2x the cycle length of the event frequency (-ef %g) - must be 0-180\n\n",thisprog,seteventdur,setpulse2); exit(1);}
	if(setpulse1>=setdur) {fprintf(stderr,"\n--- Error[%s]: pulse length (%g) must be less than data duration (%g)\n\n",thisprog,setpulse1,setdur); exit(1);}
	if(setpulseint<=0&&setpulseint!=-1) {fprintf(stderr,"\n--- Error[%s]: pulse interval (%g) must be -1 or >0\n\n",thisprog,setpulseint); exit(1);}
	if(setpulseint<setpulse1&&setpulseint!=-1) {fprintf(stderr,"\n--- Error[%s]: pulse interval (%g) must be >= pulse duration (%g)\n\n",thisprog,setpulseint,setpulse1); exit(1);}


	/* event type = 0 (none) or if any event prameters are set to -1, use the main signal parameters */
	if(seteventtype==0||setpulse2<0.0) setpulse2=setpulse1;
	if(seteventtype==0||setpulsesd2<0.0) setpulsesd2=setpulsesd1;
	if(seteventtype==0||setamp2<0.0) setamp2=setamp1;
	if(seteventtype==0||setnoise2<0.0) setnoise2=setnoise1;

	/* define the event interval and duration in terms of samples, and set the initial event start/stop samples */
	eventint=(long)(seteventint*setrate + 0.5);
	eventdur=(long)(seteventdur*setrate + 0.5);
	eventstart=0;
	eventend=eventdur;

	nsamps=(long)(setdur*setrate + 0.5) ;
	if(setoff>0.0) setoff=(setoff/180.0)*M_PI;

	/* adjust nsamps to include the duration value */
	/* eg. if duration=1 and setrate=10, 11 points are output, including a timestamp for time 1.0 */
	if(setend==1) nsamps++;

	/********************************************************************************/
	/* GENERATE THE RANDOM NUMBER SEED BASED ON THE TIME AND THE CURRENT PROCESS ID */
	/********************************************************************************/
	setseed= (unsigned long) (time(NULL) + getpid());
	srand(setseed);

	temppulse=setpulse2;
	temppulsesd=setpulsesd2;
	tempamp=setamp2;
	tempnoise=setnoise2;


double *pulse=NULL;
long npulse,nint,nextpulse;

npulse=(long)(temppulse*setrate);
nint=(long)(setpulseint*setrate);
fprintf(stderr,"npulse=%ld\n",npulse);
nextpulse=nint;

if((pulse=(double *)realloc(pulse,(npulse*sizeofdouble)))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};


if(settype==0) for(jj=0;jj<npulse;jj++) {
	cc= M_PI/(double)npulse;
	pulse[jj]= tempamp * sin((double)jj*cc);
}
if(settype==1) for(jj=0;jj<npulse;jj++) {
	pulse[jj]= tempamp;
}


for(ii=0;ii<nsamps;ii++) {

	if(ii<nextpulse) {
		/* generate a noise value */
		if(setgaussian==1) {
			aa= xf_rand1_d(1.0);
			bb= xf_rand1_d(1.0);
			noise= tempnoise * sqrt(-2.0*log(aa)) * cos(twopi*bb) ; // Box-Muler method to convert from uniform to normal distribution
		}
		else {
			noise= tempnoise * xf_rand1_d(1.0) - tempnoise*0.5;
		}
		mytime= (double)ii/(double)setrate;
		printf("%g\t%g\n",mytime,noise);
	}

	else {
		fprintf(stderr,"EVENT: %ld\n",ii);
		for(jj=0;jj<npulse;jj++) {

			/* generate a noise value */
			if(setgaussian==1) {
				aa= xf_rand1_d(1.0);
				bb= xf_rand1_d(1.0);
				noise= tempnoise * sqrt(-2.0*log(aa)) * cos(twopi*bb) ; // Box-Muler method to convert from uniform to normal distribution
			}
			else {
				noise= tempnoise * xf_rand1_d(1.0) - tempnoise*0.5;
			}

			kk=ii+jj;
			if(kk>=nsamps) break;
			mytime= (double)kk/(double)setrate;
			printf("%g\t%g\n",mytime,(pulse[jj]+noise));
		}
		ii+=(npulse-1);
		nextpulse=ii+(nint-1);
		fprintf(stderr,"nextpulse: %ld\n",nextpulse);
	}
}


free(pulse);
exit(0);














	/********************************************************************************/
	/* GENERATE THE DATA  */
	/********************************************************************************/
	for(i=0;i<nsamps;i+=j) {

		// DETERMINE IF AN EVENT HAS BEGUN OR ENDED - ADJUST PARAMETERS ACCORDINGLY
		if(seteventtype!=0) {
			if(i >= eventstart) {
				eventend=i+eventdur;
				eventstart+=eventint;
				temppulse=setpulse2;
				temppulsesd=setpulsesd2;
				tempamp=setamp2;
				tempnoise=setnoise2;
			}
			if(i >= eventend) {
				eventend+=eventint;
				temppulse=setpulse1;
				temppulsesd=setpulsesd1;
				tempamp=setamp1;
				tempnoise=setnoise1;
			}
		}


		/* DETERMINE NEXT FREQUENCY - MAKE SURE IT'S BELOW THE NYQUIST FREQUENCY*/
		newfreq=nyquist;
		while(newfreq>=nyquist || newfreq <=0.0) {
			// determine gaussian randomized frequency for current cycle
			// Box-Muler method to convert from uniform to normal distribution
			aa= xf_rand1_d(1.0);
			bb= xf_rand1_d(1.0);
			cc= sqrt(-2.0*log(aa)) * cos(twopi*bb);
			// calculate new frequency
			newfreq=  temppulse + (temppulsesd * cc) ;
		}

		//TEST: j=1;printf("%lf\n",temppulse); continue;

		// DETERMINE CYCLE-LENGTH IN CYCLES
		cyclelength=(int)(setrate/newfreq +0.5);

		//TEST: j=cyclelength;printf("%ld\n",cyclelength); continue;

		// OUTPUT THE CYCLE + NOISE
		for(j=0;j<cyclelength;j++) {

			k=i+j;

			/* break the loop if total data length is exceeded */
			if(k>=nsamps) break;
			/* break the loop if total data length is exceeded */
			if(seteventtype>1) {
				if(k>=eventstart) break;
				if(k>=eventend) break;
			}

			/* generate a noise value */
			if(setgaussian==1) {
				aa= xf_rand1_d(1.0);
				bb= xf_rand1_d(1.0);
				noise= tempnoise * sqrt(-2.0*log(aa)) * cos(twopi*bb) ; // Box-Muler method to convert from uniform to normal distribution
			}
			else {
				noise= tempnoise * xf_rand1_d(1.0) - tempnoise*0.5;
			}

			/* calculate factor which, when multiplied by the last sample-number in the cycle, = 2*PI*/
			cc= twopi/(double)cyclelength;

			/* generate signal - amplify and add noise */
			if(settype==0) signal= tempamp * cos((double)j*cc + setoff) + noise ;
			if(settype==1) signal= tempamp * sin((double)j*cc + setoff) + noise ;

			/* output the data */
			if(setout==1)
				printf("%g\n",signal);
			else if(setout==2) {
				mytime= (double)k/(double)setrate;
				printf("%g\t%g\n",mytime,signal);
			}
		}


	} // END OF LOOP for(i=0;i<n;)

	exit(0);
}
