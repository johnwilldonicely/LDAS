#define thisprog "xe-statsgrp0"
#define TITLE_STRING thisprog" v 9: 16.November.2017 [JRH]"
#define MAXLINE 10000
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>

/*
<TAGS>math stats</TAGS>
v 9: 16.November.2017 [JRH]
	- add option to output median

v 9: 9.January.2017 [JRH]
	- bugfix decimal precision calculation by xf_precision_d
		- needed to allow max decimal precision due to issues precisely representing floating-point numbers

v 9: 16.June.2014 [JRH]
	- fixed error in order of precision,value arguments for fprintf - error was masked by compiler ability to interpret second argument as the precision if it was of type int
	- speed optomization - skip output loop if ngroup <1 for any given group

v 8: 22.November.2013 [JRH]
	- include option for no-headers, to simplify output

v 7: 8.November.2013 [JRH]
	- convert grouping value to long integer and multiply during read process
	- remove "PRISM" output format - not currently used and easily achieved by transposing the output
	- greatly reduce memory requirements by calculating mean,sd,sem & ci on the fly during output instead of storing for each group
	- use nextafter and copysign functions to adjust grouping variables for proper rounding
	- switching to using long ints for grouping factors
	- eliminate pre-determined MAXGROUPS - allocate sufficient memory from the heap instead
	- note that for optimal use of memory for grouping evenly sampled time-series input, multiply by 1/interval

v 6: 15.March.2013 [JRH]
	- rename from xe-statsgrp1 to xe-statsgrp0
	- omit blank line at start of output - column header line is now the first line output
	- header begins with "grp" instead of "#grp" as previous

v 5b: 24.September.2012 [JRH]
	- renamed program from xe-statsgrp2 to xe-statsgr1 to avoid confusion with new programs accepting multiple grouping columns

v 5: 9.July.2012 [JRH]
	- allows individual data points to be NAN
	- will now output lines for all valid group-ids, even if the group has no valid y-data

v 4: 21.May.2012 [JRH]"
	- BEGUN to add ANOVA capabilities - incomplete

v 3: 2.May.2012 [JRH]
	- bugfix: integer group-ids were not being correctly derived from double-precision xdata values
		- solution: candoubleot cast directly to int from double - requires pre-casting to float

v 2.2: 24 August 2011 [JRH]
	- bugfix - change default -cg setting to 1, which is valid, instead of -1, which is not!

v 2.1: 14 August 2011 [JRH]
	- similar to xe-ststsgrpf1 but with several changes
	- completely re-wrote how the data is handled - now stored in memory
	- current version does not support different input layouts
	- can now deal with negative and fractional group-ids
		-mult option added to set decimal precision of groups
		calculates x-minimum to apply offset to group-ids
	- removed reference to old hux_error and hux_substring functions

*/

/* external functions start */
int xf_precision_d(double number,int max);
int xf_percentile1_d(double *data, long n, double *result);
int xf_compare1_d(const void *a, const void *b);
/* external functions end */

int main (int argc, char *argv[]) {
	/* general variables */
	char infile[256],line[MAXLINE],*pline,*pcol,*perror;
	long i,j,k,n,m;
	int w,x,y,z,col,sizeofint=sizeof(int),sizeoffloat=sizeof(float),sizeofdouble=sizeof(double),sizeoflong=sizeof(long);
	float a,b,c;
	double aa,bb,cc,ndouble,result_d[16];
	FILE *fpin,*fpout;
	/* program-specific variables */
	int coltot=1,alphapercent=5;
	long *xdat=NULL,*ng=NULL,xmin,xmax,grouptot,group;
	double *ydat=NULL,*zdat=NULL,*sum=NULL,*sumsquares=NULL;
	double mean,var,sd,sem,ci;
	/* arguments */
	char outfile[256];
	int xcol=1,ycol=2,sethead=1,percalc=0;
	double setmult=1.0;

	sprintf(outfile,"stdout");

	/* the following is a table of critical t-values for alpha levels of 0.1, 0.05, and 0.01 */
	/* this is needed for calculation of the confidence intervals */
	double t100[202] = {0,0,
	2.9200,2.3534,2.1318,2.0150,1.9432,1.8946,1.8595,1.8331,1.8125,
	1.7959,1.7823,1.7709,1.7613,1.7531,1.7459,1.7396,1.7341,1.7291,1.7247,
	1.7207,1.7171,1.7139,1.7109,1.7081,1.7056,1.7033,1.7011,1.6991,1.6973,
	1.6955,1.6939,1.6924,1.6909,1.6896,1.6883,1.6871,1.6860,1.6849,1.6839,
	1.6829,1.6820,1.6811,1.6802,1.6794,1.6787,1.6779,1.6772,1.6766,1.6759,
	1.6753,1.6747,1.6741,1.6736,1.6730,1.6725,1.6720,1.6716,1.6711,1.6706,
	1.6702,1.6698,1.6694,1.6690,1.6686,1.6683,1.6679,1.6676,1.6672,1.6669,
	1.6666,1.6663,1.6660,1.6657,1.6654,1.6652,1.6649,1.6646,1.6644,1.6641,
	1.6639,1.6636,1.6634,1.6632,1.6630,1.6628,1.6626,1.6624,1.6622,1.6620,
	1.6618,1.6616,1.6614,1.6612,1.6611,1.6609,1.6607,1.6606,1.6604,1.6602,
	1.6600,1.6600,1.6600,1.6600,1.6590,1.6590,1.6590,1.6590,1.6590,1.6590,
	1.6590,1.6590,1.6580,1.6580,1.6580,1.6580,1.6580,1.6580,1.6580,1.6580,
	1.6580,1.6570,1.6570,1.6570,1.6570,1.6570,1.6570,1.6570,1.6570,1.6570,
	1.6570,1.6560,1.6560,1.6560,1.6560,1.6560,1.6560,1.6560,1.6560,1.6560,
	1.6560,1.6560,1.6560,1.6560,1.6550,1.6550,1.6550,1.6550,1.6550,1.6550,
	1.6550,1.6550,1.6550,1.6550,1.6550,1.6550,1.6550,1.6550,1.6540,1.6540,
	1.6540,1.6540,1.6540,1.6540,1.6540,1.6540,1.6540,1.6540,1.6540,1.6540,
	1.6540,1.6540,1.6540,1.6540,1.6540,1.6540,1.6540,1.6530,1.6530,1.6530,
	1.6530,1.6530,1.6530,1.6530,1.6530,1.6530,1.6530,1.6530,1.6530,1.6530,
	1.6530,1.6530,1.6530,1.6530,1.6530,1.6530,1.6530,1.6530,1.6530,1.6530
	};
	double t050[202] = {0,0,
	4.3027,3.1824,2.7765,2.5706,2.4469,2.3646,2.3060,2.2622,2.2281,
	2.2010,2.1788,2.1604,2.1448,2.1315,2.1199,2.1098,2.1009,2.0930,2.0860,
	2.0796,2.0739,2.0687,2.0639,2.0595,2.0555,2.0518,2.0484,2.0452,2.0423,
	2.0395,2.0369,2.0345,2.0322,2.0301,2.0281,2.0262,2.0244,2.0227,2.0211,
	2.0195,2.0181,2.0167,2.0154,2.0141,2.0129,2.0117,2.0106,2.0096,2.0086,
	2.0076,2.0066,2.0057,2.0049,2.0040,2.0032,2.0025,2.0017,2.0010,2.0003,
	1.9996,1.9990,1.9983,1.9977,1.9971,1.9966,1.9960,1.9955,1.9949,1.9944,
	1.9939,1.9935,1.9930,1.9925,1.9921,1.9917,1.9913,1.9908,1.9905,1.9901,
	1.9897,1.9893,1.9890,1.9886,1.9883,1.9879,1.9876,1.9873,1.9870,1.9867,
	1.9864,1.9861,1.9858,1.9855,1.9852,1.9850,1.9847,1.9845,1.9842,1.9840,
	1.9840,1.9830,1.9830,1.9830,1.9830,1.9830,1.9820,1.9820,1.9820,1.9820,
	1.9820,1.9810,1.9810,1.9810,1.9810,1.9810,1.9800,1.9800,1.9800,1.9800,
	1.9800,1.9800,1.9790,1.9790,1.9790,1.9790,1.9790,1.9790,1.9790,1.9780,
	1.9780,1.9780,1.9780,1.9780,1.9780,1.9780,1.9770,1.9770,1.9770,1.9770,
	1.9770,1.9770,1.9770,1.9770,1.9760,1.9760,1.9760,1.9760,1.9760,1.9760,
	1.9760,1.9760,1.9760,1.9750,1.9750,1.9750,1.9750,1.9750,1.9750,1.9750,
	1.9750,1.9750,1.9750,1.9750,1.9740,1.9740,1.9740,1.9740,1.9740,1.9740,
	1.9740,1.9740,1.9740,1.9740,1.9740,1.9740,1.9730,1.9730,1.9730,1.9730,
	1.9730,1.9730,1.9730,1.9730,1.9730,1.9730,1.9730,1.9730,1.9730,1.9730,
	1.9720,1.9720,1.9720,1.9720,1.9720,1.9720,1.9720,1.9720,1.9720,1.9720
	};
	double t001[202] = {0,0,
	9.9250,5.8408,4.6041,4.0321,3.7074,3.4995,3.3554,3.2498,3.1693,
	3.1058,3.0545,3.0123,2.9768,2.9467,2.9208,2.8982,2.8784,2.8609,2.8453,
	2.8314,2.8188,2.8073,2.7970,2.7874,2.7787,2.7707,2.7633,2.7564,2.7500,
	2.7440,2.7385,2.7333,2.7284,2.7238,2.7195,2.7154,2.7116,2.7079,2.7045,
	2.7012,2.6981,2.6951,2.6923,2.6896,2.6870,2.6846,2.6822,2.6800,2.6778,
	2.6757,2.6737,2.6718,2.6700,2.6682,2.6665,2.6649,2.6633,2.6618,2.6603,
	2.6589,2.6575,2.6561,2.6549,2.6536,2.6524,2.6512,2.6501,2.6490,2.6479,
	2.6469,2.6458,2.6449,2.6439,2.6430,2.6421,2.6412,2.6403,2.6395,2.6387,
	2.6379,2.6371,2.6364,2.6356,2.6349,2.6342,2.6335,2.6329,2.6322,2.6316,
	2.6309,2.6303,2.6297,2.6291,2.6286,2.6280,2.6275,2.6269,2.6264,2.6259,
	2.6250,2.6250,2.6240,2.6240,2.6230,2.6230,2.6230,2.6220,2.6220,2.6210,
	2.6210,2.6200,2.6200,2.6200,2.6190,2.6190,2.6190,2.6180,2.6180,2.6170,
	2.6170,2.6170,2.6160,2.6160,2.6160,2.6150,2.6150,2.6150,2.6140,2.6140,
	2.6140,2.6140,2.6130,2.6130,2.6130,2.6120,2.6120,2.6120,2.6120,2.6110,
	2.6110,2.6110,2.6110,2.6100,2.6100,2.6100,2.6100,2.6090,2.6090,2.6090,
	2.6090,2.6090,2.6080,2.6080,2.6080,2.6080,2.6080,2.6070,2.6070,2.6070,
	2.6070,2.6070,2.6060,2.6060,2.6060,2.6060,2.6060,2.6050,2.6050,2.6050,
	2.6050,2.6050,2.6050,2.6040,2.6040,2.6040,2.6040,2.6040,2.6040,2.6030,
	2.6030,2.6030,2.6030,2.6030,2.6030,2.6030,2.6020,2.6020,2.6020,2.6020,
	2.6020,2.6020,2.6020,2.6010,2.6010,2.6010,2.6010,2.6010,2.6010,2.6010
	};


	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Calculates summary statistics for a data set or grouped data\n");
		fprintf(stderr,"Grouping variables must be numbers\n");
		fprintf(stderr,"Non-numeric data, NAN, and INF will be excluded\n");
		fprintf(stderr,"USAGE: %s [filename] [arguments]\n",thisprog);
		fprintf(stderr,"	[filename]: file name or \"stdin\"\n");
		fprintf(stderr,"	[options]...\n");
		fprintf(stderr,"		-a: alpha-%% level for conf.intervals (1,5,10) [%d]\n",alphapercent);
		fprintf(stderr,"		-cg: column containing numerical grouping values [%d]\n",xcol);
		fprintf(stderr,"		-cy: column containing data to be summarized [%d]\n",ycol);
		fprintf(stderr,"		-mult: factor by which to multiply grouping values [%g]\n",setmult);
		fprintf(stderr,"			NOTE: this allows setting decimal-precision for floating-\n");
		fprintf(stderr,"			point grouping values, which must be converted to integers.\n");
		fprintf(stderr,"			The multiplier can also optimize memory usage for when the\n");
		fprintf(stderr,"			grouping values are evenly-spaced timestamps. In this case,\n");
		fprintf(stderr,"			set -mult to 1/sample-interval\n");
		fprintf(stderr,"		-per percentile calculation (0=NO 1=YES) [%d]\n",percalc);
		fprintf(stderr,"		-out: specify an output file or \"stdout\" (screen) [%s]\n",outfile);
		fprintf(stderr,"		-h: output header labelling columns? (0=NO, 1=YES) [%d]\n",sethead);
		fprintf(stderr,"- examples:\n");
		fprintf(stderr,"	%s data.txt \n",thisprog);
		fprintf(stderr,"	cat temp.txt | %s stdin -mult 10\n",thisprog);
		fprintf(stderr,"- output:\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	sprintf(infile,"%s",argv[1]);
	for(i=2;i<argc;i++) {
		if( *(argv[i]+0) == '-') {
			if((i+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[i]); exit(1);}
			else if(strcmp(argv[i],"-a")==0) 	{ alphapercent=atoi(argv[i+1]); i++;}
			else if(strcmp(argv[i],"-cg")==0) 	{ xcol=atoi(argv[i+1]); i++;}
			else if(strcmp(argv[i],"-cy")==0) 	{ ycol=atoi(argv[i+1]); i++;}
			else if(strcmp(argv[i],"-mult")==0) { setmult=atof(argv[i+1]); i++;}
			else if(strcmp(argv[i],"-per")==0) 	{ percalc=atoi(argv[i+1]); i++;}
			else if(strcmp(argv[i],"-out")==0) 	{ sprintf(outfile,"%s",argv[i+1]); i++;}
			else if(strcmp(argv[i],"-h")==0) 	{ sethead=atoi(argv[i+1]); i++;}
			else {fprintf(stderr,"\t\a--- Error[%s]: invalid command line argument \"%s\"\n",thisprog,argv[i]); exit(1);}
	}}
	if(xcol<1) {fprintf(stderr,"\t\a--- Error[%s]: -cg (%d) must be >0\n",thisprog,xcol);exit(1);}
	if(ycol<1) {fprintf(stderr,"\t\a--- Error[%s]: -cy (%d) must be >0\n",thisprog,ycol);exit(1);}
	if(sethead!=0&&sethead!=1) {fprintf(stderr,"\t\a--- Error[%s]: -h (%d) must be 0 or 1\n",thisprog,sethead);exit(1);}
	if(percalc!=0&&percalc!=1) {fprintf(stderr,"\t\a--- Error[%s]: -per (%d) must be 0 or 1\n",thisprog,percalc);exit(1);}

	/* READ INPUT AND CALCULATE GROUP STATS ON THE FLY */
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\t\a--- Error[%s]: file \"%s\" not found\n",thisprog,infile);exit(1);}

	/* READ DATA IF IN SINGLE-COLUMN FORMAT WITH OPTIONAL GROUPING COLUMN */
	n=0;
	coltot=2;
	while(fgets(line,MAXLINE,fpin)!=NULL) {
		pline=line; z=coltot; if(line[0]=='#') continue;
		for(col=1;(pcol=strtok(pline," ,\t\n"))!=NULL;col++) {
			pline=NULL;
			if(col==xcol) {if(sscanf(pcol,"%lf",&aa)==1) z--;}
			if(col==ycol) {z--;if(sscanf(pcol,"%lf",&bb)!=1) bb=NAN;} /* store y-value if present, even if not a finite number */
		}
		if(z==0) {
			if(isfinite(aa)) {
				/* allocate memory */
				if((xdat=(long *)realloc(xdat,(n+1)*sizeoflong))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
				if((ydat=(double *)realloc(ydat,(n+1)*sizeofdouble))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
				/* make aa the smallest possible amount bigger (or smaller) */
				if(aa>=0)
					cc= nextafter(aa,DBL_MAX);
				else
					cc= nextafter(aa,-DBL_MAX);
				/* multiply by setmult to get desired precision xdat - note, round-down so that only +ive numbers are placed in the "zero" group */
				xdat[n]= (long)(setmult * cc);
				ydat[n]=bb;
				n++;
	}}}
	if(n<=0) {fprintf(stderr,"\n--- Error[%s]: no data in columns %d and %d of file %s\n\n",thisprog,xcol,ycol,infile);exit(1);};

	/* ALLOCATE MEMORY FOR PER-GROUP DATA IF PERCENTILES ARE TO BE CALCULATED */
	if(percalc==1) {
		zdat= realloc(zdat,n*sizeof(*zdat));
		if(zdat==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
	}

	/* FIND RANGE OF GROUPING VARIABLE XDAT SO ADJUSTMENT CAN BE CALCULATED TO ELIMINATE NEGATIVE GROUP NUMBERS */
	xmin=xdat[0];xmax=xdat[0]; for(i=0;i<n;i++) { if(xdat[i]<xmin) xmin=xdat[i]; if(xdat[i]>xmax) xmax=xdat[i]; }
	for(i=0;i<n;i++) xdat[i]-=xmin;
	grouptot=(xmax-xmin)+1;


	/* INITIALIZE VARIABLES */
	ng=(long*)calloc(grouptot,sizeof(long));
	sum=(double*)calloc(grouptot,sizeof(double));
	sumsquares=(double*)calloc(grouptot,sizeof(double));

	/* CALCULATE BASIC STATS FOR EACH GROUP */
	for(i=0;i<n;i++) {
		group=xdat[i];
		if(isfinite(ydat[i])) {
			ng[group]++;
			bb=ydat[i];
			sum[group]+=bb;
			sumsquares[group]+=(bb*bb);
	}}

	/* PRINT OUTPUT - CALCULATE DEATAILED STATS ON THE FLY  */
	if(strcmp(outfile,"stdout")==0) fpout=stdout;
   	else if((fpout=fopen(outfile,"w"))==0) {fprintf(stderr,"\t\a--- Error[%s]: could not write to \"%s\"\n",thisprog,outfile);exit(1);}

	if(sethead==1) {
		if(percalc==1) fprintf(fpout,"grp	n	mean	sd	sem	ci(%d%%)	med\n",alphapercent);
		else           fprintf(fpout,"grp	n	mean	sd	sem	ci(%d%%)\n",alphapercent);
	}

	z= xf_precision_d((1.0/setmult),8);
	for(group=0;group<grouptot;group++) {

		if(ng[group]<1) continue;

		aa=sum[group];
		bb=(double)(group+xmin)/setmult; // original xdat value
		mean=aa;
		sd=sem=ci=NAN;

		if (ng[group]>1)	{
			ndouble=(double)ng[group];
			mean = aa/ndouble;
			var  = ( sumsquares[group]-(aa*aa/ndouble)) / (ndouble-1.0);
			if(var>0) {
				sd = sqrt(var);
				sem= sd/sqrt(ndouble);
				x= ng[group]; if(x>200) x=200;
				if(alphapercent==10) ci= t100[x]*sem;
				else if(alphapercent==5) ci= t050[x]*sem;
				else if(alphapercent==1) ci= t001[x]*sem;
		}}

		/* if percentiles are required... */
		if(percalc==1) {
			/* copy the data for this group only  */
			for(i=m=0;i<n;i++) {
				if(xdat[i]==group) zdat[m++]= ydat[i];
			}

			/* get the percentiles  */
			k=xf_percentile1_d(zdat,m,result_d);
			if(k!=0) {fprintf(stderr,"\t\aError[%s]: insufficient memory for calculation of percentiles%s\n",thisprog);exit(1);}
			fprintf(fpout,"%.*f\t%d\t%g\t%g\t%g\t%g\t%g\n",z,bb,m,mean,sd,sem,ci,result_d[5]);
		}
		else {
			fprintf(fpout,"%.*f\t%d\t%g\t%g\t%g\t%g\n",z,bb,ng[group],mean,sd,sem,ci);
		}
	}

	if(strcmp(outfile,"stdout")!=0) fclose(fpout);
	if(sethead==1) fprintf(fpout,"\n");

	if(xdat!=NULL) free(xdat);
	if(ydat!=NULL) free(ydat);
	if(zdat!=NULL) free(zdat);
	if(ng!=NULL)   free(ng);
	if(sum!=NULL)  free(sum);
	if(sumsquares!=NULL) free(sumsquares);

	exit(0);
}
