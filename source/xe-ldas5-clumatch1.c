#define thisprog "xe-ldas5-clumatch1"
#define TITLE_STRING thisprog" v 1: 2.June.2017 [JRH]"
#define MAXLINELEN 1000
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <sys/stat.h>  /* this and next header allow testing file exists using stat() function */
#include <errno.h>

/* <TAGS>LDAS dt.spikes</TAGS> */

/************************************************************************
v 1: 1.June.2017 [JRH]
	- new prog: match clusters from different clustering runs based on timestamps
*************************************************************************/

/* external functions start */
long *xf_lineparse2(char *line,char *delimiters, long *nwords);
long xf_readssp1(char *infile, long **start, long **stop, char *message);
off_t xf_readbin2_v(FILE *fpin, void **data, off_t startbyte, off_t bytestoread, char *message);
long xf_readclub1(char *infile1, char *infile2, long **clubt, short **club, char *message);
void xf_qsortindex1_l(long *data, long *index,long n);
/* external functions end */

int main (int argc, char *argv[]) {

	/* general variables */
	char *infile1,*infile2,*infile3,*infile4,message[256];
	int v,w,x,y,z,sizeofshort=sizeof(short),sizeoflong=sizeof(long);
	long int ii,jj,kk,mm,nn;
	float a,b,c,d;
	double aa,bb,cc,dd,result_d[64];
	FILE *fpin,*fpout1,*fpout2;
	struct stat sts;

	/* program-specific variables */
	short *club1=NULL,*club2=NULL;
	long *clubt1=NULL,*clubt2=NULL,*clucount2=NULL,*clucount1=NULL,*clumatch=NULL;
	long *sort1=NULL,*sort2=NULL;
	long clumax1,clumax2,diff,start2,stop2,tref,index1,mindiff,mindiffindex;
	off_t nlist,nclu;

	/* arguments */
	char *setscreenfile=NULL,*setscreenlist=NULL,*setclulist=NULL;
	int setout=-1,setverb=0;
	long setmaxdiff=0;
	nlist=0;

	/************************************************************
	PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED
	************************************************************/
	if(argc<5) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Match clusters from different clustering runs based on timestamps\n");
		fprintf(stderr,"- how are reference cluster spikes found in comparison clusters?\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [clubt1] [club1] [clubt2] [club2] [options]\n",thisprog);
		fprintf(stderr,"	[clubt1]: reference .clubt file\n");
		fprintf(stderr,"	[club1]:  reference .club file\n");
		fprintf(stderr,"	[clubt2]: comparison .clubt file\n");
		fprintf(stderr,"	[club2]:  comparison .club file\n");
		fprintf(stderr,"VALID OPTIONS:\n");
		fprintf(stderr,"	-diff: max offset (samples) between matched timestamps [%ld]\n",setmaxdiff);
		fprintf(stderr,"	-verb: set verbocity of output (0=low, 1=high) [%d]\n",setverb);
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s A.clubt A.club B.clubt B.club -diff 5\n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	[c0] [n] [p] [x1:m1] [x2:m2] [x3:m3] ... etc\n");
		fprintf(stderr,"		[c0]: reference cluster ID\n");
		fprintf(stderr,"		[n]: spike count for reference cluster\n");
		fprintf(stderr,"		[p]: proportion of [n] found in comparison cluster\n");
		fprintf(stderr,"			- NOTE: typically 0-1, but may be >1\n");
		fprintf(stderr,"		x+: comparison cluster ID\n");
		fprintf(stderr,"		m+: comparison cluster timestamps matching [c0]\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}


	/************************************************************
	READ THE FILENAMES AND OPTIONAL ARGUMENTS
	************************************************************/
	infile1=argv[1];
	infile2=argv[2];
	infile3=argv[3];
	infile4=argv[4];
	for(ii=5;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-diff")==0) setmaxdiff=atol(argv[++ii]);
			else if(strcmp(argv[ii],"-verb")==0) setverb=atoi(argv[++ii]);
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
	}}
	if(setverb<0 || setverb>1) {fprintf(stderr,"\n--- Error[%s]: invalid -verb (%d) : must be 0-1\n\n",thisprog,setverb);exit(1);}



	/* FIRST REPORT */
	if(setverb==1) {
		fprintf(stderr,"reading input files...\n");
		fprintf(stderr,"	reference clubt file: %s\n",infile1);
		fprintf(stderr,"	reference lub  file: %s\n",infile2);
		fprintf(stderr,"	comparison clubt file: %s\n",infile3);
		fprintf(stderr,"	ccomparison lub  file: %s\n",infile4);
	}

	/************************************************************
	READ THE CLUSTER TIMESTAMPS AND IDs
	***********************************************************/
	nn= xf_readclub1(infile1,infile2,&clubt1,&club1,message);
 	if(nn<0) {fprintf(stderr,"\n*** %s/%s\n\n",thisprog,message); exit(1);}

	mm= xf_readclub1(infile3,infile4,&clubt2,&club2,message);
 	if(nn<0) {fprintf(stderr,"\n*** %s/%s\n\n",thisprog,message); exit(1);}

	/************************************************************
	CALCULATE THE MAXIMUM CLUSTRE-ID IN THER REFERENCE AND COMPARISON DATA
	***********************************************************/
	clumax1=-1; for(ii=0;ii<nn;ii++) if(club1[ii]>clumax1) clumax1=club1[ii];
	clumax2=-1; for(ii=0;ii<mm;ii++) if(club2[ii]>clumax2) clumax2=club2[ii];
	if(setverb==1) {
		fprintf(stderr,"	clumax1= %ld\n",clumax1);
		fprintf(stderr,"	clumax2= %ld\n",clumax2);
	}

	/************************************************************
	ALLCOATE MEMORY FOR THE CLUMATCH RECORDS
	- this is a matrix of size clumax1*clumax2
	***********************************************************/
	kk= (clumax1+1)*(clumax2+1);
	if((clumatch=calloc(kk,sizeof *clumatch))==NULL) {fprintf(stderr,"\n--- Error [%s]: insufficient memory\n\n",thisprog); exit(1);};
	if((sort1=calloc(clumax2+1,sizeof *sort1))==NULL) {fprintf(stderr,"\n--- Error [%s]: insufficient memory\n\n",thisprog); exit(1);};
	if((sort2=calloc(clumax2+1,sizeof *sort2))==NULL) {fprintf(stderr,"\n--- Error [%s]: insufficient memory\n\n",thisprog); exit(1);};

	/************************************************************
	CLU-COUNTS FOR REFERENCE CLUSTERS
	- allows empty clusters to be skipped
	***********************************************************/
	if(setverb==1) fprintf(stderr,"	allocating memory...\n");
	if((clucount1=calloc(clumax1+1,sizeof *clucount1))==NULL) {fprintf(stderr,"\n--- Error [%s]: insufficient memory\n\n",thisprog); exit(1);};
	if((clucount2=calloc(clumax2+1,sizeof *clucount2))==NULL) {fprintf(stderr,"\n--- Error [%s]: insufficient memory\n\n",thisprog); exit(1);};
	for(ii=0;ii<nn;ii++) clucount1[club1[ii]]++;
	for(ii=0;ii<mm;ii++) clucount2[club2[ii]]++;

	/************************************************************
	MATCH THE CLUSTERS
	***********************************************************/
	if(setverb==1) fprintf(stderr,"	matching clusters...\n");
	/* for every reference timestamp... */
	start2=0;
	for(ii=0;ii<nn;ii++) {
		/* ...find matching comparison timestamps */
		for(jj=start2;jj<mm;jj++) {
			/* test timestamp difference - break if too far, reset start and continue if not far enough */
			diff= clubt2[jj]-clubt1[ii];
			if(diff>setmaxdiff) break;
			if(diff<-setmaxdiff) { start2= jj+1; continue; } // current sample cannot be useful for any other reference events

			/* otherwise, calculate the current time-difference */
			mindiff= diff;
			mindiffindex= jj;
			/* scan forward to see if there are any closer timestamps */
			tref= clubt1[ii];
			//start2= jj; // current sample MIGHT be useful for other referecnce events - maintain start-point
			stop2= jj+(2.0*setmaxdiff);
			if(stop2>mm) stop2= mm;
			for(kk=start2;kk<stop2;kk++) {
				diff= clubt2[jj]-tref;
				if(diff<0) diff= 0-diff;
				if(diff<mindiff) { mindiff=diff; mindiffindex=kk; }
			}
			/* calculate the index to the cluster pair */
			index1= club1[ii]*clumax2 + club2[mindiffindex];
			/* increment the match-count */
			clumatch[index1]++;
		} // end of loop - comparison timestamps
	} // end of loop: reference timestamps



	/************************************************************
	REPORT RESULTS
	- format: [refcluster] [count] [c1:n1] [c2:n2] [c3:n3] ... etc
		- [c?:n?]:
		 	c?= reference cluster ID
			n?= no. timestamps matching [refcluster]
	***********************************************************/
	if(setverb==1) fprintf(stderr,"	reporting...\n");
	for(ii=0;ii<=clumax1;ii++) {
		if(clucount1[ii]<1) continue;

		printf("%ld\t%ld",ii,clucount1[ii]);
		kk=0; // reset the counter for the number of good cluster-2 matches
		mm=0; // re-use clu2 counter to store the total matches
		for(jj=0;jj<clumax2;jj++) {
			index1= ii*clumax2 + jj;
			if(clumatch[index1]<1) continue;
			sort1[kk]= jj; // temporary array holding current cluster-2
			sort2[kk]= clumatch[index1]; // temporary array holding match-count
			mm+= clumatch[index1]; // talley total spike-matches
			kk++;  // increment counter for good cluster-2 matches
		}

		/* print the percentage matched */
		printf("\t%.3f",(double)mm/(double)clucount1[ii]);

		/* sort cluster-2 based on number of matches */
		xf_qsortindex1_l(sort2,sort1,kk);

		/* print the sorted matches in reverse order */
		for(jj=(kk-1);jj>=0;jj--) printf("\t%ld:%ld",sort1[jj],sort2[jj]);
		printf("\n");
	}




	if(club1!=NULL) free(club1);
	if(club2!=NULL) free(club2);
	if(clubt1!=NULL) free(clubt1);
	if(clubt2!=NULL) free(clubt2);
	if(clucount1!=NULL) free(clucount1);
	if(clucount2!=NULL) free(clucount2);
	if(clumatch!=NULL) free(clumatch);
	if(sort1!=NULL) free(sort1);
	if(sort2!=NULL) free(sort2);

 	exit(0);

}
