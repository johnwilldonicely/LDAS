#define thisprog "xe-spectproc1"
#define TITLE_STRING thisprog" v 1: 30.May.2018 [JRH]"
#define MAXLINELEN 1000

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>


/* <TAGS> signal_processing spectra filter transform </TAGS> */

/*

v 1: 18.February.2021 [JRH]
	- use Butterworth filter instead of FIR for flattening the spectrum

v 1: 30.May.2018 [JRH]
	- improvement: use xf_geom_offset1_d to calculate relative peak amplitude

v 1: 4.April.2018 [JRH]
	- bugfix: do not close file out stream if output is to stdout

v 1: 3.March.2018 [JRH]
	- for user-defined bands, seek the closes match in the actual spectrum

v 1: 2.March.2018 [JRH]
	- bugfix: correctly calculate half-AUC
	- bugfix: make sure number of taps <= n/2
	- bugfix: fix print-error in instructions

v 1: 28.February.2018 [JRH]
	- switch AUC outputs to half-AUC, but keep auc as "aucfull"

v 1: 19.February.2018 [JRH]
	- switch to width= freq/div) ... default div=2

v 1: 18.February.2018 [JRH]
	- process power spectra (freq,amplitude)
	- started out as xe-smoothgauss2: adaptive log-smoother for physiological power-spectra
	- width= log(freq*mul)*(freq/div) ... default mul=1, div=5
	- other options considered:
		width= log(freq*setmul)*(freq/setdiv);
		width= 10.0*log(freq*setmul)/log(10.0);

*/

/* external functions start */
long *xf_lineparse2(char *line,char *delimiters, long *nwords);
int xf_smoothgauss0_f(float *original,size_t arraysize,size_t index,int smooth,float *result);
int xf_filter_bworth1_f(float *X, size_t nn, float sample_freq, float low_freq, float high_freq, float res, char *message);
long xf_detectinflect1_f(float *data,long n1,long **itime,int **isign,char *message);
int xf_auc2_f(float *curvex, float *curvey, size_t nn, int ref, double *result ,char *message);
double xf_geom_offset1_d(double x1,double y1,double x2,double y2, double x3, double y3, char *message);
/* external functions end */

int main (int argc, char *argv[]) {
	/* general variables */
	char line[MAXLINELEN],message[MAXLINELEN];
	int v,w,x,y,z;
	long int ii,jj,kk,nn,mm;
	float a,b,c,d;
	double aa,bb,cc,dd,ee;
	FILE *fpin,*fpout;
	/* program specific variables */
	char *out1,*out2;
	int *isign=NULL;
	long *itime=NULL,*index=NULL,*start1=NULL,*middle1=NULL,*stop1=NULL,nbands,halfwidth,mid1,min1,max1,min2,max2;
	float *freq1=NULL,*amp1=NULL,*amp2=NULL;
	double freq,width,spectfreq,spectmin,peakamp,auc1a,auc1b,auc1c,auc2a,auc2b,auc2c,result_d[8];
	/* filter variables */
	double *coefs=NULL,fomegac,fbw;
	int ftaps=101,fpass=2,fshift=1;
	double fbeta=0.0;
	/* arguments */
	char *infile,*setscreenlist=NULL;;
	int setout=0,setverb=0,setuseadj=1;
	double setdiv=2.0,setffreq=0.05;

	out1="temp_"thisprog"_bands.txt\0";
	out2="temp_"thisprog"_spect.txt\0";

	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Process neurophysiological power spectra \n");
		fprintf(stderr,"- 1. adjusts spectrum: adaptive smoothing, more at higher frequencies\n");
		fprintf(stderr,"- 2. adjusts spectrum: Butterworth HP filter to flatten the spectrum\n");
		fprintf(stderr,"- 3. detects bands (if undefined) using +ive and -ive inflections\n");
		fprintf(stderr,"- 4. gets stats (see below) for either orignal or adjusted spectrum\n");
		fprintf(stderr,"\nUSAGE: \n");
		fprintf(stderr,"    %s [infile]\n",thisprog);
		fprintf(stderr,"        [infile]: 2-column file or stdin in format [freq] [power]\n");
		fprintf(stderr,"VALID OPTIONS: defaults in []\n");
		fprintf(stderr,"    -bands: CSV band-triplets (start,mid,stop) [unset=auto]\n");
		fprintf(stderr,"    -out: output options [%d]\n",setout);
		fprintf(stderr,"        0= files-only\n");
		fprintf(stderr,"        1= spectrum\n");
		fprintf(stderr,"        2= band-definitions and measurements\n");
		fprintf(stderr,"        3= CSV list of band definition triplets (min,mid,max)\n");
		fprintf(stderr,"    -verb: verbose output (0=NO 1=YES) [%d]\n",setverb);
		fprintf(stderr,"\n");
		fprintf(stderr,"...spectral adjustment options...\n");
		fprintf(stderr,"    -div: divisor in width= freq/div  (0=SKIP) [%g]\n",setdiv);
		fprintf(stderr,"    -filt: HP filter frequency (0=SKIP) [%g]\n",setffreq);
		fprintf(stderr,"    -useadj: use adjusted spectrum to calculate stats (0=NO 1=YES) [%d]\n",setuseadj);
		fprintf(stderr,"    	- 0 avoids negative AUCs\n");
		fprintf(stderr,"    	- 1 avoids negative relative values on steep slopes\n");
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"    %s: the adjusted spectrum\n",out2);
		fprintf(stderr,"    %s: stats for each band\n",out1);
		fprintf(stderr,"        min mid max peak relpeak  auc aucpos aucneg  aucfull\n");
		fprintf(stderr,"        min: low-frequency boundary for the band\n");
		fprintf(stderr,"        mid: frequency at the band-peak\n");
		fprintf(stderr,"        max: high-frequency boundary for the band\n");
		fprintf(stderr,"        peak: absolute amplitude at the peak\n");
		fprintf(stderr,"        relpeak: amplitude relative to line joining min-max\n");
		fprintf(stderr,"        auc: half-width area-under-the-curve (maximises info)\n");
		fprintf(stderr,"        aucpos: as above, positive-values only\n");
		fprintf(stderr,"        aucneg: as above, negative-values only\n");
		fprintf(stderr,"        aucful: complete AUC between min and max\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(1);
	}

	/********************************************************************************
	READ THE FILENAME AND OPTIONAL ARGUMENTS - including comma-separated list item
	********************************************************************************/
	infile= argv[1];
	for(ii=2;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-bands")==0) setscreenlist=argv[++ii];
			else if(strcmp(argv[ii],"-out")==0)  setout=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-verb")==0) setverb=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-div")==0)  setdiv=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-filt")==0) setffreq=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-useadj")==0) setuseadj=atoi(argv[++ii]);
			else {fprintf(stderr,"\n*** %s [ERROR: invalid command line argument \"%s\"]\n\n",thisprog,argv[ii]); exit(1);}
	}}
	if(setout<0 || setout>3) { fprintf(stderr,"\n--- Error [%s]: invalid -out [%d] must be 0-3\n\n",thisprog,setout);exit(1);}
	if(setuseadj<0 || setuseadj>1) { fprintf(stderr,"\n--- Error [%s]: invalid -useadj [%d] must be 0-1\n\n",thisprog,setuseadj);exit(1);}
	if(setverb!=0 && setverb!=1) { fprintf(stderr,"\n--- Error [%s]: invalid -verb [%d] must be 0 or 1\n\n",thisprog,setverb);exit(1);}


	/*******************************************************************************
	STORE THE SPECTRUM
	*******************************************************************************/
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
	nn=0;
	x= sizeof(*freq1);
	y= sizeof(*amp1);
	while(fgets(line,MAXLINELEN,fpin)!=NULL) {
		if(sscanf(line,"%f %f",&a,&b)!=2) continue;
		if(!isfinite(a)) continue;
		freq1= realloc(freq1,(nn+1)*x);
		amp1= realloc(amp1,(nn+1)*y);
		if(freq1==NULL||amp1==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
		freq1[nn]= a;
		amp1[nn]= b;
		nn++;
	}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);
	//TEST:	for(ii=0;ii<nn;ii++) printf("%g\t%g\n",freq1[ii],amp1[ii]);exit(0);

	/* ALLOCATE MEMORY FOR TEMPORARY AMP2 ARRAY (FLOAT) */
	amp2= malloc(nn*sizeof(amp2));
	if(amp2==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}

	/* DETERMINE THE SPECRTUM SAMPLE-FREQUENCY AND MINIMUM  */
	spectmin= freq1[0];
	spectfreq= 1.0/(freq1[1]-freq1[0]);


	/*******************************************************************************
	PARSE THE BAND START-STOP PAIRS
	*******************************************************************************/
	if(setscreenlist!=NULL) {
		index= xf_lineparse2(setscreenlist,",",&nbands);
		if((nbands%3)!=0) {fprintf(stderr,"\n--- Error[%s]: screen-list does not contain triplets of numbers\n\n",thisprog);exit(1);}
		nbands/=3;
		if((start1= malloc(nbands*sizeof(start1)))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		if((middle1= malloc(nbands*sizeof(middle1)))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		if((stop1= malloc(nbands*sizeof(stop1)))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		for(ii=0;ii<nbands;ii++) {
			aa= atof(setscreenlist+index[3*ii]);
			bb= atof(setscreenlist+index[3*ii+1]);
			cc= atof(setscreenlist+index[3*ii+2]);
			/* find the closest match in the actual spectrum, save the sample-number */
			ee= fabs(aa-freq1[0]);
			for(jj=kk=0;jj<nn;jj++) {if(fabs(aa-freq1[jj])<ee) {ee=fabs(aa-freq1[jj]); kk=jj; }}
			start1[ii]= kk;
			ee= fabs(bb-freq1[0]);
			for(jj=kk=0;jj<nn;jj++) {if(fabs(bb-freq1[jj])<ee) {ee=fabs(bb-freq1[jj]); kk=jj; }}
			middle1[ii]= kk;
			ee= fabs(cc-freq1[0]);
			for(jj=kk=0;jj<nn;jj++) {if(fabs(cc-freq1[jj])<ee) {ee=fabs(cc-freq1[jj]); kk=jj; }}
			stop1[ii]= kk;
		}
		if(setverb==1) fprintf(stderr,"	- defined %ld good bands\n",nbands);
	}


	/*******************************************************************************
	APPLY ADAPTIVE SMOOTHER TO AMPLITUDES - STORE IN AMP2[]
	*******************************************************************************/
	if(setdiv>0) {
		if(setverb==1) fprintf(stderr,"\t- adaptive-smoothing: width=freq/%g\n",setdiv);
		for(ii=0;ii<nn;ii++) {
			/* determine the frequency */
			freq= (double)freq1[ii];
			/* calculate the smoothing half-window (samples) for this frequency */
			width=freq/setdiv;
			halfwidth= (long)((width-1.0)/setdiv);
			/* apply the smoother for this frequency */
			/* note that if halfwidth is <1, the function returns the original value */
			z= xf_smoothgauss0_f(amp1,(size_t)nn,(size_t)ii,halfwidth,&a);
			if(z!=0) {fprintf(stderr,"\n--- Error[%s]: error in smoothing function\n\n",thisprog);exit(1);}
			amp2[ii]= (float)a;
		}
	}
	else for(ii=0;ii<nn;ii++) amp2[ii]=amp1[ii];
	//TEST:	for(ii=0;ii<nn;ii++) printf("%g	%g\n",freq1[ii],amp2[ii]);free(amp1);free(amp2);exit(0);

	/*******************************************************************************
	APPLY A HIGH-PASS FIR FILTER
	*******************************************************************************/
	if(setffreq>0.0) {
		if(setverb==1) fprintf(stderr,"\t- filtering: HP=%g\n",setffreq);
		x= xf_filter_bworth1_f(amp2,nn,spectfreq,setffreq,0,sqrt(2),message);
		if(x!=0) { fprintf(stderr,"\n*** %s/%s\n\n",thisprog,message); exit(1); }
	}
	//TEST:	for(ii=0;ii<nn;ii++) printf("%g	%g\n",freq1[ii],amp2[ii]);exit(0);

	/*******************************************************************************
	AUTO-DEFINE BANDS BY DETECTING INFLECTIONS IN THE SPECTRUM
	*******************************************************************************/
	if(setscreenlist==NULL) {
		if(setverb==1) fprintf(stderr,"\t- detecting inflections: ");
		mm= xf_detectinflect1_f(amp2,nn,&itime,&isign,message);
		if(mm<0) {fprintf(stderr,"\n--- Error[%s]: error in smoothing function\n\n",thisprog);exit(1);}
		//TEST:for(ii=0;ii<mm;ii++) fprintf(stderr,"%g\t%d\n",freq1[itime[ii]],isign[ii]);
		for(ii=jj=0;ii<mm;ii++) if(isign[ii]>0) jj++;
		if(setverb==1) fprintf(stderr,"%ld total peaks found\n",jj);
		if((start1= malloc(jj*sizeof(start1)))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		if((middle1= malloc(jj*sizeof(middle1)))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		if((stop1= malloc(jj*sizeof(stop1)))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		nbands=0;
		for(ii=0;ii<mm;ii++) {
			if(isign[ii]<0) continue;
			/* determine spectrum-sample at positive inflection */
			mid1= itime[ii];
			/* determine the theoretical minimum/maximum based on the sample-rate (1/resolution) of the spectrum */
			kk= (long)(0.5 * freq1[mid1]*spectfreq);
			min1= mid1-kk; if(min1<0) min1=0;
			max1= mid1+kk; if(max1>=nn) max1=(nn-1);
			/* if neighbouring negative inflections fall within the limits, adjust  */
			if(ii>0 && itime[ii-1]>=min1) min1= itime[ii-1];
			if(ii<(mm-1) && itime[ii+1]<=max1) max1= itime[ii+1];
			/* band-rejection */
			if((max1-mid1)<(kk/8) || (mid1-min1)<(kk/8) ) continue;
			if(result_d[0]<0.0) continue;
			/* store bands */
			start1[nbands]=min1;
			middle1[nbands]=mid1;
			stop1[nbands]=max1;
			nbands++;
		}
		if(setverb==1) fprintf(stderr,"	- found %ld good bands\n",nbands);
	}
	//TEST: for(ii=0;ii<nbands;ii++) {fprintf(stderr,"%ld	%ld	%ld\n",start1[ii],middle1[ii],stop1[ii]);}



	/*******************************************************************************
	SAVE THE MODIFIED SPECTRUM - FILE OR SCREEN
	*******************************************************************************/
	if(setout==0 || setout==1) {
		if(setverb==1) fprintf(stderr,"\t- writing spectrum:   %s\n",out2);
		if(setout==1) fpout=stdout;
		else if((fpout=fopen(out2,"w"))==0) {fprintf(stderr,"\n--- Error[%s]: unable to write to file \"%s\"\n\n",thisprog,out2);exit(1);}
		fprintf(fpout,"freq\tsmoothed\n");
		for(ii=0;ii<nn;ii++) fprintf(fpout,"%g\t%g\n",freq1[ii],amp2[ii]);
		if(setout!=1) fclose(fpout);
	}

	/*******************************************************************************
	CALCULATE AND OUTPUT BAND PARAMETERS
	*******************************************************************************/
	/* IF BAND-STATS ARE TO BE BASED ON ADJUSTED SPECTRUM (-useadj 1), COPY ORIGINAL AMP1 TO AMP2*/
	if(setuseadj==0) for(ii=0;ii<nn;ii++) amp2[ii]= amp1[ii];
	//TEST:	for(ii=0;ii<nn;ii++) printf("%ld\t%g\t%g\t%g\n",ii,freq1[ii],amp1[ii],amp2[ii]);

	if(setout==0 || setout==2) {
		if(setverb==1) fprintf(stderr,"\t- writing band-stats: %s\n",out1);
		if(setout==2) fpout=stdout;
		else if((fpout=fopen(out1,"w"))==0) {fprintf(stderr,"\n--- Error[%s]: unable to write to file \"%s\"\n\n",thisprog,out1);exit(1);}
		fprintf(fpout,"min\tmid\tmax\tpeak\trelpeak\tauc\taucpos\taucneg\taucfull\n");

		for(ii=0;ii<nbands;ii++) {

			/* temporarily store parameters for current peak */
			min1= start1[ii];
			mid1= middle1[ii];
			max1= stop1[ii];

			/* calculate the relative-amplitude of the peak (mid) */
			peakamp= xf_geom_offset1_d(freq1[min1],amp2[min1],freq1[max1],amp2[max1],freq1[mid1],amp2[mid1],message);

			/* calcualte the full AUC */
			z= xf_auc2_f((freq1+min1),(amp2+min1),(1+max1-min1),1,result_d,message);
			if(z<0) { fprintf(stderr,"\n*** %s/%s\n\n",thisprog,message); exit(1); }
			auc1a= result_d[0];
			auc1b= result_d[1];
			auc1c= result_d[2];

			/* calculate half-width AUC */
			min2= mid1-(long)(((double)(mid1-min1))/2.0);
			max2= mid1+(long)(((double)(max1-mid1))/2.0);
			z= xf_auc2_f((freq1+min2),(amp2+min2),(1+max2-min2),1,result_d,message);
			if(z<0) { fprintf(stderr,"\n*** %s/%s\n\n",thisprog,message); exit(1); }
			auc2a= result_d[0];
			auc2b= result_d[1];
			auc2c= result_d[2];

			/* output options 0/2: bands and stats to file or stdout */
			fprintf(fpout,"%g\t%g\t%g	%.4f\t%.4f	%.4f\t%.4f\t%.4f	%.4f\n",
			freq1[min1],freq1[mid1],freq1[max1], amp2[mid1],peakamp, auc2a,auc2b,auc2c, auc1a);
		}
		if(setout==0) fclose(fpout);
	}


	/*******************************************************************************
	OUTPUT OPTION3: CSV TRIPLET LISTS OF MIN,MID,MAX
	*******************************************************************************/
	if(setout==3){
		 fprintf(fpout,"bands");
		 for(ii=0;ii<nbands;ii++) fprintf(fpout,",%g,%g,%g",freq1[start1[ii]],freq1[middle1[ii]],freq1[stop1[ii]]);
		 fprintf(fpout,"\n");
	}


	/*******************************************************************************
	FREE MEMORY AND EXIT
	*******************************************************************************/
END:
	if(freq1!=NULL) free(freq1);
	if(amp1!=NULL) free(amp1);
	if(amp2!=NULL) free(amp2);
	if(itime!=NULL) free(itime);
	if(isign!=NULL) free(isign);
	if(index!=NULL) free(index);
	if(start1!=NULL) free(start1);
	if(middle1!=NULL) free(middle1);
	if(stop1!=NULL) free(stop1);
	exit(0);
}
