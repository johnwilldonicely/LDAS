/********************************************************************************
- draw postscipt firing rate map
- maptype argument determines if map is...

	(0) O'Keefe style blue-red map
	- 10% colour steps to peak
	- looks best with smoothing

	(1) Muller style map
	- fixed proportion of pixels dedicated to each colour
	- ensures that "abberant" very high rate pixels don't steal the scale
	- but can obscure problems with high-rate pixels
		1.0 yellow-purple scale (yellow= no spikes)
		1.1 black-red scale (black= no spikes)
	

	(2) Line map - for data collapsed in one dimension 
		2.0 collapsed in the x-dimension (vertical 1-d environment)
		2.1 collapsed in the y-dimension (horizontal 1-d environment)

	
- draw bounding box which accurately represents potential "field of view"
- firing rates converted to a percentage of peak
- peak firing rate value should be determined by calling function
- bins are scaled to make data fit within 255x255 pixel grid
- scaling is based on camera pixel resolution, NOT visited camera pixels
*********************************************************************************/
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define FUNC_NAME "xf_psratemap"

void xf_mullerize_f(float *rate,long bintot, float *result);

void xf_psratemap(
	float maptype,	/* type of map (see above) */
	float *maprate,	/* array of binned firing rate data */
	int mapbins,	/* width of map (assumed square - ie. if mapbins=4, total pixels= 16) */
	float mapbinsize,	/* size of bins, in cm */
	float peakrate,	/* peak firing rate - can be set to fixed value for absolute rate coding */
	int	peakxbin,	/* calling-function defines where the map peak is */
	int	peakybin,	/* calling-function defines where the map peak is */
	float mapbinscale, /* scaling factor for mapbinsize - calculated from camera resolution (256/max_cm) */
	float vidratio,	/* aspect ration of video signal: xpixels/ypixels */
	char *title, 
	float fontsize,
	char *outfile
	)
{
	char command[256];
	int i,j,k,w,x,y,z,n,skip,xbin,ybin,xlimit,ylimit,index,colour,coloursteps,border=10;
	float a,colour_coeff,result[65],peakrate2;
	double aa,bb,cc;
	FILE *fpout;

	coloursteps=10;
	colour_coeff = (1.0/peakrate)*coloursteps;
	/* regardless of number of bins, map is always 256 pixels square */
	/* this maintains relationship between postscript pixels and original binned data (cm)*/
	mapbinsize = mapbinscale*mapbinsize; /* mapbinscale is determined by pixel range of camera */

	/* define dimensions bounding plot based on video aspect ratio */
	xlimit = ylimit = 256;
	if(vidratio>1) ylimit = (int) (ylimit/vidratio); /* if x-range > y-range, scale down y-dimentions of box */
	if(vidratio<1) xlimit = (int) (xlimit/vidratio); /* if y-range > x-range, scale down x-dimentions of box */

	if((fpout=fopen(outfile,"w"))==NULL) {fprintf(stderr,"\n--- Error[%s]: can't write to file %s\n\n",FUNC_NAME,outfile);exit(1);}
	
	/* write postscript header info */
	fprintf(fpout,"%c!\n",37);
	fprintf(fpout,"/Helvetica findfont %.2f scalefont setfont\n",fontsize);

	/* define colours for maps */
	if(maptype==(float)0) {
		fprintf(fpout,"/c0 {0 0 1} def\n");
		fprintf(fpout,"/c1 {.25 .5 1} def\n");
		fprintf(fpout,"/c2 {.5 .75 1} def\n");
		fprintf(fpout,"/c3 {0 1 .75} def\n");
		fprintf(fpout,"/c4 {0  1 0} def\n");
		fprintf(fpout,"/c5 {.75  1 0} def\n");
		fprintf(fpout,"/c6 {1 1 0} def\n");
		fprintf(fpout,"/c7 {1 .75 0} def\n");
		fprintf(fpout,"/c8 {1 .5 0} def\n");
		fprintf(fpout,"/c9 {1 0 0} def\n");
		fprintf(fpout,"/c10 {1 0 0} def\n"); /* this will be the colour of peak rate bin(s) */
	}
	/* ...or, for Bob Muller style maps... */
	else if (maptype==(float)1.0) {
		fprintf(fpout,"/c0 {1 1 .5} def\n");
		fprintf(fpout,"/c1 {1 .75 .25} def\n");
		fprintf(fpout,"/c2 {1 0 0} def\n");
		fprintf(fpout,"/c3 {0 .75 0} def\n");
		fprintf(fpout,"/c4 {0  0 1} def\n");
		fprintf(fpout,"/c5 {.5 0 .5} def\n");
		fprintf(fpout,"/c6 {.75 .75 .75} def\n");  /* this will be the colour of peak rate bin(s) */
	}    
	else if (maptype==(float)1.1) {
		fprintf(fpout,"/c0 {0 0 0} def\n");
		fprintf(fpout,"/c1 {0 .5 1} def\n");
		fprintf(fpout,"/c2 {0 1 0} def\n");
		fprintf(fpout,"/c3 {1 1 0} def\n");
		fprintf(fpout,"/c4 {1 .75 0} def\n");
		fprintf(fpout,"/c5 {1 0 0} def\n");
		fprintf(fpout,"/c6 {1 0 0} def\n");  /* this will be the colour of peak rate bin(s) */
	}    
	else if(maptype!=(float)2.0 && maptype!=(float)2.1) {fprintf(stderr,"\n--- Error[%s]: illegal map-type specified: %g\n\n",FUNC_NAME,maptype);exit(1);}
	
	fprintf(fpout,"\n");

	/* define size of bins for ratemap */
	if((int)maptype==0) fprintf(fpout,"/size {%.2f mul} def\n",mapbinsize*1.1);
	else if((int)maptype==1) fprintf(fpout,"/size {%.2f mul} def\n",mapbinsize*0.8);
	else if((int)maptype==2) fprintf(fpout,"/size {%.2f mul} def\n",mapbinsize*1.1);

	fprintf(fpout,"\n");

	/* define routine for drawing bins */
	for(i=0;i<=coloursteps;i++) {
	fprintf(fpout,"/box%d	{\n",i);
	fprintf(fpout,"	c%d setrgbcolor\n",i);
	fprintf(fpout,"	newpath\n");
	fprintf(fpout,"	moveto\n");
	fprintf(fpout,"	0 1 size rlineto\n");
	fprintf(fpout,"	1 size 0 rlineto\n");
	fprintf(fpout,"	0 -1 size rlineto\n");
	fprintf(fpout,"	closepath\n");
	fprintf(fpout,"	fill\n");
	fprintf(fpout,"	} def\n");
	}
	fprintf(fpout,"\n");
	fprintf(fpout,"/outline	{\n");
	fprintf(fpout,"	newpath\n");
	fprintf(fpout,"	moveto\n");
	fprintf(fpout,"	0 1 size rlineto\n");
	fprintf(fpout,"	1 size 0 rlineto\n");
	fprintf(fpout,"	0 -1 size rlineto\n");
	fprintf(fpout,"	closepath\n");
	fprintf(fpout,"	stroke\n");
	fprintf(fpout,"	} def\n");
	fprintf(fpout,"\n");

	/* save state and original coordinate system */
	fprintf(fpout,"gsave\n"); 
	fprintf(fpout,"\n");

	/* Draw bounding box ********************************************/
	fprintf(fpout,"newpath\n");
	fprintf(fpout,"0 0 moveto\n");
	fprintf(fpout,"0 %d rlineto\n",ylimit);
	fprintf(fpout,"%d 0 rlineto\n",xlimit);
	fprintf(fpout,"0 -%d rlineto\n",ylimit);
	fprintf(fpout,"closepath\n");
	fprintf(fpout,"stroke\n");
	fprintf(fpout,"\n");

	/* Draw firing rate map *****************************************/

	fprintf(fpout,"\n%% RATEMAP_PLOT_CODE_START %s\n",title);

	if((int)maptype==0) {
		for(ybin=0;ybin<mapbins;ybin++) for(xbin=0;xbin<mapbins;xbin++)	{
			index=(ybin*mapbins)+xbin;
			if(maprate[index]<0) continue;
			if(maprate[index]>=peakrate) {
				colour=coloursteps;
				fprintf(fpout,"%.2f %.2f box%d\n",xbin*mapbinsize,ybin*mapbinsize,colour);
				continue;
			}
			else colour = (int) (maprate[index]*colour_coeff);
			fprintf(fpout,"%.2f %.2f box%d\n",xbin*mapbinsize,ybin*mapbinsize,colour);
	}}

	/* ...or, if it's a Bob Muller style map... */
	else if ((int)maptype==1) {
		xf_mullerize_f(maprate,(long)(mapbins*mapbins),result);
		for(ybin=0;ybin<mapbins;ybin++) for(xbin=0;xbin<mapbins;xbin++)	{
			index=(ybin*mapbins)+xbin;
			if(maprate[index]<0) continue;
	
			if(maprate[index]>result[4]) fprintf(fpout,"%.2f %.2f box5\n",xbin*mapbinsize,ybin*mapbinsize);
			else if(maprate[index]>result[3]) fprintf(fpout,"%.2f %.2f box4\n",xbin*mapbinsize,ybin*mapbinsize);
			else if(maprate[index]>result[2]) fprintf(fpout,"%.2f %.2f box3\n",xbin*mapbinsize,ybin*mapbinsize);
			else if(maprate[index]>result[1]) fprintf(fpout,"%.2f %.2f box2\n",xbin*mapbinsize,ybin*mapbinsize);
			else if(maprate[index]>result[0]) fprintf(fpout,"%.2f %.2f box1\n",xbin*mapbinsize,ybin*mapbinsize);
			else fprintf(fpout,"%.2f %.2f box0\n",xbin*mapbinsize,ybin*mapbinsize);
			if(maprate[index]>=peakrate) {
				fprintf(fpout,"%.2f %.2f box6\n",xbin*mapbinsize,ybin*mapbinsize);			
	}}}

	/* ...or, for a line-graph of ybin rates... */
	else if (maptype==(float)2.0) { 
		n=xbin=0;	for(ybin=0;ybin<mapbins;ybin++)	{
			index=(ybin*mapbins)+xbin;
			if(maprate[index]<0) continue;
			n++; /* n incriments only when a valid position is found */
			a= xlimit*maprate[index]/peakrate;
			if(a>xlimit) a=xlimit; /* to prevent plot from running outside box, if peakrate is manually set too low */
			if(n==1) {fprintf(fpout,"\nnewpath\n");	fprintf(fpout,"\t%.2f %.2f moveto\n",a,(ybin+0.5)*mapbinsize);}
			else fprintf(fpout,"\t%.2f %.2f lineto\n",a,(ybin+0.5)*mapbinsize);
		}
		fprintf(fpout,"stroke\n\n");
	}

	/* ...or, for a line-graph of xbin rates... */
	else if (maptype==(float)2.1) { 
		n=ybin=0; for(xbin=0;xbin<mapbins;xbin++)	{
			index=(ybin*mapbins)+xbin;
			if(maprate[index]<0) continue;
			n++; /* n incriments only when a valid position is found */
			a = ylimit*maprate[index]/peakrate;
			if(a>ylimit) a=ylimit; /* to prevent plot from running outside box, if peakrate is manually set too low */
			if(n==1) {fprintf(fpout,"\nnewpath\n");	fprintf(fpout,"\t%.2f %.2f moveto\n",(xbin+0.5)*mapbinsize,a);}
			else fprintf(fpout,"\t%.2f %.2f lineto\n",(xbin+0.5)*mapbinsize,a);
		}
		fprintf(fpout,"stroke\n\n");
	}

	/* Print title for plot (includes firing rate etc) */
	fprintf(fpout,"0 0 0 setrgbcolor\n"); 
	fprintf(fpout,"0 %f moveto\n",(ylimit+(0.3*fontsize)));
	fprintf(fpout,"(%s) show\n",title);

	fprintf(fpout,"\n%% RATEMAP_PLOT_CODE_END\n");

	/* now outline peak bin to make it stand out - has to be done here to avoid overlapping bins obscuring box borders */
	fprintf(fpout,"0 0 0 setrgbcolor %.2f %.2f outline\n",peakxbin*mapbinsize,peakybin*mapbinsize);


	fprintf(fpout,"\n");
	fprintf(fpout,"\nshowpage\n");
	fclose(fpout);
}
