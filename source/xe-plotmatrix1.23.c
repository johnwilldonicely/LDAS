#define thisprog "xe-plotmatrix1"
#define TITLE_STRING thisprog" v 23: 2.April.2020 [JRH]"
#define MAXWORDLEN 1000

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>

/*
<TAGS>plot dt.matrix</TAGS>

TO DO:
	???: incorporate simplified xtlaff/ytaloff definition and axis-functions-definitions as per xe-plottable1 (1.January.2019)
	???: incoporate updated min/max/range calculation as per xe-plottable1 (24.November.2018, 30.September.2018)

v 23: 2.April.2020 [JRH]
	- use new palette-generating function xf_palette7 to create colour scales and to add additional palettes from the viridis collection (viridis,plasma,magma,inferno)
v 23: 10.March.2020 [JRH]
	- update to allow setting colour-ranges which represent a percentile
v 23: 1.January.2019 [JRH]
	- correct mis-printing of tics due to floating-point imprecision
		- for integers, use %ld format instead %.0f
v 23: 28.November.2018 [JRH]
	- simplify variables used for setting x- and y-ranges
v 23: 26.February.2018 [JRH]
	- bugfix and improve definition of z-range
v 23: 25.February.2018 [JRH]
	- bugfix: failure to initialize *line variable had been causng segmentation faults
v 21: 22.February.2018 [JRH]
	- bugfix: update user-lines definitions
	- update variable formats
	- update assignment of infile,xlabel,ylabel,title (avoid sprintf)
	- update memory allocation & free
v 21: 6.February.2018 [JRH]
	- bugfix: only define user-lines if sethline and/or setvline are "1"
v 21: 16.October.2017 [JRH]
	- really fix tics-omission code
	- fix print-to-string (use snprintf() and MAXWORDLEN)
	- fix estimate of tic-label-size to account for small size of "decimal"
	- avoid definition of bigtic an xaloff/yaloff in the early portion of output
	- title now always aligns to edge of plot, not edge of y-axis-labels
v 21: 15.October.2017 [JRH]
	- bugfix xint/yint settings  -now omission of tics actually works
v 21: 9.January.2017 [JRH]
	- bugfix decimal precision calculation by xf_precision_d
		- needed to allow max decimal precision due to issues precisely representing floating-point numbers

...

v 2: 19.July.2012 [JRH]
	- add zmin/max settings and zclip option
*/

/* external functions start */
double xf_strtod1(char *str);
char *xf_lineread1(char *line, long *maxlinelen, FILE *fpin);
long *xf_lineparse2(char *line,char *delimiters, long *nwords);
double xf_trimdigits_d(double number_to_trim, int digits_to_keep);
char* xf_strsub1 (char *source, char *str1, char *str2);
int xf_compare1_d(const void *a, const void *b);
int xf_precision_d(double number, int max);
long xf_interp3_f(float *data, long ndata);
double xf_percentile2_d(double *data, long nn, double setper, char *message);
int xf_palette7(float *red, float *green, float *blue, long nn, char *palette);
/* external functions end */

int main (int argc, char *argv[]) {

	/* GENERAL VARIABLES */
	char *line=NULL,*templine=NULL,*pline=NULL,*pcol=NULL,message[256];
	long ii,jj,kk,nn,col;
	int v,w,x,y,z;
	float a,b,c,d,e;
	double aa,bb,cc,dd,ee,result_d[64];
	FILE *fpin,*fpout;

	/* PROGRAM-SPECIFIC VARIABLES */
	char *xlabel=NULL,*ylabel=NULL,*plottitle=NULL,*bigtic=NULL;
	int xticprecision,yticprecision,sizeofx,sizeofy,sizeofz,sizeofncols,zminper=0,zmaxper=0;
	long *index1=NULL,*index2=NULL,*ncols=NULL,nrows=0,colmax=0,maxlinelen=0,nhlines=0,nvlines=0;
	float xlimit=500.0,ylimit=500.0; // this is the postscript plotting space, in pixels
	float yticmaxchar;
	double *xdata=NULL,*ydata=NULL,*zdata=NULL,*wdata=NULL;
	double xmin=0.0,xmax=1.0,ymin=0.0,ymax=1.0,xrange,yrange,xfactor,yfactor,xint,yint;
	double blockwidth,blockheight,zmin,zmax,zmid,zrange;

	/* colour-palette variables */
	char *setrgbpal=NULL;
	long setrgbn=99;
	float *red=NULL,*green=NULL,*blue=NULL;

	/* ARGUMENTS */
	char *infile=NULL,outfile[MAXWORDLEN];
	int setxpad=0,setypad=0,setytrim=3;
	int framestyle=15, f1=0,f2=0,f3=0,f4=0;
	int setzclip=0,setyflip=0;
	int setblockwidth=1,setulinecolour=-1,setulinestyle=0;
	float xscale=.3,yscale=.3,setticsize=-3,fontsize=10.0,setbg=-1.0,zx=-1,zy=-1;
	float lwdata=1,lwaxes=1; // linewidth for drawing data and frame/tics
	double setxminval=NAN,setxmaxval=NAN,setyminval=NAN,setymaxval=NAN,setxstep=NAN,setystep=NAN,setxint=0,setyint=0,xpad=0.0,ypad=0.0;
	char *sethlines=NULL,*setvlines=NULL,*setzmintemp=NULL,*setzmaxtemp=NULL;
	double *hlines=NULL,*vlines=NULL;
	double setzmin=NAN,setzmax=NAN,setzmid=NAN;

	/* INITIALIZE VARIABLES */
	sprintf(outfile,"temp_%s.ps",thisprog);
	templine= malloc(MAXWORDLEN*sizeof(*templine));

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Produces a postscript \"heat-map\" plot of a matrix of data\n");
		fprintf(stderr,"If some rows of input have fewer values, the blocks on that row will \n");
		fprintf(stderr,"	be stretched to span the entire matrix\n");
		fprintf(stderr,"Non-numeric values will be red for greyscale plots, white otherwise\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [filename] [options]\n\n",thisprog);
		fprintf(stderr,"VALID OPTIONS (default in [])...\n");
		fprintf(stderr,"	[filename]: file name or \"stdin\"\n");
		fprintf(stderr,"	-bw: block-width fixed (0) or adaptive (1) [%d]\n",setblockwidth);
		fprintf(stderr,"		0 reduces file-size, 1 stretches blocks to fill rows\n");
		fprintf(stderr,"	-pal: colour palette (black=min, white=NAN): \n");
		fprintf(stderr,"		grey: darkgrey-lightgrey\n");
		fprintf(stderr,"		rainbow: blue-green-red\n");
		fprintf(stderr,"		viridis: purple-green-yellow\n");
		fprintf(stderr,"		plasma: blue-purple-yellow\n");
		fprintf(stderr,"		magma: black-purple-cream\n");
		fprintf(stderr,"		inferno: black-purple-orange-paleyellow\n");
		fprintf(stderr,"	-paln: number of colours in the palette [%ld]\n",setrgbn);
		fprintf(stderr,"	-xstep -ystep: set interval between matrix values [%g,%g]\n",setxstep,setystep);
		fprintf(stderr,"		- an alternative methods for data range-setting\n");
		fprintf(stderr,"		- if \"nan\", uses -x/ymin and -x/ymax instead\n");
		fprintf(stderr,"		- NOTE: works with -x/ymin, but overrides -x/ymax\n");
		fprintf(stderr,"	-title: plot title (enclose in quotes)\n");
		fprintf(stderr,"	-xlabel -ylabel: axis labels (1 word, use \"_\" for spaces)\n");
		fprintf(stderr,"	-xpad -ypad: pad between data range and plot axes\n");
		fprintf(stderr,"	-xint -yint: intervals for x[%g] and y[%g] tics (0=AUTO -1=OMIT)\n",setxint,setyint);
		fprintf(stderr,"	-xscale -yscale: scale plot in x [%g] and y [%g] dimensions\n",xscale,yscale);
		fprintf(stderr,"	-xmin -xmax -ymin -ymax: manually set data range\n");
		fprintf(stderr,"	-zmin -zmax: colour-range (NAN=auto) [%g,%g]\n",setzmin,setzmax);
		fprintf(stderr,"		- add %% (eg 95%%) to set as a percentile of z-range\n");
		fprintf(stderr,"	-zmid: set value for middle colour (non-numeric=auto) [%g]\n",setzmid);
		fprintf(stderr,"		- attempts to equalize range around mid-point\n");
		fprintf(stderr,"		- may override zmin,zmax or both\n");
		fprintf(stderr,"	-zclip: compress (0) or clip (1) out-of-range values [%d]\n",setzclip);
		fprintf(stderr,"	-yflip: flip matrix on y-axis (0=NO, 1=YES) [%d]\n",setyflip);
		fprintf(stderr,"	-font: base font size [%g]\n",fontsize);
		fprintf(stderr,"	-frame: draw frame at bottom(1) left(2) top(4) right(8) [%d]\n",framestyle);
		fprintf(stderr,"	-tics: size of x- and y-tics (negative=outside frame) [%g]\n",setticsize);
		fprintf(stderr,"	-hline: CSV position-list for horizontal dashed lines [unset]\n");
		fprintf(stderr,"	-vline: CSV position-list for vertical dashed lines [unset]\n");
		fprintf(stderr,"	-uc: user-line colour (0 to -paln, or -1 =white|yellow ) [%d]\n",setulinecolour);
		fprintf(stderr,"	-us: user-line style (0=dashed, 1=solid) [%d]\n",setulinestyle);
		fprintf(stderr,"	-lwd: line width for data [%g]\n",lwdata);
		fprintf(stderr,"	-zx -zy: page offset (-1= default A4 top-left) [%g,%g]\n",zx,zy);
		fprintf(stderr,"	-out: output file name [%s]\n",outfile);
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"OUTPUT: postscript file\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}


	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	infile= argv[1];
	for(ii=2;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			aa=xf_strtod1(argv[ii]+1);
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-xlabel")==0)  xlabel= argv[++ii];
			else if(strcmp(argv[ii],"-ylabel")==0)  ylabel= argv[++ii];
			else if(strcmp(argv[ii],"-title")==0)   plottitle= argv[++ii];
			else if(strcmp(argv[ii],"-out")==0)     sprintf(outfile,"%s",argv[++ii]);
			else if(strcmp(argv[ii],"-font")==0)    fontsize= atof(argv[++ii]);
			else if(strcmp(argv[ii],"-xscale")==0)  xscale=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-yscale")==0)  yscale=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-xstep")==0)   setxstep=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-ystep")==0)   setystep=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-xmin")==0)    setxminval=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-ymin")==0)    setyminval=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-xmax")==0)    setxmaxval=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-ymax")==0)    setymaxval=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-zmin")==0)    setzmintemp= argv[++ii];
			else if(strcmp(argv[ii],"-zmax")==0)    setzmaxtemp= argv[++ii];
			else if(strcmp(argv[ii],"-zmid")==0)    setzmid= atof(argv[++ii]);
			else if(strcmp(argv[ii],"-xpad")==0)    { xpad=atof(argv[++ii]); setxpad=1;}
			else if(strcmp(argv[ii],"-ypad")==0)    { ypad=atof(argv[++ii]); setypad=1;}
			else if(strcmp(argv[ii],"-xint")==0)    setxint=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-yint")==0)    setyint=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-zclip")==0)   setzclip= atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-yflip")==0)   setyflip= atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-hline")==0)   sethlines= argv[++ii];
			else if(strcmp(argv[ii],"-vline")==0)   setvlines= argv[++ii];
			else if(strcmp(argv[ii],"-lwd")==0)     lwdata= atof(argv[++ii]);
			else if(strcmp(argv[ii],"-lwa")==0)     lwaxes= atof(argv[++ii]);
			else if(strcmp(argv[ii],"-uc")==0)      setulinecolour= atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-us")==0)      setulinestyle= atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-frame")==0)   framestyle= atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-tics")==0)    setticsize= atof(argv[++ii]);
			else if(strcmp(argv[ii],"-zx")==0)      zx= atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-zy")==0)      zy= atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-pal")==0)      setrgbpal= argv[++ii];
			else if(strcmp(argv[ii],"-paln")==0)      setrgbn= atol(argv[++ii]);
			else if(strcmp(argv[ii],"-bw")==0)      setblockwidth= atoi(argv[++ii]);
			else {fprintf(stderr,"\n\a--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
	}}

	if(setblockwidth!=0 && setblockwidth!=1) {fprintf(stderr,"\n\a--- Error[%s]: illegal -bw (%d), should be 0 or 1\n\n",thisprog,setblockwidth);exit(1);}
	if(setzclip!=0 && setzclip!=1) {fprintf(stderr,"\n\a--- Error[%s]: illegal -zclip (%d), should be either 0 or 1\n\n",thisprog,setzclip);exit(1);}
	if(setyflip!=0 && setyflip!=1) {fprintf(stderr,"\n\a--- Error[%s]: illegal -yflip (%d), should be either 0 or 1\n\n",thisprog,setyflip);exit(1);}
	if(setulinecolour<-1 || setulinecolour>setrgbn) {fprintf(stderr,"\n\a--- Error[%s]: illegal -uc (%d), should be between -1 and %ld\n\n",thisprog,setulinecolour,setrgbn);exit(1);}
	if(setulinestyle!=0 && setulinestyle!=1) {fprintf(stderr,"\n\a--- Error[%s]: illegal -us (%d), should be 0 or 1\n\n",thisprog,setulinestyle);exit(1);}
	if(setrgbn<2) {fprintf(stderr,"\n\a--- Error[%s]: illegal -paln (%ld), should be at least 2\n\n",thisprog,setrgbn);exit(1);}
	if(setrgbpal==NULL) setrgbpal="rainbow";
	if(
		strcmp(setrgbpal,"grey")!=0
		&& strcmp(setrgbpal,"rainbow")!=0
		&& strcmp(setrgbpal,"viridis")!=0
		&& strcmp(setrgbpal,"plasma")!=0
		&& strcmp(setrgbpal,"magma")!=0
		&& strcmp(setrgbpal,"inferno")!=0
	) {fprintf(stderr,"\n\a--- Error[%s]: illegal -pal (%s)\n\n",thisprog,setrgbpal);exit(1);}

	if(xlabel==NULL) xlabel="\0";
	if(ylabel==NULL) ylabel="\0";
	if(plottitle==NULL) plottitle="\0";

	/* allocate memory for bigtic */
	bigtic= realloc(bigtic,MAXWORDLEN*sizeof(*bigtic));
	if(bigtic==NULL) {fprintf(stderr,"\n\a--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}

	/********************************************************************************
	STORE USER-LINES
	********************************************************************************/
	// build the list of horizontal lines
	if(sethlines!=NULL) {
		index1= xf_lineparse2(sethlines,",",&nhlines);
		if((hlines= malloc(nhlines*sizeof(hlines)))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		for(ii=0;ii<nhlines;ii++) hlines[ii]= atof(sethlines+index1[ii]);
		free(index1);
	}
	// build the list of vertical lines
	if(setvlines!=NULL) {
		index2= xf_lineparse2(setvlines,",",&nvlines);
		if((vlines= malloc(nvlines*sizeof(vlines)))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		for(ii=0;ii<nvlines;ii++) vlines[ii]= atof(setvlines+index2[ii]);
		free(index2);
	}

	/********************************************************************************
	NOW READ THE FILE AND STORE THE DATA
	********************************************************************************/
	nn= nrows= colmax= 0;
	sizeofx= sizeof(*xdata);
	sizeofy= sizeof(*ydata);
	sizeofz= sizeof(*zdata);
	sizeofncols= sizeof(*ncols);
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n\a--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
	while((line=xf_lineread1(line,&maxlinelen,fpin))!=NULL) {
		if(maxlinelen==-1)  {fprintf(stderr,"\n--- Error[%s]: readline function encountered insufficient memory\n\n",thisprog);exit(1);}
		if(line[0]=='#') continue;
		pline=line;

		for(col=0;(pcol=strtok(pline," ,\t\n"))!=NULL;col++)	{
		 	pline=NULL;
		 	xdata= realloc(xdata,(nn+1)*sizeofx);
		 	ydata= realloc(ydata,(nn+1)*sizeofy);
		 	zdata= realloc(zdata,(nn+1)*sizeofz);
		 	if(xdata==NULL||ydata==NULL||zdata==NULL) {fprintf(stderr,"\n\a--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
		 	xdata[nn]= (double)col;
		 	ydata[nn]= (double)nrows;
		 	if(sscanf(pcol,"%lf",&zdata[nn])!=1) zdata[nn]=NAN;
		 	nn++;
		}
		// get width to be applied to values in this row - later to be corrected by xrange
		// this is used to fill each row of the plot regardless of the number of items on each row
		ncols= realloc(ncols,(nrows+1)*sizeofncols);
		if(ncols==NULL) {fprintf(stderr,"\n\a--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
		ncols[nrows]=col;
		if(colmax<col) colmax=col; // store longest row of input in case xstep is used to determine xrange
		nrows++;
	}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);


	/* IF BLOCKWIDTH IS FIXED, CHECK THAT MATRIX IS SYMMETRICAL */
	if(setblockwidth==0) {
		for(ii=0;ii<nrows;ii++) {
			if(ncols[ii]!=ncols[0]) {fprintf(stderr,"\n\a--- Error[%s]: unequal columns in row %ld. Fix, or use dynamic block-widths (-bw 1)\n\n",thisprog,(ii+1));exit(1);}
		}
	}
	//TEST: for(ii=0;ii<nn;ii++) fprintf(stderr,"%g	%g	%g\n",xdata[ii],ydata[ii],zdata[ii]); exit(0);

	/* OVERRIDE ALL MIN/MAX WITH COMMAND-LINE SETTINGS, AND ULTIMATELY, TO ACCOMODATE USER-LINES, IF REQUIRED */
	/* initiall set range to 0-1 */
	xmin=ymin=0;
	xmax=ymax=1;

	/* FINAL CHECK FOR INVALID MIN-MAX VALUES - POSSIBLE IF USER DEFINES A MIN OR MAX VALUE OUTSIDE THE DATA RANGE? */
	/* next see if offset is applied using setxmin/setymin */
	if(isfinite(setxminval)) { xmin=setxminval; xmax+=xmin;}
	if(isfinite(setyminval)) { ymin=setyminval; ymax+=ymin;}
	/* now adjust xmax and ymax using setxstep/setystep stepxmax/setymax */
	if(isfinite(setxstep)) xmax= setxstep*colmax + xmin ;
	else if(isfinite(setxmaxval)) xmax= setxmaxval;
	if(isfinite(setystep)) ymax= setystep*nrows + ymin ;
	else if(isfinite(setymaxval)) ymax= setymaxval;

	if(xmin>xmax) {fprintf(stderr,"\n\a--- Error[%s]: xmin (%g) must be <= xmax (%g)\n\n",thisprog,xmin,xmax);exit(1);}
	if(ymin>ymax) {fprintf(stderr,"\n\a--- Error[%s]: ymin (%g) must be <= ymax (%g)\n\n",thisprog,ymin,ymax);exit(1);}
	if(xmax<xmin) {fprintf(stderr,"\n\a--- Error[%s]: -xmax (%g) must be >= -xmin (%g)\n\n",thisprog,xmax,xmin);exit(1);}
	if(ymax<ymin) {fprintf(stderr,"\n\a--- Error[%s]: -ymax (%g) must be >= -ymin (%g)\n\n",thisprog,ymax,ymin);exit(1);}


	/* DETERMINE THE RANGES */
	xrange=xmax-xmin;
	yrange=ymax-ymin;

	// DETERMINE BLOCK WIDTHS (XRANGE/NCOLS FOR EACH LINE)
	wdata= realloc(wdata,nn*sizeof(*wdata));
	if(wdata==NULL) {fprintf(stderr,"\n\a--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}

	kk=0;
	for(ii=0;ii<nrows;ii++) {
		for(jj=0;jj<ncols[ii];jj++) {
			wdata[kk++]= xrange/(double)ncols[ii];
			//TEST: printf("ii=%ld jj=%ld kk=%ld xrange=%g ncols[%ld]=%ld\n",ii,jj,kk,xrange,ii,ncols[ii]);
		}
	}


	// CORRECT X-VALUES : multiply by width for each datapoint and add xmin
	for(ii=0;ii<nn;ii++) xdata[ii]= xdata[ii]*wdata[ii] + xmin;



	/********************************************************************************
	SET Z-RANGE
	********************************************************************************/
	/* define default values based on actual numbers */
	zmin=zmax=NAN;
	for(ii=0;ii<nn;ii++) if(isfinite(zdata[ii])) {zmin=zmax=zdata[ii];break;}
	if(isfinite(zmin)) {
		for(ii=0;ii<nn;ii++) {
			if(zdata[ii]<zmin) zmin=zdata[ii];
			if(zdata[ii]>zmax) zmax=zdata[ii];
	}}

	/* set overrides for zmin & zmax - percentiles if required */
	if(setzmintemp!=NULL) {
		setzmin= atof(setzmintemp);
		if(strstr(setzmintemp,"%")!=NULL) {
			setzmin= xf_percentile2_d(zdata,nn,setzmin,message);
			if(!isfinite(setzmin)) { fprintf(stderr,"\b\n\t%s/%s\n\n",thisprog,message); exit(1); }
	}}
	if(setzmaxtemp!=NULL) {
		setzmax= atof(setzmaxtemp);
		if(strstr(setzmaxtemp,"%")!=NULL) {
			setzmax= xf_percentile2_d(zdata,nn,setzmax,message);
			if(!isfinite(setzmax)) { fprintf(stderr,"\b\n\t%s/%s\n\n",thisprog,message); exit(1); }
	}}


	/* if setzmin was set, override default zmin */
	if(isfinite(setzmin)) {
		if(setzclip==0) { for(ii=0;ii<nn;ii++) { if(zdata[ii]<setzmin) zdata[ii]=setzmin; }}
		if(setzclip==1) { for(ii=0;ii<nn;ii++) { if(zdata[ii]<setzmin) zdata[ii]=NAN; }}
		zmin=setzmin;
	}
	/* if setzmax was set, override default zmaz */
	if(isfinite(setzmax)) {
		if(setzclip==0) { for(ii=0;ii<nn;ii++) { if(zdata[ii]>setzmax) zdata[ii]=setzmax; }}
		if(setzclip==1) { for(ii=0;ii<nn;ii++) { if(zdata[ii]>setzmax) zdata[ii]=NAN; }}
		zmax=setzmax;
	}
	/* if setzmid was set, adjust zmin and zmax accordingly, regardless of how they were initially derived  */
	if(isfinite(setzmid)) {
		if(setzmid<=zmin) { aa=fabs(setzmid-zmax); zmin=setzmid-aa; }
		else if(setzmid>=zmax) { bb=fabs(setzmid-zmin); zmax=setzmid+bb; }
		else { aa=zmax-setzmid; bb=setzmid-zmin; if(aa>bb) zmin=setzmid-aa; else zmax=setzmid+bb; }
	}
	/* if after all that zrange is zero, create a fake range */
	if(zmin==zmax) {
		if(zmin<0) aa= zmin/-2.0 ;
		else if(zmin>0) aa= zmin/2.0 ;
		else aa= 0.5;
		zmin-=aa;
		zmax+=aa;
	}
	zrange=zmax-zmin;

	/* WARN IF PLOT IS TO BE EMPTY - MAKE SOME FAKE DATA BUT KEEP nn=0 */
	if(nn<1) {
		nn=0;
		fprintf(stderr,"\n\a--- Warning[%s]: no data! Check input columns and whether input is non-numeric\n\n",thisprog);
		wdata= realloc(wdata,1*sizeof(*wdata));
		xdata= realloc(xdata,1*sizeof(*xdata));
		ydata= realloc(ydata,1*sizeof(*ydata));
		zdata= realloc(zdata,1*sizeof(*zdata));
		wdata[0]=1.0; xdata[0]=0.0; ydata[0]=0.0;  zdata[0]=0.0;
	}

	/* AUTOMATICALLY CREATE A FAKE RANGE IF THERE IS NO VARIANCE IN THE X- or Y-DATA (FOR Y-DATA, ONLY IF NO ERROR-BARS EITHER)*/
	if(xmin==xmax) {
		xmin=xmin-1.0; xmax=xmax+1.0; xrange=xmax-xmin;
		if(!isfinite(setxminval) && !isfinite(setxmaxval) && setxint==0) {setxint=1; xint=1.0;}
	}
	if(ymin==ymax) {
		ymax=ymin+ymin; yrange=ymax-ymin;
		if(!isfinite(setyminval) && !isfinite(setymaxval) && setyint==0) {setyint=1; yint=1.0;}
	}
	/* AUTOMATICALLY SET XINT, YINT */
	if(setxint<=0) { if(xrange>0.0) xint=xf_trimdigits_d( (double)(xrange/5.0),(int)1); else xint=1.0; }
	else xint=setxint;
	if(setyint<=0) { if(yrange>0.0) yint=xf_trimdigits_d( (double)(yrange/5.0),(int)1); else yint=1.0; }
	else yint=setyint;
	/* set tic-precision based on tic-intervals */
	if(xint>10) xticprecision= 0;
	else xticprecision= xf_precision_d(xint,8);
	if(yint>10) yticprecision= 0;
	else yticprecision= xf_precision_d(yint,8);
	/* determine padding (or manual override) based on tic-intervals */
	if(setxpad<0) xpad= xint*0.5;
	if(setypad<0) ypad= yint*0.5;
	//TEST:	fprintf(stderr,"setxint:%g xint:%g xmin:%g xmax:%g\nsetyint:%g yint:%g ymin:%g ymax:%g\n",setxint,xint,xmin,xmax,setyint,yint,ymin,ymax); //exit(0);

	/* SET BLOCK WIDTH & HEIGHT : WIDTH ONLY APPLIES IF BLOCK-WIDTH IS FIXED (-bw 0) */
	blockwidth=  xrange/(double)ncols[0];
	blockheight= yrange/(double)nrows;


	/* ADJUST Y-VALUES AND FLIP MATRIX ON Y-AXIS IF NECESSARY */
	if(setyflip==1) { // if user says "flip", the matrix is actually kept "as-read"
		for(ii=0;ii<nn;ii++) ydata[ii]=ydata[ii]*blockheight + ymin;
	}
	if(setyflip==0) { // if user says "don't flip", the matrix is inverted so the bottom row corresponds with y=0
		aa=(double)(nrows-1);
		for(ii=0;ii<nn;ii++) ydata[ii]= (aa-ydata[ii])*blockheight + ymin;
	}

	/* ADJUST PLOT SIZE AND DETERMINE FACTOR BY WHICH TO ADJUST VALUES FOR PLOTTING */
	xlimit*=xscale;
	ylimit*=yscale;
	xfactor=xlimit/(xrange+2*xpad);
	yfactor=ylimit/(yrange+2*ypad);

	/* DETERMINE MAXIMUM CHARACTERS IN Y-AXIS TIC-LABELS, FORM MAP OFFSET */
	/* if setyint==-1, there will be no y-tics so the value stays at 0 */
	yticmaxchar=0.0;
	if(setyint!=-1) {
		if((ymin<=0. && ymax>0.)||(ymax>=0 && ymin<0.)) {
			for(aa=0;aa>=ymin;aa-=yint) {
				if(yticprecision==0) snprintf(templine,MAXWORDLEN,"%ld",(long)aa);
				else snprintf(templine,MAXWORDLEN,"%.*f",yticprecision,aa);
				z=strlen(templine); if(z>(int)yticmaxchar) {yticmaxchar=(float)z;sprintf(bigtic,"%s",templine); }
			}
			for(aa=(0+yint);aa<=ymax;aa+=yint) {
				if(yticprecision==0) snprintf(templine,MAXWORDLEN,"%ld",(long)aa);
				else snprintf(templine,MAXWORDLEN,"%.*f",yticprecision,aa);
				z=strlen(templine); if(z>(int)yticmaxchar) {yticmaxchar=(float)z;sprintf(bigtic,"%s",templine); }
			}
		}
		else {
			for(aa=ymin; aa<=ymax; aa+=yint) {
				if(yticprecision==0) snprintf(templine,MAXWORDLEN,"%ld",(long)aa);
				else snprintf(templine,MAXWORDLEN,"%.*f",yticprecision,aa);
				z=strlen(templine); if(z>(int)yticmaxchar) {yticmaxchar=(float)z;sprintf(bigtic,"%s",templine); }
			}
		}
	}
	if( yint!=(int)yint) yticmaxchar-=0.75 ; // reduced width for inclusion of a decimal
	if( ymin<0) yticmaxchar-=0.5 ; // reduced width for the negative symbol
	//TEST:fprintf(stderr,"yticmaxchar=%g\n",yticmaxchar); fprintf(stderr,"bigtic=%s\n",bigtic); exit(0);

	/* AUTOMATICALLY SET zx AND zy TO ACCOUNT FOR AXIS LABELS */
	if(zx<0) zx= setticsize+fontsize*yticmaxchar+(fontsize*4.0);
	if(zy<0) zy= 842-(ylimit + fontsize*4.4) ; // 842 = y-points in an A4 page


	/*************************************************************************/
	/* DEFINE THE RGB COLOUR ARRAYS */
	/*************************************************************************/
	red= realloc(red,setrgbn*sizeof(*red));
	green= realloc(green,setrgbn*sizeof(*green));
	blue= realloc(blue,setrgbn*sizeof(*blue));
	if(red==NULL||green==NULL||blue==NULL) {fprintf(stderr,"\n\a--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}

	/*************************************************************************/
	/*************************************************************************/
	/*************************************************************************/
	/*  NOW PRODUCE THE POSTSCRIPT OUTPUT **********************************/
	/*************************************************************************/
	/*************************************************************************/
	/*************************************************************************/

	/* OPEN POSTSCRIPT FILE */
	if((fpout=fopen(outfile,"w"))==0) {fprintf(stderr,"\n\a--- Error[%s]: file \"%s\" not found\n\n",thisprog,outfile);exit(1);}

	/* PRINT BASIC POSTSCRIPT FILE HEADER */
	fprintf(fpout,"%%!\n");

	/* DEFINE BOUNDING BOX FOR PRINTING, PDF CONVERSION ETC */
	fprintf(fpout,"%%%%BoundingBox: 0 0 595 842\n"); // this assumes an A4 page
	fprintf(fpout,"/Helvetica findfont 12.00 scalefont setfont\n");

	/* DEFINE THE COLOURS */
	fprintf(fpout,"\n%% DEFINE_COLOUR_SET  DESCRIPTION	GROUP-LABEL\n");
	fprintf(fpout,"/ct {.5 .5 .5} def   %% title colour\n");
	fprintf(fpout,"/cf {.0 .0 .0} def   %% frame colour\n");
	/* invalid (NAN) colour  */
	fprintf(fpout,"/cx {1. 1. 1.} def   %% invalid NAN colour\n");
	/* lowest-value colour */
	fprintf(fpout,"/c0 {.0 .0 .0} def   %% lowest-value colour\n");
	/* the data palette */
	for(ii=0;ii<setrgbn;ii++) red[ii]=green[ii]=blue[ii]=NAN;
	x= xf_palette7(red,green,blue,setrgbn,setrgbpal);
	for(ii=0;ii<setrgbn;ii++) fprintf(fpout,"/c%ld {%.14g %.14g %.14g} def\n",(ii+1),red[ii],green[ii],blue[ii]);
		// TEST RGB COLOUR MODEL: for(ii=0;ii<setrgbn;ii++) { printf("1	%d	%g\n",i,red[ii]);	printf("2	%d	%g\n",i,green[ii]); 	printf("3	%d	%g\n",i,blue[ii]); }


	/* DEFINE AXIS-LABELLING FUNCTIONS - SHOULD BE THE SAME FOR ALL PLOTS ON A PAGE */
	/* ...EXCEPT Y-AXIS LABEL OFFSET (YALOFF) - THIS IS RE-DEFINED AT PLOT-TIME */
	fprintf(fpout,"\n%% DEFINE_AXIS_LABEL_FUNCTIONS\n");
	fprintf(fpout,"/xaxislabel { moveto dup stringwidth pop 2 div neg xaloff rmoveto show } def\n");
	fprintf(fpout,"/yaxislabel { 90 rotate moveto dup stringwidth pop 2 div neg yaloff rmoveto show -90 rotate } def\n");
	fprintf(fpout,"/f_plottitle { ct setrgbcolor basefontsize .25 mul add moveto 0 basefontsize 2 div rmoveto show cf setrgbcolor } def\n");
	/* define tics-label functions: offsets determined by font and tic-size, and whether tics are positive or negative */
	fprintf(fpout,"\n%% DEFINE_TIC_LABEL_FUNCTIONS\n");
	fprintf(fpout,"/xtloff { basefontsize -1 mul ticout add } def\n");
	fprintf(fpout,"/ytloff { basefontsize -0.25 mul ticout add } def\n");
	fprintf(fpout,"/xticlabel { moveto dup stringwidth pop 2 div neg xtloff rmoveto show } def\n");
	fprintf(fpout,"/yticlabel { moveto dup stringwidth pop neg ytloff add -0.25 basefontsize mul rmoveto show } def\n");
	/* define tic-drawing function - negative tics are outside frame */
	fprintf(fpout,"\n%% DEFINE_TIC_FUNCTIONS\n");
	fprintf(fpout,"/xtic { newx dup 0 exch 0 moveto 0 ticsize rlineto stroke xticlabel} def\n");
	fprintf(fpout,"/ytic { newy 0 exch 1 index 1 index moveto ticsize 0 rlineto stroke yticlabel } def\n");

	/* DEFINE_DATA-PLOTTING_FUNCTIONS */
	fprintf(fpout,"\n%% DEFINE_DATA_PLOTTING_FUNCTIONS\n");
	fprintf(fpout,"/line {newpoint lineto} def \n");
	fprintf(fpout,"/pfill {fill} def \n");
	fprintf(fpout,"/popen {closepath} def \n");
	fprintf(fpout,"\n");

	fprintf(fpout,"/A {\n");
	fprintf(fpout,"\t%% block-draw (fixed-width), input: r-g-b\n");
	fprintf(fpout,"\tsetrgbcolor\n");
	fprintf(fpout,"\tpx1 py1 moveto\n"); // insert x/y coordinates
	fprintf(fpout,"\t0 blockheight rlineto\n");
	fprintf(fpout,"\tblockwidth 0 rlineto\n");
	fprintf(fpout,"\t0 blockheight neg rlineto\n");
	fprintf(fpout,"\tfill\n");
	fprintf(fpout,"\tpx1 blockwidth add /px1 exch def\n");
	fprintf(fpout,"} def\n");
	fprintf(fpout,"\n");

	fprintf(fpout,"/B {\n");
	fprintf(fpout,"\t%% block-draw (variable-width), input: width x y r-g-b\n");
	fprintf(fpout,"\tsetrgbcolor\n");
	fprintf(fpout,"\tnewpoint moveto\n"); // insert x/y coordinates
	fprintf(fpout,"\t0 blockheight rlineto\n");
	fprintf(fpout,"\tshiftx 0 rlineto\n");
	fprintf(fpout,"\t0 blockheight neg rlineto\n");
	fprintf(fpout,"\tfill\n");
	fprintf(fpout,"} def\n");
	fprintf(fpout,"\n");

	fprintf(fpout,"/C {\n");
	fprintf(fpout,"\t%% carriage-return for fixed-width block-drawing (A)\n");
	fprintf(fpout,"\tpx1 ncols blockwidth mul sub /px1 exch def\n");
	fprintf(fpout,"\tpy1 blockheight sub /py1 exch def\n");
	fprintf(fpout,"} def\n");
	fprintf(fpout,"\n");

	fprintf(fpout,"/key {\n");
	fprintf(fpout,"\t%% draw the colour-scale key, input: x y r-g-b\n");
	fprintf(fpout,"\t/xbsize { basefontsize .5 mul } def\n");
	fprintf(fpout,"\t/ybsize { ybsize_thisplot } def\n");
	fprintf(fpout,"\tsetrgbcolor\n");
	fprintf(fpout,"\tmoveto\n"); // insert x/y coordinates
	fprintf(fpout,"\t0 ybsize rlineto\n");
	fprintf(fpout,"\txbsize 0 rlineto\n");
	fprintf(fpout,"\t0 ybsize neg rlineto\n");
	fprintf(fpout,"\tfill\n");
	fprintf(fpout,"} def\n");
	fprintf(fpout,"\n");

	/* SHIFT COORDINATES FOR THIS PLOT */
	fprintf(fpout,"\n%% SHIFT_COORDINATES\n");
	fprintf(fpout,"%g %g translate\n",zx,zy);
	fprintf(fpout,"\n");

 	fprintf(fpout,"%% PLOT_CODE_START\n");
	fprintf(fpout,"\n");

	/* DEFINE EDITABLE PARAMETERS */
	fprintf(fpout,"%% DEFINE_EDITABLE_PLOT_PARAMETERS\n");
	fprintf(fpout,"/xlabel {(%s)} def\n",xlabel);
	fprintf(fpout,"/ylabel {(%s)} def\n",ylabel);
	fprintf(fpout,"/plottitle {(%s)} def\n",plottitle);
	fprintf(fpout,"/linewidth_data {%.14g} def\n",lwdata);
	fprintf(fpout,"/linewidth_axes {%.14g} def\n",lwaxes);
	fprintf(fpout,"/ticsize {%g} def\n",setticsize);
	fprintf(fpout,"/basefontsize {%g} def\n",fontsize);
	fprintf(fpout,"\n");

	/* DEFINE FIXED PLOT PARAMETERS */
	fprintf(fpout,"%% DEFINE_FIXED_PLOT_PARAMETERS_(DO_NOT_EDIT)\n");
	fprintf(fpout,"/yticmaxchar {%g} def\n",yticmaxchar);
	fprintf(fpout,"ticsize 0 lt {/ticout ticsize def} {/ticout 0 def} ifelse \n");
	fprintf(fpout,"/ncols {%ld} def\n",ncols[0]);
	fprintf(fpout,"/xoffset {%g} def\n",(xpad-xmin)*xfactor);
	fprintf(fpout,"/yoffset {%g} def\n",(ypad-ymin)*yfactor);
	fprintf(fpout,"/newx {%g mul xoffset add } def\n",xfactor);
	fprintf(fpout,"/newy {%g mul yoffset add } def\n",yfactor);
	fprintf(fpout,"/shiftx {%g mul} def\n",xfactor);
	fprintf(fpout,"/shifty {%g mul} def\n",yfactor);
	fprintf(fpout,"/newpoint {newy exch newx exch} def\n");
	fprintf(fpout,"/ybsize_thisplot { %g } def\n",((200.0*yscale)/setrgbn));
	fprintf(fpout,"/px1 %g newx def \n",xdata[0]);
	fprintf(fpout,"/py1 %g newy def \n",ydata[0]);
	fprintf(fpout,"/blockwidth { %g } def \n",blockwidth*xfactor);
	fprintf(fpout,"/blockheight { %g } def \n",blockheight*yfactor);

	/********************************************************************************/
	/* PLOT DATA */
	/********************************************************************************/
	fprintf(fpout,"\n%% PLOT_DATA\n");
	fprintf(fpout,"\n%% PLOT_POINTS\n");
	fprintf(fpout,"linewidth_data setlinewidth\n");
	fprintf(fpout,"newpath\n");
	/* find the smallest possible float just less than 1 : this adjuster ensures "z" (below) does not exceed setrgbn, and only the exact minimum is assigned to c0 */
	a= nextafterf((float)1,-FLT_MAX);
	/* option: fixed block-width - postscript function "A" */
	if(setblockwidth==0) {
		kk=0;
		for(ii=0;ii<nrows;ii++) {
			for(jj=0;jj<ncols[ii];jj++) {
				if(isfinite(zdata[kk])) {
					z=0+(int)(a+(setrgbn*(zdata[kk]-zmin)/zrange));
					fprintf(fpout,"c%d\tA\n",z);
				}
				else fprintf(fpout,"cx\tA\n");
				kk++;
			}
			fprintf(fpout,"C\n");
		}
	}
	/* option: adaptive block-width - postscript function "B" */
	if(setblockwidth==1) {
		// first go to first in-range data point
		fprintf(fpout,"px1 py1 moveto\n");
		for(ii=0;ii<nn;ii++) {
			if(isfinite(zdata[ii])) {
				z=0+(int)(a+(setrgbn*(zdata[ii]-zmin)/zrange));
				fprintf(fpout,"%g\t%g\t%g\tc%d\tB\n",wdata[ii],xdata[ii],ydata[ii],z);
			}
			else {
				fprintf(fpout,"%g\t%g\t%g\tcx\tB\n",wdata[ii],xdata[ii],ydata[ii]);
	}}}
	fprintf(fpout,"stroke\n");
	fprintf(fpout,"closepath\n");


	/********************************************************************************/
	/* DRAW FRAME */
	/********************************************************************************/
	fprintf(fpout,"\n%% DRAW_FRAME\n");
	fprintf(fpout,"cf setrgbcolor\n");
	fprintf(fpout,"linewidth_axes setlinewidth\n");
	if(framestyle>0) {
		if ((int)(framestyle&1)>0) f1=1;
		if ((int)(framestyle&2)>0) f2=1;
		if ((int)(framestyle&4)>0) f3=1;
		if ((int)(framestyle&8)>0) f4=1;
		fprintf(fpout,"newpath\n");
		if (f1==1) {
			fprintf(fpout,"%g 0 moveto 0 0 lineto\n",xlimit);
			if (f2==0) fprintf(fpout,"stroke\n");
		}
		if (f2==1) {
			if(f1==0) fprintf(fpout,"newpath 0 0 moveto\n");
			fprintf(fpout,"0 %g lineto\n",ylimit);
			if(f3==0) fprintf(fpout,"stroke\n");
		}
		if (f3==1) {
			if(f2==0) fprintf(fpout,"newpath 0 %g moveto\n",xlimit);
			fprintf(fpout,"%g %g lineto\n",xlimit,ylimit);
			if(f4==0) fprintf(fpout,"stroke\n");
		}
		if (f4==1){
			if(f3==0) fprintf(fpout,"newpath %g %g moveto\n",xlimit,ylimit);
			fprintf(fpout,"%g 0 lineto \n",xlimit);
			if(f1==0) fprintf(fpout,"stroke\n");
			else fprintf(fpout,"0 0 lineto stroke\n");
		}
		fprintf(fpout,"stroke\n");
	}
	fprintf(fpout,"closepath\n");


	/* DRAW YZERO-LINE IF REQUIRED */
	if(ymin<0&&ymax>0) {
		fprintf(fpout,"\n%% DRAW_YZERO_LINE\n");
		fprintf(fpout,"cf setrgbcolor\n");
		fprintf(fpout,"	linewidth_axes setlinewidth\n");
		fprintf(fpout,"newpath 0 0 newy moveto %g 0 rlineto\n",xlimit);
		fprintf(fpout,"stroke\n");
		fprintf(fpout,"\n");
	}


	/********************************************************************************/
	/* DRAW TICS AND TIC-LABELS */
	/********************************************************************************/
	fprintf(fpout,"\n%% DRAW_TICS_AND_LABELS\n");
	fprintf(fpout,"/Helvetica findfont basefontsize scalefont setfont\n");
	fprintf(fpout,"cf setrgbcolor\n");
	fprintf(fpout,"	linewidth_axes setlinewidth\n");
	/* XTICS: if zero is in the range, make sure zero is included as a tic mark */
	if(setxint>=0) {
		/* set max decimal precision for tic-labels */
		xticprecision= xf_precision_d(xint,8);
		fprintf(fpout,"newpath\n");
		if((xmin<=0 && xmax>0)||(xmax>=0 && xmin<0)) {
			for(aa=0;aa>=(xmin-xpad);aa-=xint) {
				if(xticprecision==0) fprintf(fpout,"(%ld) %ld xtic\n",(long)aa,(long)aa);
				else fprintf(fpout,"(%.*f) %.*f xtic\n",xticprecision,aa,xticprecision,aa);
			}
			for(aa=(0+xint);aa<=(xmax+xpad);aa+=xint) {
				if(xticprecision==0) fprintf(fpout,"(%ld) %ld xtic\n",(long)aa,(long)aa);
				else fprintf(fpout,"(%.*f) %.*f xtic\n",xticprecision,aa,xticprecision,aa);
			}
		}
		else {
			for(aa=xmin; aa<=(xmax+xpad); aa+=xint) {
				if(xticprecision==0) fprintf(fpout,"(%ld) %ld xtic\n",(long)aa,(long)aa);
				else fprintf(fpout,"(%.*f) %.*f xtic\n",xticprecision,aa,xticprecision,aa);
			}
		}
		fprintf(fpout,"stroke\n");
	}
	/* YTICS: if zero is in the range, make sure zero is included as a tic mark */
	/* - note that we also print to a templine variable here to estimate the print-size of the largest tic */
	if(setyint>=0) {
		/* set max decimal precision for tic-labels */
		yticprecision= xf_precision_d(yint,8);
		fprintf(fpout,"newpath\n");
		yticmaxchar=0.0;
		if((ymin<=0 && ymax>0)||(ymax>=0 && ymin<0)) {
			/* do numbers <0 */
			for(aa=0;aa>=(ymin-ypad);aa-=yint) {
				if(yticprecision==0) fprintf(fpout,"(%ld) %ld ytic\n",(long)aa,(long)aa);
				else fprintf(fpout,"(%.*f) %.*f ytic\n",yticprecision,aa,yticprecision,aa);
				snprintf(templine,MAXWORDLEN,"%.*f",yticprecision,aa);
				z=strlen(templine);
				if(z>(int)yticmaxchar) {yticmaxchar=(float)z; snprintf(bigtic,MAXWORDLEN,"%s",templine);}

			}
			/* do numbers >0 */
			for(aa=(0+yint);aa<=(ymax+ypad);aa+=yint) {
				if(yticprecision==0) fprintf(fpout,"(%ld) %ld ytic\n",(long)aa,(long)aa);
				else fprintf(fpout,"(%.*f) %.*f ytic\n",yticprecision,aa,yticprecision,aa);
				snprintf(templine,MAXWORDLEN,"%.*f",yticprecision,aa);
				z=strlen(templine);
				if(z>(int)yticmaxchar) {yticmaxchar=(float)z; snprintf(bigtic,MAXWORDLEN,"%s",templine);}
			}
		}
		else {
			for(aa=ymin; aa<=(ymax+ypad); aa+=yint) {
				if(yticprecision==0) fprintf(fpout,"(%ld) %ld ytic\n",(long)aa,(long)aa);
				else fprintf(fpout,"(%.*f) %.*f ytic\n",yticprecision,aa,yticprecision,aa);
				snprintf(templine,MAXWORDLEN,"%.*f",yticprecision,aa);
				z=strlen(templine);
				if(z>(int)yticmaxchar) {yticmaxchar=(float)z; snprintf(bigtic,MAXWORDLEN,"%s",templine);}
			}
		}
		fprintf(fpout,"stroke\n");
	}
	fprintf(fpout,"\n");
	/* correct bigtic so postscript stringwidth function doesn't depreciate the width for non-proportional fonts */
	bigtic= xf_strsub1(bigtic,".","P");

	/********************************************************************************/
	/* DRAW AXIS LABELS - 2 font sizes larger than ticlabels, centred on each axis*/
	/********************************************************************************/
	fprintf(fpout,"\n%% DRAW_AXIS_LABELS\n");
	fprintf(fpout,"/Helvetica-Bold findfont basefontsize 2 add scalefont setfont\n");
	// define label offset
	if(setxint!=-1) fprintf(fpout,"/xaloff { basefontsize 2 add 1 mul basefontsize add -1 mul ticout add } def\n");
	else 		fprintf(fpout,"/xaloff { basefontsize 2 add 0 mul basefontsize add -1 mul ticout add } def\n");
	fprintf(fpout,"/yaloff { (%s) stringwidth pop ticout 2 mul sub } def\n",bigtic);
	fprintf(fpout,"xlabel %.14g 0 xaxislabel\n",xlimit/2.0);
	fprintf(fpout,"ylabel %.14g 0 yaxislabel\n",ylimit/2.0);


	/********************************************************************************/
	/* DRAW USER-DEFINED HORIZONTAL LINES, IF REQUIRED */
	/* Note: this uses the "line" function to position at the correct coordinates*/
	/********************************************************************************/
	if(sethlines!=NULL) {
		fprintf(fpout,"\n%% PLOT_USERLINE_HORIZONTAL\n");
		fprintf(fpout,"linewidth_axes setlinewidth\n");
		if(setulinecolour>=0) fprintf(fpout,"c%d setrgbcolor\n",setulinecolour);
		else fprintf(fpout,"cx setrgbcolor\n");
		for(ii=0;ii<nhlines;ii++) {
			if(isfinite(hlines[ii]) && hlines[ii]>=ymin && hlines[ii]<=ymax) {
				fprintf(fpout,"newpath\n");
				fprintf(fpout,"%g %g newpoint moveto\n",(xmin-xpad),hlines[ii]);
				fprintf(fpout,"%g %g line\n",(xmax+xpad),hlines[ii]);
				if(setulinestyle==1) fprintf(fpout,"stroke\n");
				if(setulinestyle==0) fprintf(fpout,"[3] 0 setdash stroke\n");
				fprintf(fpout,"closepath\n\n");
		}}
		fprintf(fpout,"cf setrgbcolor\n");
	}
	/********************************************************************************/
	/* DRAW USER-DEFINED VERTICAL LINES, IF REQUIRED */
	/* Note: this uses the "line" function to position at the correct coordinates*/
	/********************************************************************************/
	if(setvlines!=NULL) {
		fprintf(fpout,"\n%% PLOT_USERLINES_VERTICAL\n");
		fprintf(fpout,"linewidth_axes setlinewidth\n");
		if(setulinecolour>=0) fprintf(fpout,"c%d setrgbcolor\n",setulinecolour);
		else fprintf(fpout,"cx setrgbcolor\n");
		for(ii=0;ii<nvlines;ii++) {
			if(isfinite(vlines[ii]) && vlines[ii]>=xmin && vlines[ii]<=xmax) {
				fprintf(fpout,"newpath\n");
				fprintf(fpout,"%g %g newpoint moveto\n",vlines[ii],(ymin-ypad));
				fprintf(fpout,"%g %g line\n",vlines[ii],(ymax+ypad));
				if(setulinestyle==1) fprintf(fpout,"stroke\n");
				if(setulinestyle==0) fprintf(fpout,"[3] 0 setdash stroke\n");
				fprintf(fpout,"closepath\n\n");
		}}
		fprintf(fpout,"cf setrgbcolor\n");
	}

	/********************************************************************************/
	/* DRAW PLOT TITLE - 4 font sizes larger than basefont */
	/********************************************************************************/
	fprintf(fpout,"\n%% DRAW_PLOT_TITLE\n");
	fprintf(fpout,"/Helvetica-Bold findfont basefontsize 4 add scalefont setfont\n");
	fprintf(fpout,"plottitle 0 %.14g f_plottitle\n",ylimit);

	/********************************************************************************/
	/* DRAW KEY */
	/********************************************************************************/
	fprintf(fpout,"\n%% DRAW_KEY\n");
	fprintf(fpout,"newpath\n");
	dd=((200.0*yscale)/setrgbn); // height of each block in the key
	aa=xlimit+fontsize; // aa marks where (x) the key will begin (to the right of the plot)
	bb= ylimit-(ypad*yfactor)-fontsize; // bb marks where (y) the top label will go
	cc= bb-dd; // cc marks where (y) the top colour block will go
	for(ii=setrgbn;ii>0;ii--) {
		fprintf(fpout,"%g %g c%ld key\n",aa,cc,ii);
		cc-=dd;
	}
	/* print labels for top and bottom rate */
	fprintf(fpout,"stroke\n");
	fprintf(fpout,"closepath\n");
	fprintf(fpout,"/Helvetica findfont basefontsize .75 mul scalefont setfont\n");
	fprintf(fpout,"0 0 0 setrgbcolor\n");
	fprintf(fpout,"%g %g moveto\n",(xlimit+fontsize),(bb+0.5*fontsize));
	fprintf(fpout,"(%.3g) show\n",zmax);
	fprintf(fpout,"%g %g moveto\n",(xlimit+fontsize),(cc+dd-fontsize));
	fprintf(fpout,"(%.3g) show\n",zmin);

	/********************************************************************************/
	/* FINISH POSTSCRIPT */
	/********************************************************************************/
	fprintf(fpout,"\n%% PLOT_CODE_END\n");
 	fprintf(fpout,"\n%% SHOW_PAGE\n");
	fprintf(fpout,"showpage\n");
	fclose(fpout);


	/******************************************************************************/
	/* FREE MEMORY AND EXIT */
	/******************************************************************************/
	if(line!=NULL) free(line);
	if(templine!=NULL) free(templine);
	if(xdata!=NULL) free(xdata);
	if(ydata!=NULL) free(ydata);
	if(zdata!=NULL) free(zdata);
	if(hlines!=NULL) free(hlines);
	if(vlines!=NULL) free(vlines);
	if(wdata!=NULL) free(wdata);
	if(ncols!=NULL) free(ncols);
	if(red!=NULL)free(red);
	if(green!=NULL) free(green);
	if(blue!=NULL) free(blue);
	if(bigtic!=NULL) free(bigtic);
	exit(0);
}
