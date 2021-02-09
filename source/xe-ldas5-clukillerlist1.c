#define thisprog "xe-ldas5-clukillerlist1"
#define TITLE_STRING thisprog" v 1: 3.April.2017 [JRH]"

#define MAXLINELEN 1000
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/*<TAGS>LDAS dt.spikes</TAGS>*/

/*
v 1: 3.April.2017 [JRH]
	- add option to remove cluster-zero after killing bad clusters (-kz)
v 1: 28.February.2017 [JRH]
	- bugfix: logic for detecting if zero needs to be added to .wfm output
	- bugfix in xf_writewave1_f: was using a depreciated variable for the number of NANs to be filled in on a waveform row
v 1: 19.February.2017 [JRH]
	- add option to read (and save) .wfm file
v 1: 17.February.2017 [JRH]
	- completed version and renamed using ldas5
	- also maybe add option to completely remove spikes from bad clusters
*/

/* external functions start */
long *xf_lineparse2(char *line,char *delimiters, long *nwords);
long xf_readclub1(char *infile1, char *infile2, long **clubt, short **club, char *message);
double xf_readwave1_f(char *infile, short **id, long **count, float **data, short **wchans, long *resultl, char *message);
int xf_writebin2_v(char *outfileclub, void *data0, size_t nn, size_t datasize, char *message);
int xf_writewave1_f(char *outfile, short *wid, long *wn, float *wmean, short *wchans, long *result_l, long makez, double srate, char *message);
/* external functions end */

int main (int argc, char *argv[]) {


	/* general variables */
	char line[256],*pline,*pcol,message[MAXLINELEN];
	int w,x,y,z;
	long ii,jj,kk,mm,nn,result_l[16];
	float a,b,c;
	double aa,bb,cc;
	FILE *fpin,*fpout;

	/* program-specific variables */
	char outfileclubt[64],outfileclub[64],outfilewfm[64];
	short *wid=NULL,*wchans=NULL,wzero;
	long *wclun=NULL, *wavecorchan=NULL, wnchans, wspklen, wspkpre, wnclust, wtotlen, wclumax, wprobe;
	float *wmean=NULL, wavemin,wavemax,wavecor;
	double wsfreq;

	short *club=NULL,*killflag=NULL,clumax=0;
	long *clubt=NULL,*clun=NULL,spiketot,nclust;
	long *listindex=NULL;

	/* arguments */
	char *infile1,*infile2,*setlist,*setfilewave=NULL;
	int setkz=0;

	sprintf(outfileclubt,"temp_%s.clubt",thisprog);
	sprintf(outfileclub,"temp_%s.club",thisprog);
	sprintf(outfilewfm,"temp_%s.wfm",thisprog);

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<4) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Reallocates cluster ID's in a .club file\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [infile1] [infile2] [list] [options]\n",thisprog);
		fprintf(stderr,"	[infile1]: .clubt timestamp file\n");
		fprintf(stderr,"	[infile2]: .club cluster-id file\n");
		fprintf(stderr,"	[list]: comma-separated list of cluster-id's to kill\n",thisprog);
		fprintf(stderr,"VALID OPTIONS, defaults in []:\n");
		fprintf(stderr,"	-w: specify a waveform file to also be updated [%s]\n",setfilewave);
		fprintf(stderr,"	-kz: remove cluster zero (0=NO 1=YES) [%d]\n",setkz);
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s data.clubt data.club 4,13,27\n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	modified .clubt file (%s)\n",outfileclubt);
		fprintf(stderr,"	modified .club file (%s)\n",outfileclub);
		fprintf(stderr,"	modified .wfm file (%s)\n",outfilewfm);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/************************************************************
	READ THE FILENAME AND OPTIONAL ARGUMENTS
	************************************************************/
	infile1=argv[1];
	infile2=argv[2];
	setlist=argv[3];
	for(ii=4;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-w")==0)  setfilewave=argv[++ii];
			else if(strcmp(argv[ii],"-kz")==0) setkz= atoi(argv[++ii]);
			else {fprintf(stderr,"\n*** %s [ERROR: invalid command line argument \"%s\"]\n\n",thisprog,argv[ii]); exit(1);}
	}}
	if(setkz!=0&&setkz!=1) {fprintf(stderr,"\n--- Error[%s]: invalid -kz (%d), must be 0 or 1\n\n",thisprog,setkz); exit(1);}


	/************************************************************
	STORE THE CLUSTER DATA
	************************************************************/
	/* read the clubt and club files */
 	spiketot= xf_readclub1(infile1,infile2,&clubt,&club,message);
 	if(spiketot<0) {fprintf(stderr,"\n*** %s/%s\n\n",thisprog,message); exit(1);}
	/* find highest cluster id */
	clumax=-1; for(ii=0;ii<spiketot;ii++) if(club[ii]>clumax) clumax=club[ii];
	/* calculate spike-count in each cluster */
	clun= calloc((clumax+1),sizeof *clun);
	if(clun==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n",thisprog);exit(1);}
	for(ii=0;ii<spiketot;ii++) clun[club[ii]]++;
	//TEST:printf("spiketot=%ld\n",spiketot); for(ii=0;ii<=clumax;ii++) if(clun[ii]>0) printf("clun[%ld]=%ld\n",ii,clun[ii]); exit(0);
	/* determine the total number of populated clusters */
	nclust=0; for(ii=0;ii<=clumax;ii++) if(clun[ii]>0) nclust++;

	/********************************************************************************
	READ THE OPTIONAL .WFM FILE
	********************************************************************************/
	if(setfilewave!=NULL) {
		wsfreq = xf_readwave1_f(setfilewave,&wid,&wclun,&wmean,&wchans,result_l,message);
		if(wsfreq<0) {fprintf(stderr,"\nError[%s]: %s\n\n",thisprog,message); exit(1); }
		wnclust= result_l[0]; // total clusters, not the same as max_cluster_id (result_l[5])
		wnchans= result_l[1]; // number of channels per combined-waveform
		wspklen= result_l[2]; // number of samples per channel
		wspkpre=result_l[3];  // the number of samples preceding the peak
		wtotlen=result_l[4];  // total length of combined-waveform
		wclumax=result_l[5];  // largest cluster-number
		wprobe=result_l[6];   // probe number
		/* check maximum-cluster-id's match in club and wfm records */
		if(wclumax!=clumax) {fprintf(stderr,"\n--- Error[%s]: max cluster in waveform file (%ld) does not match the club file (%ld)\n\n",thisprog,wclumax,clumax);exit(1);}
		/* check that the total number of populated clusters also matches */
		if(wnclust!=nclust) {fprintf(stderr,"\n--- Error[%s]: no. of clusters in waveform file (%ld) does not match the club file (%ld)\n\n",thisprog,wnclust,nclust);exit(1);}
		/* compare spike counts for each cluster  */
		/* also check if a cluster-zero is defined */
		wzero=0;
		for(ii=0;ii<wnclust;ii++) {
			z=wid[ii];
			if(wclun[ii]!=clun[z]) {fprintf(stderr,"\n--- Error[%s]: cluster[%ld] count in waveform file (%ld) file does not match the club file (%ld)\n\n",thisprog,ii,wclun[ii],clun[z]);exit(1);}
			if(z==0) wzero=1;
		}
	}

	/************************************************************
	BUILD THE CLUSTER KILL-FLAG LIST
	************************************************************/
	killflag= calloc((clumax+1), sizeof *killflag);
	if(killflag==NULL) {{fprintf(stderr,"\n--- Error[%s]: memory allocation error\n\n",thisprog);exit(1);}}
	listindex= xf_lineparse2(setlist,",",&mm);
	for(ii=0;ii<mm;ii++) killflag[atoi(setlist+listindex[ii])]=1;
	//TEST:	for(ii=0;ii<clumax;ii++) printf("killflag[%ld]=%d\n",ii,killflag[ii]);exit(0);

	/************************************************************
	REASIGN CLUSTER IDS TO ZERO
	************************************************************/
	for(ii=0;ii<spiketot;ii++) if(killflag[club[ii]]==1) club[ii]=0;
	/* adjust club spike counts (except cluster 0) */
	for(ii=1;ii<=clumax;ii++) { if(killflag[ii]==1) { clun[0]+=clun[ii]; clun[ii]=0; }}
	//TEST:	for(ii=0;ii<=clumax;ii++) if(clun[ii]>0) printf("clun[%ld]=%ld\n",ii,clun[ii]);exit(0);

	/************************************************************
	REMOVE CLUSTER ZERO
	************************************************************/
	if(setkz==1) {
		for(ii=jj=0;ii<spiketot;ii++) {
			if(club[ii]!=0) {
				clubt[jj]= clubt[ii];
				club[jj]=  club[ii];
				jj++;
			}
		}
		if(jj<1) {fprintf(stderr,"\n--- Error[%s]: killing cluster zero removed all spikes - no output\n\n",thisprog);exit(1);}
		spiketot=jj;
		clun[0]=0;
	}

	/************************************************************
	CREATE NEW CLUB(T) FILES
	************************************************************/
	z= xf_writebin2_v(outfileclubt,(void *)clubt,spiketot,(sizeof *clubt),message);
	if(z!=0) { fprintf(stderr,"\n\t--- %s\n\n",message); exit(1); }
	z= xf_writebin2_v(outfileclub,(void *)club,spiketot,(sizeof *club),message);
	if(z!=0) { fprintf(stderr,"\n\t--- %s\n\n",message); exit(1); }

	/************************************************************
	IF SPECIFIED, MODIFY WAVEFORMS AND CREATE NEW .WFM FILE
	- in effect only the spike-counts (wclun) are altered
	- xf_writewave1_f only outputs a cluster if wclun[]>0
	- note that if cluster-zero is removed (-kz 1) then wclun will become zero
	************************************************************/
	if(setfilewave!=NULL) {
		/* adjust waveform spike-counts to match club records */
		for(ii=0;ii<wnclust;ii++) wclun[ii]=clun[wid[ii]];
		/* set the values for the header etc  */
		result_l[0]=wnclust; // number of rows in the array - unchanged, even if clusters were killed
		result_l[1]=wnchans;
		result_l[2]=wspklen;
		result_l[3]=wspkpre;
		result_l[4]=wprobe;
		/* determine if a new row for cluster-zero needs to be generated: kk= 0(no)  or 1(yes) */
		if(wzero==0 && clun[0]>0) kk=clun[0]; else kk=0;
		/* write the new waveform file */
		z= xf_writewave1_f(outfilewfm,wid,wclun,wmean,wchans,result_l,kk,wsfreq,message);
		if(z!=0) { fprintf(stderr,"\n\t--- %s\n\n",message); exit(1); }
	}

	/************************************************************
	CLEANUP AND EXIT
	************************************************************/
	free(club);
	free(clubt);
	free(clun);
	free(killflag);
	free(wmean);
	free(wid);
	free(wchans);
	free(wclun);

	exit(0);
	}
