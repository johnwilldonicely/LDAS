#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define thisprog "xe-ldas2-pathstats2"
#define TITLE_STRING thisprog" v 3: 14.August.2012 [JRH]"
#define MAXLINELEN 1000


/*
<TAGS>signal_processing  behaviour</TAGS>

v 3: 14.August.2012 [JRH]
	- bugfix - changed use of fscanf to read data with fgets/sscanf, to avoid problems related to "-" and "."
*/


/* external functions start */
float xf_geom_dist1(float x1,float y1,float x2,float y2);
/* external functions end */

int main (int argc, char *argv[]) {
	/* general variables */
	char infile[256],outfile[256],line[MAXLINELEN],templine[MAXLINELEN],word[MAXLINELEN],*matchstring=NULL,*pline,*pcol;
	long int i,j,k;
	int v,w,x,y,z,n,col,colmatch;
	int sizeofchar=sizeof(char),sizeofshort=sizeof(short),sizeoflong=sizeof(long),sizeofint=sizeof(int),sizeoffloat=sizeof(float),sizeofdouble=sizeof(double);
	float a,b,c,d,resultf[64]; 
	double aa,bb,cc,dd,resultd[64];
	FILE *fpin,*fpout;
	/* program-specific variables */ 
	char posfile[256],winfile[256];
	long int npos=0,nwins=0,pos1,pos2;
	float *posx=NULL,*posy=NULL,*posd=NULL,distoptimal,distactual;
	double *postime=NULL,*winstart=NULL,*winend=NULL;
	/* arguments */
	double setwin=0.4;
	
	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<3) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Calculate statistics on paths defined by time windows\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [position data] [winfile] [options]\n",thisprog);
		fprintf(stderr,"	[position data]: filename or \"stdin\", format: <time><x><y>\n");
		fprintf(stderr,"	[winfile]: file listing time windows - format <start><stop>\n");
		fprintf(stderr,"VALID OPTIONS (defaults in []):\n");
		fprintf(stderr,"	-int: time (s) over which to integrate movement distance [%g]\n",setwin);
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	sprintf(posfile,"%s",argv[1]);
	sprintf(winfile,"%s",argv[2]);
	for(i=3;i<argc;i++) {
		if( *(argv[i]+0) == '-') {
			if(i>=argc) break;
			else if(strcmp(argv[i],"-int")==0) 	{ setwin=atof(argv[i+1]); i++;}
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
		if(colmatch!=0 || !isfinite(aa) || !isfinite(a) || !isfinite(b)) continue;

 		postime=(double *)realloc(postime,(npos+1)*sizeofdouble); if(postime==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
		posx=(float *)realloc(posx,(npos+1)*sizeoffloat); if(posx==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
		posy=(float *)realloc(posy,(npos+1)*sizeoffloat); if(posy==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
		posd=(float *)realloc(posd,(npos+1)*sizeoffloat); if(posd==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
		postime[npos]=aa; 
		posx[npos]=a;
		posy[npos]=b;
		posd[npos]=0.00;
		npos++;
	}
	if(strcmp(posfile,"stdin")!=0) fclose(fpin);

	//for(i=0;i<10;i++) printf("%g	%g	%g	%g\n",postime[i],posx[i],posy[i],posd[i]); printf("\n\n");;//???
	
	/* CALCULATE DISTANCE TRAVELLED FOR EACH POSITION SAMPLE: i= current sample, j= look-forward sample, j-i >= sample integration period */
	setwin=setwin-0.0001;
	for(i=0;i<npos;i++) { 
		for(j=i+1;j<npos;j++) {
			if(postime[j]-postime[i]>=setwin) {
				a= xf_geom_dist1(posx[i],posy[i],posx[j],posy[j]) / (float)(j-i); /* total distance travelled divided by number of samples */
				k=(int)((float)(i+j)/2.0); /* mid-sample for this window */
				posd[k]=a; 
				//printf("%d %d	%.3f		%.2f	%.2f	%.2f	%.2f		%g	%d\n",i,j,(postime[j]-postime[i]),posx[i],posy[i],posx[j],posy[j],a,k);
				break;
	}}}

	//for(i=0;i<10;i++) printf("%g	%g	%g	%g\n",postime[i],posx[i],posy[i],posd[i]); exit(0);//???


	/* STORE WINDOWS */
	if((fpin=fopen(winfile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,winfile);exit(1);}
	nwins=0; 
	while(fgets(line,MAXLINELEN,fpin)!=NULL) {
		if(sscanf(line,"%lf %lf",&aa,&bb)!=2) continue;
		winstart= (double *) realloc(winstart,(nwins+1)*sizeofdouble);
		winend= (double *) realloc(winend,(nwins+1)*sizeofdouble);
		if(winstart==NULL || winend==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
		winstart[nwins]=aa; 
		winend[nwins]=bb; 
		nwins++;
	}
	fclose(fpin);

	/* ANALYZE THE PATHS DEFINED BY EACH WINDOW */
	printf("path	start	stop	d-opt	d-act	effic\n");
	
	for(i=0;i<nwins;i++) {

		pos1=pos2=-1; 
		distoptimal=distactual=0.00;
		
		for(j=0;j<npos;j++) if(postime[j]>=winstart[i]) {
			pos1=j; 
			break;
		}
	
		for(j=(pos1+1);j<npos && postime[j]<winend[i];j++) {
				pos2=j; if(posd[j]>0.0) distactual+= posd[j];
		}
	
		distoptimal=xf_geom_dist1(posx[pos1],posy[pos1],posx[pos2],posy[pos2]);
	
		printf("%d	%.3f	%.3f	%.3f	%.3f	%.3f	\n",i,postime[pos1],postime[pos2],distoptimal,distactual,(distoptimal/distactual));
	}
	
	free(postime); 
	free(posx); 
	free(posy); 
	free(posd); 
	free(winstart); 
	free(winend); 
	exit(0);
}
	
	
