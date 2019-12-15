#define thisprog "xe-distvel3"
#define TITLE_STRING thisprog" v 2: 2.August.2012 [JRH]"
#define MAXLINELEN 1000

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/*
<TAGS>signal_processing math</TAGS>
v 2: 2.August.2012 [JRH]
	- replace "-" with "nan" for missing values

*/

/* external functions start */
float xf_geom_dist1(float x1,float y1,float x2,float y2);
/* external functions end */

int main (int argc, char *argv[]) {
	/* general variables */
	char infile[256],outfile[256],line[MAXLINELEN],templine[MAXLINELEN],word[256],*matchstring=NULL,*pline,*pcol;
	long int i,j,k;
	int v,w,x,y,z,n,col,colmatch;
	int sizeofchar=sizeof(char),sizeofshort=sizeof(short),sizeoflong=sizeof(long),sizeofint=sizeof(int),sizeoffloat=sizeof(float),sizeofdouble=sizeof(double);
	float a,b,c,d,resultf[64];
	double aa,bb,cc,dd,resultd[64];
	FILE *fpin,*fpout;
	/* program-specific variables */
	char posfile[256],winfile[256];
	int *posgood=NULL;
	long int npos=0,nwins=0,pos1,pos2;
	float *posx=NULL,*posy=NULL,*posd=NULL,*posv=NULL;
	double *postime=NULL;
	/* arguments */
	double setwin=0.4;

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Calculate distance travelled and velocity in a time-x-y series\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [position data] [options]\n",thisprog);
		fprintf(stderr,"	[position data]: filename or \"stdin\", format: <time><x><y>\n");
		fprintf(stderr,"VALID OPTIONS (defaults in []):\n");
		fprintf(stderr,"	-t: time (s) over which to integrate movement distance [%g]\n",setwin);
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"OUTPUT: \n");
		fprintf(stderr,"	a series of distance-travelled and velocity values \n");
		fprintf(stderr,"NOTES:\n");
		fprintf(stderr,"	- output is for last sample in each integration window\n");
		fprintf(stderr,"	- one line output for each non-blank line of input\n");
		fprintf(stderr,"	- blank lines will cause misalignment with original data series\n");
		fprintf(stderr,"	- invlid times or x/y values result in \"-1 -1\" output\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	sprintf(posfile,"%s",argv[1]);
	for(i=2;i<argc;i++) {
		if( *(argv[i]+0) == '-') {
			if(i>=argc) break;
			else if(strcmp(argv[i],"-t")==0) 	{ setwin=atof(argv[i+1]); i++;}
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[i]); exit(1);}
	}}

	/* STORE POSITION DATA */
	if(strcmp(posfile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(posfile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,posfile);exit(1);}
	npos=0;
	n=0;
	while(fgets(line,MAXLINELEN,fpin)!=NULL) {
		pline=line; colmatch=3; // number of columns to match
		for(col=1;(pcol=strtok(pline," ,\t\n\r"))!=NULL;col++) {
			pline=NULL;
			if(col==1 && sscanf(pcol,"%lf",&aa)==1) colmatch--;
			if(col==2 && sscanf(pcol,"%f",&a)==1) colmatch--;
			if(col==3 && sscanf(pcol,"%f",&b)==1) colmatch--;
		}
 		postime=(double *)realloc(postime,(npos+1)*sizeofdouble); if(postime==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
		posx=(float *)realloc(posx,(npos+1)*sizeoffloat); if(posx==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
		posy=(float *)realloc(posy,(npos+1)*sizeoffloat); if(posy==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
		posd=(float *)realloc(posd,(npos+1)*sizeoffloat); if(posd==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
		posv=(float *)realloc(posv,(npos+1)*sizeoffloat); if(posv==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
		posgood=(int *)realloc(posgood,(npos+1)*sizeofint); if(posgood==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}

		if(colmatch==0 && isfinite(aa) && isfinite(a)  && isfinite(b)) {
			postime[npos]=aa;
			posx[npos]=a;
			posy[npos]=b;
			posd[npos]=NAN;
			posv[npos]=NAN;
			posgood[npos]=1;
		}
		else {
			postime[npos]=aa; posx[npos]=posy[npos]=posd[npos]=posv[npos]=NAN; posgood[npos]=0;
		}
		npos++;
	}
	if(strcmp(posfile,"stdin")!=0) fclose(fpin);

	/* CALCULATE DISTANCE TRAVELLED FOR EACH POSITION SAMPLE: i= current sample, j= look-forward sample, j-i >= sample integration period */
	setwin=setwin-0.0001;
	for(i=1;i<npos;i++) {
		for(j=(i-1);j>=0;j--) {
			aa=postime[i]-postime[j];
			if(aa>=setwin) {
				if(posgood[i]==1 && posgood[j]==1) {
					/* total distance travelled divided by number of samples */
					a= xf_geom_dist1(posx[i],posy[i],posx[j],posy[j]) / (float)(i-j);
					posd[i]=a;
					posv[i]=(float)((double)a/aa);
					break;
				}
	}}}

	for(i=0;i<npos;i++) printf("%g	%g\n",posd[i],posv[i]);

	free(postime);
	free(posx);
	free(posy);
	free(posd);
	free(posv);
	free(posgood);
	exit(0);
}
