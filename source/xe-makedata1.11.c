#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>   /* needed for time() function */
#include <unistd.h>	/* needed for getpid() function */

#define thisprog "xe-makedata1"
#define TITLE_STRING thisprog" v 11: 23.January.2017 [JRH]"
#define MAXLINELEN 1000
#define MAXLABELS 1000

/*
<TAGS>synthetic_data</TAGS>

v 11: 23.January.2017 [JRH]
	- add option to include data for timestamp corresponding to [dur]

v 11: 30.September.2015 [JRH]
	- minor bugfixes for compliance with Ubuntu - fix print format for integer

v 10: 19.January.2014 [JRH]
	- minor bugfix - warn if frequency exceeds Nyquist and avoid random violations
	- slight change to how new-frequencies <=0 are handled
		- now loops to find new frequency, which should prevent distortion of the normal distribution

v 9: 15.July.2014 [JRH]
	- add support for periodic events in the signal
	- add support for single-column output

v 8: 14.June.2014 [JRH]
	- bugfix: Gaussian noise no longer overwrites variable aa (which was effectively destroying the signal itself)
	- bugfix: Unniform noise now has a mean of zero, like the Gauussian noise
	- bugfix in instructions - previous version tried to print settype as a character
	- make sine wave the default
	- remove impulse and sweep options

v 7: 22.October.2013 [JRH]
	- remove Ziggurat code - results are platform-dependent
	- in the absence of an easy-to-implement Ziggurat method (only advantage=speed?) this program will continue to use the Box-Muler method - which is apparently the method used by Python and the Boost libraries..


v 6: 1.October.2013 [JRH]
	- include Box-Muler method for generating Gaussian noise distributions
	- UNIMPLIMENTED: code put in place to allow use of George Marsaglia's ziggurat method for normal randoms

v 5: 23.September.2013 [JRH]
	- include generation of random seed based on time + process ID - ensures a unnique set of numbers every time
	- simplify signal calculation: output = signal*setamp1 + noise*setnoise1
	- allows any combination of signal and noise amplitude

v 4: 29.July.2013 [JRH]
	- bugfix: previously noise was 0-2 and conditional add/subtract created artefacts in signal at zero
		- now noise is -1 to +1 before attenuation, just as signal is
		- no artefact, and should be faster
	- add option to generate white noise only

v 3: 3.April.2013 [JRH]
 - ad option to apply a phase offset

v 2: 3.February.2012 [JRH]
	- add frequency-sweep functionality

v 1: 27.October.2012 [JRH]
	- create a fake data signal at a given frequency with added noise
*/




/* external functions start */
double xf_rand1_d(double setmax);
/* external functions end */

int main (int argc, char *argv[]) {
	/* general variables */
	char infile[256],outfile[256],line[MAXLINELEN],templine[MAXLINELEN],word[256],*pline,*pcol;
	long int i,j,k,n;
	unsigned long setseed;
	int v,w,x,y,z,col,colmatch;
	int sizeofchar=sizeof(char),sizeofshort=sizeof(short),sizeoflong=sizeof(long),sizeofint=sizeof(int),sizeoffloat=sizeof(float),sizeofdouble=sizeof(double);
	float a,b,c,d;
	double aa,bb,cc,dd, result_d[64];
	FILE *fpin,*fpout;
	/* program-specific variables */
	long nsamps=0,halfsamps,cyclelength,eventstart,eventend;
	double nyquist,mytime,signal,noise,twopi=2.0*M_PI,newfreq,tempfreq,tempfreqsd,tempamp,tempnoise;
	/* arguments */
	int settype=1,seteventtype=0,setgaussian=0,setend=0,setout=1;
	long eventint,eventdur;
	double setdur,setrate,setoff=0.0;
	double setfreq1=10.0,setfreqsd1=0.0,setamp1=1.0,setnoise1=0.0;
	double setfreq2=-1.0,setfreqsd2=-1.0,setamp2=-1.0,setnoise2=-1.0;
	double seteventint=0.0,seteventdur=0.0;


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
		fprintf(stderr,"	[dur]: length of output, in seconds (see -end option, below)\n");
		fprintf(stderr,"	[rate]: sampling rate in samples/second (e.g. 24000))\n");
		fprintf(stderr,"\n");
		fprintf(stderr,"VALID OPTIONS (defaults in []):\n");
		fprintf(stderr,"	-t: signal type [%d]\n",settype);
		fprintf(stderr,"		0: cosine function\n");
		fprintf(stderr,"		1: sine function\n");
		fprintf(stderr,"	-p: phase offset (0-180) for signal types 0 and 1, above [%g]\n",setoff);
		fprintf(stderr,"	-f: frequency (Hz) of signal to insert [%g]\n",setfreq1);
		fprintf(stderr,"	-fsd: standard deviation (Hz) around the main frequency [%g]\n",setfreqsd1);
		fprintf(stderr,"		NOTE: for some combinations of -f and rate, it may not\n");
		fprintf(stderr,"		be possible to produce a cycle at exacly frequency -f,\n");
		fprintf(stderr,"		especially as -f exceeds 1/10 the sample rate. This\n");
		fprintf(stderr,"		can be corrected by increasing sampling rate or -vf.\n");
		fprintf(stderr,"	-a: amplitude multiplier for signal (base = -1 to 1)  [%g]\n",setamp1);
		fprintf(stderr,"	-n: amplitude multiplier for noise (base value is 0-1) [%g]\n",setnoise1);
		fprintf(stderr,"	-g: noise distribution (0=uniform, 1=gaussian) [%d]\n",setgaussian);
		fprintf(stderr,"		NOTE: generated using Box-Muler method\n");
		fprintf(stderr,"		NOTE: for Gaussian noise, -n determines the std.deviation\n");
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
		fprintf(stderr,"	-ef: signal frequency during event (-1:no change)[%g]\n",setfreq2);
		fprintf(stderr,"	-efsd: std.dev of freq. durng event (-1:no change)[%g]\n",setfreqsd2);
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
			else if(strcmp(argv[i],"-p")==0) 	setoff=atof(argv[++i]);
			else if(strcmp(argv[i],"-o")==0) 	setout=atoi(argv[++i]);
			else if(strcmp(argv[i],"-end")==0) 	setend=atoi(argv[++i]);

			else if(strcmp(argv[i],"-f")==0) 	setfreq1=atof(argv[++i]);
			else if(strcmp(argv[i],"-fsd")==0) 	setfreqsd1=atof(argv[++i]);
			else if(strcmp(argv[i],"-a")==0) 	setamp1=atof(argv[++i]);
			else if(strcmp(argv[i],"-n")==0) 	setnoise1=atof(argv[++i]);

			else if(strcmp(argv[i],"-et")==0) 	seteventtype=atoi(argv[++i]);
			else if(strcmp(argv[i],"-ei")==0) 	seteventint=atof(argv[++i]);
			else if(strcmp(argv[i],"-ed")==0) 	seteventdur=atof(argv[++i]);
			else if(strcmp(argv[i],"-ef")==0) 	setfreq2=atof(argv[++i]);
			else if(strcmp(argv[i],"-efsd")==0) setfreqsd2=atof(argv[++i]);
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
	if(setfreq1<0.0) {fprintf(stderr,"\n--- Error[%s]: invalid signal frequency (-f %g) - must be >0\n\n",thisprog,setfreq1); exit(1);}
	if(setfreqsd1<0.0) {fprintf(stderr,"\n--- Error[%s]: invalid signal frequency std.dev (-fsd %g) - must be >0\n\n",thisprog,setfreqsd1); exit(1);}
	if(setnoise1<0.0) {fprintf(stderr,"\n--- Error[%s]: invalid noise (-n %g) - must be >= 0\n\n",thisprog,setnoise1); exit(1);}
	if(setgaussian!=0 && setgaussian!=1) {fprintf(stderr,"\n--- Error[%s]: invalid gaussian switch (-g %d) - must be 0 or 1\n\n",thisprog,setgaussian); exit(1);}
	if(setamp1<0.0) {fprintf(stderr,"\n--- Error[%s]: invalid amplitude (-a %g) - must be >= 0 \n\n",thisprog,setamp1); exit(1);}
	if(settype<0 || settype>4) {fprintf(stderr,"\n--- Error[%s]: invalid type (-t %d) - must be 0-4\n\n",thisprog,settype); exit(1);}
	if(setoff<0.0 || setoff>180.0) {fprintf(stderr,"\n--- Error[%s]: invalid offset (-p %g) - must be 0-180\n\n",thisprog,setoff); exit(1);}
	if(setout<1 || setout>2) {fprintf(stderr,"\n--- Error[%s]: invalid output format (-o %d) - must be 1 or 2\n\n",thisprog,setout); exit(1);}
	if(seteventdur<(2.0/setfreq2)) {fprintf(stderr,"\n--- Error[%s]: event duration (-ed %g) must be at lest 2x the cycle length of the event frequency (-ef %g) - must be 0-180\n\n",thisprog,seteventdur,setfreq2); exit(1);}
	if(setfreq1>=nyquist) {fprintf(stderr,"\n--- Error[%s]: signal frequency (%g) must be less than half the sample frequency (%g)\n\n",thisprog,setfreq1,setrate); exit(1);}


	/* event type = 0 (none) or if any event prameters are set to -1, use the main signal parameters */
	if(seteventtype==0||setfreq2<0.0) setfreq2=setfreq1;
	if(seteventtype==0||setfreqsd2<0.0) setfreqsd2=setfreqsd1;
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

	tempfreq=setfreq2;
	tempfreqsd=setfreqsd2;
	tempamp=setamp2;
	tempnoise=setnoise2;

	/********************************************************************************/
	/* GENERATE THE DATA  */
	/********************************************************************************/
	for(i=0;i<nsamps;i+=j) {

		// DETERMINE IF AN EVENT HAS BEGUN OR ENDED - ADJUST PARAMETERS ACCORDINGLY
		if(seteventtype!=0) {
			if(i >= eventstart) {
				eventend=i+eventdur;
				eventstart+=eventint;
				tempfreq=setfreq2;
				tempfreqsd=setfreqsd2;
				tempamp=setamp2;
				tempnoise=setnoise2;
			}
			if(i >= eventend) {
				eventend+=eventint;
				tempfreq=setfreq1;
				tempfreqsd=setfreqsd1;
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
			newfreq=  tempfreq + (tempfreqsd * cc) ;
		}

		//TEST: j=1;printf("%lf\n",tempfreq); continue;

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
			signal= NAN;
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
