#define thisprog "xe-ldas5-clucombinelist1"
#define TITLE_STRING thisprog" v 1: 4.April.2017 [JRH]"
#define MAXLINE 1000
#define HISTMAXSIZE 200

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>

/*
<TAGS>ldas spikes</TAGS>

Versions History:
v 1: 4.April.2017 [JRH]
*/

/* external functions start */
long xf_readclub1(char *infile1, char *infile2, long **clubt1, short **club1, char *message);
double xf_readwave1_f(char *infile, short **id, long **count, float **data, short **wlistc, long *resultl, char *message);
long *xf_lineparse2(char *line,char *delimiters, long *nwords);
int xf_writebin2_v(char *outfile, void *data0, size_t nn, size_t datasize, char *message);
int xf_writewave1_f(char *outfile, short *wid, long *wn, float *wmean, short *wlistc, long *result_l, long makez, double srate, char *message);
/* external functions end */


int main (int argc, char *argv[]) {

	/* general variables */
	char infile[256],line[256],*pline,*pcol,message[MAXLINE];
	int v,w,x,y,z,col,result_i[32];
	int sizeofint=sizeof(int),sizeofshort=sizeof(short int),sizeoflong=sizeof(long int),sizeoffloat=sizeof(float),sizeofdouble=sizeof(double);
	long ii,jj,kk,mm,nn,index0,index1,index2,result_l[32];
	float a,b,c,d;
	double aa,bb,cc,dd,ee,ff,gg,hh;
	FILE *fpin,*fpout;

	/* program-specific variables */
	char outfileclub[64],outfilewfm[64],outfilehist[64];
	short *club1=NULL,*club2=NULL,*combine=NULL,cluid1,cluid2,clumin,clumax;
	long *clubt1=NULL,*clubt2=NULL,*clun=NULL,*intervals=NULL,spiketot,spiketot2,nclust;

	short *wid=NULL,*wlistc=NULL;
	long *wclun=NULL, *wavecorchan=NULL, wnchans, wspklen, wspkpre, wnclust, wtotlen, wclumax, wprobe;
	float *wmean=NULL, *wmeantemp=NULL, wavemin,wavemax,wavecor;
	double wsfreq;

	int *cluarray1=NULL,*cluarray2=NULL;
	long *indexarray=NULL;
	off_t nlist;

	/* arguments */
	char *fileclub1,*fileclubt1,*filewfm,*setlist=NULL;
	int setpass=1,setverb=0,setminspikes=25,setcumulative=1,setsign=-1,sethistout=0;
	float setrefmax=0.01,setauto=0.02,setwavecor=0.975;
	float setmaxratio=0.8,setmint=3.0,setmaxp=0.5;
	float setwavehp=0;
	long setxcorside=13,setxcorcentre=2;
	double setacorz1=0.015,setacorz2=0.002,setadeltamax=1.0;

	sprintf(outfileclub,"temp_%s.club",thisprog);
	sprintf(outfilewfm,"temp_%s.wfm",thisprog);
	sprintf(outfilehist,"temp_%s.hist",thisprog);


	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<5) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Combines cluster-pairs using a list\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [in1] [in2] [in3] [list] [options]\n",thisprog);
		fprintf(stderr,"	[in1]: timestamps (.clubt) file for a given probe\n");
		fprintf(stderr,"	[in2]: cluster-id (.club) file for a given probe\n");
		fprintf(stderr,"	[in3]: waveform (.wfm) file a given probe\n");
		fprintf(stderr,"	[list]: list of cluster-pairs to combine\n");
		fprintf(stderr,"VALID OPTIONS (defaults in []):\n");
		fprintf(stderr,"	-verb: verbocity [%d]\n",setverb);
		fprintf(stderr,"		0= combine only, no report\n");
		fprintf(stderr,"		1= combine + report: \n");
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s file.clubt file.club file.wfm\n",thisprog);
		fprintf(stderr,"FILE OUTPUT (-v 1 or 2):\n");
		fprintf(stderr,"	modified .club file %s\n",outfileclub);
		fprintf(stderr,"	modified .wfm file %s\n",outfilewfm);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/********************************************************************************
	READ THE FILENAME AND OPTIONAL ARGUMENTS
	********************************************************************************/
	fileclubt1= argv[1];
	fileclub1= argv[2];
	filewfm= argv[3];
	setlist= argv[4];

	for(ii=5;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if(ii>=argc) break;
			else if(strcmp(argv[ii],"-verb")==0)   { setverb=atoi(argv[ii+1]); ii++;}
			else {fprintf(stderr,"Error[%s]: invalid command line argument \"%s\"\n",thisprog,argv[ii]); exit(1);}
	}}
	if(setverb!=0 && setverb!=1) { fprintf(stderr,"\n--- Error [%s]: invalid -verb [%d] must be 0 or 1\n\n",thisprog,setverb);exit(1);}

	/********************************************************************************
	BUILD THE CLUSTER-PAIR ARRAYS
	********************************************************************************/
	indexarray= xf_lineparse2(setlist,",",&nlist);
	if((nlist%2)!=0) {fprintf(stderr,"\n--- Error[%s]: cluster-list does not contain pairs of numbers: %s\n\n",thisprog,setlist);exit(1);}
	nlist/=2;
	cluarray1= calloc(nlist,sizeof *cluarray1);
	cluarray2= calloc(nlist,sizeof *cluarray2);
	if(cluarray1==NULL||cluarray2==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
	for(ii=0;ii<nlist;ii++) {
		cluarray1[ii]= atoi(setlist+indexarray[2*ii]);
		cluarray2[ii]= atoi(setlist+indexarray[2*ii+1]);
	}

	/********************************************************************************
	READ THE CLUSTER TIMESTAMPS AND IDs
	********************************************************************************/
 	spiketot= xf_readclub1(fileclubt1,fileclub1,&clubt1,&club1,message);
 	if(spiketot<0) {fprintf(stderr,"\n*** %s/%s\n\n",thisprog,message); exit(1);}
	//TEST: for(ii=0;ii<spiketot;ii++) printf("%ld	%d\n",clubt1[ii],club1[ii]); exit(0);
	/* determine the lowest & highest cluster id */
	clumin=SHRT_MAX;
	clumax=SHRT_MIN;
	for(ii=0;ii<spiketot;ii++) {
		cluid1=club1[ii];
		if(cluid1<clumin) clumin=cluid1;
		if(cluid1>clumax) clumax=cluid1;
	}
	/* calculate spike-count in each cluster */
	clun= calloc((clumax+1),sizeof *clun); // holds the total spikes in each cluster
	if(clun==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n",thisprog);exit(1);}
	for(ii=0;ii<spiketot;ii++) clun[club1[ii]]++;
	/* determine the total number of populated clusters */
	nclust=0; for(ii=clumin;ii<=clumax;ii++) if(clun[ii]>0) nclust++;
	//TEST: printf("club_max=%d\n",clumax);
	//TEST: for(ii=0;ii<=clumax;ii++) if(clun[ii]>0) printf("%ld	%d\n",ii,clun[ii]); exit(0);


	/********************************************************************************
	READ THE .WFM FILE TO GET MEAN WAVEFORMS AND WAVEFORM PARAMETERS
	- waveform is stored as wavemean[cluster][channel][sample]
	- channel-order is defined by wlistc[]
	- id's and spike-counts for each cluster stored in wid[] and wclun[] respectively
	********************************************************************************/
	wsfreq= xf_readwave1_f(filewfm,&wid,&wclun,&wmean,&wlistc,result_l,message);
	if(wsfreq<0) {fprintf(stderr,"\nError[%s]: %s\n\n",thisprog,message); exit(1); }
	wnclust= result_l[0]; // total clusters, not the same as max_cluster_id (result_l[5])
	wnchans= result_l[1]; // number of channels per combined-waveform
	wspklen= result_l[2]; // number of samples per channel
	wspkpre= result_l[3]; // number of samples before time zero
	wtotlen= result_l[4]; // total length of combined-waveform
	wclumax= result_l[5]; // largest cluster number
	wprobe= result_l[6]; // probe-id
	/* check maximum-cluster-id's match in club and wfm records */
	if(wclumax!=clumax) {fprintf(stderr,"\n--- Error[%s]: max cluster in waveform file (%ld) does not match the club file (%ld)\n\n",thisprog,wclumax,clumax);exit(1);}
	if(wnclust!=nclust) {fprintf(stderr,"\n--- Error[%s]: no. of clusters in waveform file (%ld) does not match the club file (%ld)\n\n",thisprog,wnclust,nclust);exit(1);}
	for(ii=0;ii<wnclust;ii++) {if(wclun[ii]!=clun[wid[ii]]) {fprintf(stderr,"\n--- Error[%s]: cluster[%ld] count in waveform file (%ld) file does not match the club file (%ld)\n\n",thisprog,ii,wclun[ii],clun[wid[ii]]);exit(1);}}

	//TEST for(ii=0;ii<wnclust;ii++) fprintf(stderr,"%ld	%ld\n",wid[ii],wclun[ii]); exit(0);
	//TEST WAVEFORM: for(ii=0;ii<wnclust;ii++) {if(wclun[ii]>0) { index1=ii*wtotlen;for(jj=0;jj<wtotlen;jj++) printf("%ld\t%ld\t%g\n",wid[ii],jj,wmean[index1+jj]); }} exit(0);

	/* COMBINE THE CLUSTERS */
	for(index0=0;index0<nlist;index0++) {
		cluid1= cluarray1[index0];
		cluid2= cluarray2[index0];
		if(setverb==1) printf("	combining %d with %d\n",cluid1,cluid2);
		// combine clusters - copy lower cluster to higher one
		for(kk=0;kk<spiketot;kk++) { if(club1[kk]==cluid1) club1[kk]=cluid2; }
		clun[cluid2]+= clun[cluid1];
		clun[cluid1]= 0;
		/* determine ii and jj: indices to waveforms data for this pair */
		ii=jj=-1; for(kk=0;kk<wnclust;kk++) { if(wid[kk]==cluid1) ii=kk; if(wid[kk]==cluid2) jj=kk; }
		if(ii==-1) {fprintf(stderr,"\n--- Error[%s]: no cluster %d in waveform file \n\n",thisprog,cluid1);exit(1);}
		if(jj==-1) {fprintf(stderr,"\n--- Error[%s]: no cluster %d in waveform file \n\n",thisprog,cluid2);exit(1);}
		// recalculate waveform mean
		aa= (double)wclun[ii];
		bb= (double)wclun[jj];
		cc= (double)(wclun[ii]+wclun[jj]);
		index1= ii*wtotlen;
		index2= jj*wtotlen;
		for(kk=0;kk<wtotlen;kk++) {
			wmean[index2+kk]= (float)((wmean[index1+kk]*aa + wmean[index2+kk]*bb) / cc );
			wmean[index1+kk]= NAN;
		}
		wclun[jj]+= wclun[ii];
		wclun[ii]= 0;
	}

	/********************************************************************************
	OUTPUT MODIFIED CLU FILE
	*********************************************************************************/
	z= xf_writebin2_v(outfileclub,(void *)club1,spiketot,sizeof(short),message);
	if(z<0) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }
	if(setverb==1) printf("	- new cluster id's sent to %s\n",outfileclub);

	/************************************************************
	OUTPUT MODIFIED .WFM FILE
	- in effect only the spike-counts (wclun) are altered
	- xf_writewave1_f only outputs a cluster if wclun[id]>0
	************************************************************/
	/* set the values for the header etc  */
	result_l[0]=wnclust; // number of rows in the array - unchanged, even if clusters were killed
	result_l[1]=wnchans;
	result_l[2]=wspklen;
	result_l[3]=wspkpre;
	result_l[4]=wprobe;
	/* write the new waveform file */
	z= xf_writewave1_f(outfilewfm,wid,wclun,wmean,wlistc,result_l,0,wsfreq,message);
	if(z!=0) { fprintf(stderr,"\b\n\t*** %s\n\n",message); exit(1); }
	if(setverb==1) printf("	- new waveforms sent to %s\n",outfilewfm);

	free(clubt1);
	free(club1);
	free(clun);
	free(clubt2);
	free(club2);
	free(wmean);
	free(wmeantemp);
	free(wavecorchan);
	free(wid);
	free(wlistc);
	free(wclun);
	free(indexarray);
	free(cluarray1);
	free(cluarray2);

	exit(0);
}
