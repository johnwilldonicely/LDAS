/********************************************************************************
- create isi and autocorrelation histograms
*********************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void hux_error(char message[]);
int hux_autocorr_array(double *time, int n, int *count, int bintot, int winsize,float scale, char *mode);

void hux_pshistograms(
	double *time,	/* pointer to a floating point array of time data (seconds) */
	int n,		/* number of elements in the time array */
	int probe,
	int cluster,
	char *output_path /* path for postscript output */
	)
{
	char outfile[256];
	int *count,fontsize=12,yoffbase=10,yoffset,bintot=100,winsize;
	int i,bin, boxwidth=256, halfboxwidth=(int)(boxwidth/2.0), boxheight=150;
	float ratio, insetscale=0.475, refract=2.0;
	FILE *fpout;

	/* write postscript file */
	sprintf(outfile,"%scrunch_isi_P%02d_C%02d.ps",output_path,probe,cluster);
	if((fpout=fopen(outfile,"w"))==NULL) {free(count);hux_error("hux_pshistograms: can't open postscript output file");}
	fprintf(fpout,"%c!\n",37);
	fprintf(fpout,"%cstart_histoplot\n",37);
	fprintf(fpout,"gsave\n");
	fprintf(fpout,"\n");
	fprintf(fpout,"1 setlinewidth\n");
	fprintf(fpout,"/bar	{\n");
	fprintf(fpout,"	moveto\n");
	fprintf(fpout,"	rlineto\n");
	fprintf(fpout,"	} def\n");
	fprintf(fpout,"\n");

	/* Produce 1/2 theta autocorrelogram */
	/* set variables: 2x500 ms window, 2.5ms bins */
	yoffset=yoffbase; bintot=200; winsize=500; ratio=128.0/(float)bintot;
	count=(int *)calloc(bintot+1,sizeof(int)); if(count==NULL) hux_error("hux_pshistogram: not enough memory for histogram");
	/* add label */
//	fprintf(fpout,"/Helvetica findfont %d scalefont setfont\n",fontsize);
//	fprintf(fpout,"0 %d moveto\n",(int)(yoffset-0.8*fontsize));
//	fprintf(fpout,"(2x%d ms AutoCor., bars= %.1f ms) show\n",winsize,((float)winsize/(float)bintot));
//	fprintf(fpout,"\n");
	/* fill histogram count array */
	for(i=0;i<bintot;i++) count[i]=0;

	if(n>1) i=hux_autocorr_array(time,n,count,bintot,winsize,boxheight,"acor");
	/* draw bounding box */
	fprintf(fpout,"newpath\n");
	fprintf(fpout,"	0 %d moveto\n",yoffset);
	fprintf(fpout,"	0 %d rlineto\n", boxheight);
	fprintf(fpout,"	%d 0 rlineto\n",boxwidth);
	fprintf(fpout,"	0 -%d rlineto\n",boxheight);
	fprintf(fpout,"	closepath\n");
	fprintf(fpout,"	stroke\n");
	fprintf(fpout,"\n");
	/* draw histogram */
	fprintf(fpout,".5 .5 .5 setrgbcolor\n");
	fprintf(fpout,"%.2f setlinewidth\n",ratio);
	fprintf(fpout,"newpath\n");
	for(bin=0;bin<bintot;bin++) fprintf(fpout,"0 %d %.2f %d  bar\n",count[bin],((bin*ratio)+halfboxwidth),yoffset); // right half
	fprintf(fpout,"stroke\n");
	fprintf(fpout,"1 setlinewidth\n");
	fprintf(fpout,"\n");
	/* draw midline */
	fprintf(fpout,"newpath\n");
	fprintf(fpout,"	1 0 0 setrgbcolor\n");
	fprintf(fpout,"	%d %d moveto\n",halfboxwidth,yoffset);
	fprintf(fpout,"	0 %d rlineto\n",boxheight);
	fprintf(fpout,"	stroke\n");
	fprintf(fpout,"	0 0 0 setrgbcolor\n");
	fprintf(fpout,"\n");

	fprintf(fpout,"gsave\n");
	fprintf(fpout,"%.3f %.3f scale\n",0.475,insetscale);

	/* Produce refractory interval inset isi histogram (+/-8ms window) */
	yoffset=(int)((float)(yoffbase+0.5*boxheight)/insetscale)+(int)(1.0/insetscale); 
	bintot=50; winsize=8; ratio=halfboxwidth/(float)bintot;
	free(count);
	count=(int *)calloc(bintot+1,sizeof(int)); if(count==NULL) hux_error("hux_pshistogram: not enough memory for histogram");
	fprintf(fpout,"1 setlinewidth\n");
	/* fill histogram count array */
	if(n>1) i=hux_autocorr_array(time,n,count,bintot,winsize,boxheight,"isi");
	/* draw dupicate histogram on either side of box */
	fprintf(fpout,"\n");
	fprintf(fpout,"%.2f setlinewidth\n",ratio);
	fprintf(fpout,"newpath\n");
	fprintf(fpout,".5 .5 .5 setrgbcolor\n");
	for(bin=0;bin<bintot;bin++) fprintf(fpout,"0 %d %.2f %d  bar\n",count[bin],(halfboxwidth-(bin*ratio)),yoffset);
	for(bin=0;bin<bintot;bin++) fprintf(fpout,"0 %d %.2f %d  bar\n",count[bin],((bin*ratio)+halfboxwidth),yoffset);
	fprintf(fpout,"	stroke\n");
	fprintf(fpout,"1 setlinewidth\n");
	fprintf(fpout,"\n");
	/* draw refractory lines */
	fprintf(fpout,"newpath\n");
	fprintf(fpout,"	1 0 0 setrgbcolor\n");
	fprintf(fpout,"	%d %d moveto\n",(int)(halfboxwidth*(1.0-refract/winsize)),yoffset);
	fprintf(fpout,"	0 %d rlineto\n",boxheight);
	fprintf(fpout,"	%d %d moveto\n",(int)(halfboxwidth*(1.0+refract/winsize)),yoffset);
	fprintf(fpout,"	0 %d rlineto\n",boxheight);
	fprintf(fpout,"	stroke\n");
	fprintf(fpout,"	0 0 0 setrgbcolor\n");

	/* Produce pyramidal/interneuron inset autocorrelogram (+/-50ms window) */
	yoffset=(int)(yoffbase/insetscale)+(int)(1.0/insetscale); 
	bintot=50; winsize=50; ratio=halfboxwidth/(float)bintot;
	free(count);
	count=(int *)calloc(bintot+1,sizeof(int)); if(count==NULL) hux_error("hux_pshistogram: not enough memory for histogram");
	fprintf(fpout,"1 setlinewidth\n");
	/* fill histogram count array */
	if(n>1) i=hux_autocorr_array(time,n,count,bintot,winsize,boxheight,"acor");
	/* draw dupicate histogram on either side of box */
	fprintf(fpout,"\n");
	fprintf(fpout,"%.2f setlinewidth\n",ratio);
	fprintf(fpout,"newpath\n");
	fprintf(fpout,".5 .5 .5 setrgbcolor\n");
	for(bin=0;bin<bintot;bin++) fprintf(fpout,"0 %d %.2f %d  bar\n",count[bin],(halfboxwidth-(bin*ratio)),yoffset);
	for(bin=0;bin<bintot;bin++) fprintf(fpout,"0 %d %.2f %d  bar\n",count[bin],((bin*ratio)+halfboxwidth),yoffset);
	fprintf(fpout,"	stroke\n");
	fprintf(fpout,"1 setlinewidth\n");
	fprintf(fpout,"\n");
	/* draw refractory lines */
	fprintf(fpout,"1 setlinewidth\n");
	fprintf(fpout,"newpath\n");
	fprintf(fpout,"	1 0 0 setrgbcolor\n");
	fprintf(fpout,"	%d %d moveto\n",halfboxwidth,yoffset);
	fprintf(fpout,"	0 %d rlineto\n",boxheight);
	fprintf(fpout,"	stroke\n");
	fprintf(fpout,"	0 0 0 setrgbcolor\n");

	fprintf(fpout,"\n");
	fprintf(fpout,"grestore\n");
	fprintf(fpout,"grestore\n");
	fprintf(fpout,"%cend_histoplot\n",37);
	fprintf(fpout,"\nshowpage\n");
	fclose(fpout);

	free(count);
	return;
}
