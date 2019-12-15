/******************************************************************************
Assemble postscript files into summary report
One file produced per page
Assume page dimensions = 595 x 841 pixels
Rotate page by 90 degress to "landscape" orientation (dimensions 841 x 595, range = 0 to -841, 0 to 595)
In rotated coordinates, 0,0 = top left, x increases to right, and y extends to negative nubers as you go down
Recommended map scale = 0.16 for A4 paper printed landscape
******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void hux_error(char error_txt[]);
void hux_winpath(char *line);

void hux_psratemap_assemble (
	unsigned int *spikecount, /* 2D probe*cluster array holding number of spikes in each cluster */
	int clusterlimit,	/* cluster limits for spikecount array */
	int probelimit,		/* probe limits for spikecount array */
	int probemax,		/* what's the highest probe number */
	float vidratio,		/* x/y video aspect ratio */
	float fontsize, 	/* in points */
	int datatype,		/* which type of records (eg. Neuralynx, Csicsvari lab, O'Keefe etc) */
	char *filebase1,	/* base map-input filename, including path (filebaseP01_C01.ps, filebaseP01_C02.ps, etc) */
	char *filebase2,	/* base isi histogram-input filename, */
	char *filebase3,	/* base output filename */
	int system,		/* 0=unix/linux, 1=windows */
	int mapcompact,		/* 0=one probe per line, 1=clusters wrap to next line */
	int clean		/* 0=keep input postscript files, 1=delete them */
	)
{
	char infile1[256],infile2[256],outfile[256],command[256],line[256];
	int maptransx=256, maptransy=249, mappageborder, headerprinted;
	int i,x,probe=-1,cluster=-1,probestart,clustart,linecount,colcount,filecount;
	int pagelinemax=8, pagecolmax=16;
	float mapscale=0.175;
	FILE *fpin1,*fpin2,*fpout;

	if(vidratio>1) maptransy = (int) (maptransy/vidratio); /* if x-range > y-range, scale down y-dimensions */
	if(vidratio<1) maptransx = (int) (maptransx/vidratio); /* if y-range > x-range, scale down x-dimensions */
	maptransx = (int) (1*maptransx + 20); 	/* add 20 pixels for horizontal space between maps */
	maptransy = (int) (-maptransy -150 -20 - (int)(2.0*fontsize)); /* calculate per-map y-offsets (150 = size of histogram box) */

	mappageborder=30; /* unscaled offset from edges of paper */
	mappageborder=(int)((float)mappageborder/mapscale); /* offset adjusted for scaling */

	if(datatype==1) {probestart=0; clustart=1;} /* Neuralynx */
	if(datatype==2) {probestart=0; clustart=2;} /* Csicsvari */
	if(datatype==3) {probestart=1; clustart=1;} /* Axona */

	fpout=0; /* set file pointer to zero so we can check if any files need to be closed */
	filecount=0; 	/* which block of "pagelinemax" probes are we in */
	linecount=1;	/* which probe within the block is this */
	colcount=0; /* which cluster on the line is this */
	headerprinted=0; 

	sprintf(outfile,"%s%02d.ps\0",filebase3,filecount);
	if((fpout=fopen(outfile,"w"))==NULL) {sprintf(command,"can't open \"%s\"\n\n",outfile);hux_error(command);}

	/* The probe loop starts here */
	for(probe=probestart;probe<=probemax;probe++) {
		x=0; for(i=clustart;i<clusterlimit;i++) {if(spikecount[probe*clusterlimit+i]>0) {x=1;break;}}
		if(x==1 && mapcompact==0) linecount++;
		if(x==0 && mapcompact==1) continue;
		if(linecount>pagelinemax) { 
			filecount++; linecount=1; headerprinted=0;
			if(fpout!=0) {fprintf(fpout,"showpage\n"); fclose(fpout);} /* if file has been opened for last set, close it */
			sprintf(outfile,"%s%02d.ps\0",filebase3,filecount);
			if((fpout=fopen(outfile,"w"))==NULL) {sprintf(command,"can't open \"%s\"\n\n",outfile);hux_error(command);}
		}
		/* the cluster loop starts here */ 
		if(mapcompact==0)	colcount=0; 
		for(cluster=clustart;cluster<clusterlimit;cluster++) {
			if(spikecount[probe*clusterlimit+cluster]<=0) {continue;}
			colcount++;
			/* check if maps need to be "wrapped" to next line - if so, it gets a little tricky...*/
			if(colcount > pagecolmax && mapcompact==1) {
				colcount=1;
				linecount++;
				/* the following is simply a copy of the code used in the "probe loop" in case linecount number exceeds page limits */
				/* not that only the probe and file counters, not the probe number is incrimented */
				if(linecount>pagelinemax) { 
					filecount++; linecount=1; headerprinted=0;
					if(fpout!=0) {fprintf(fpout,"showpage\n"); fclose(fpout);} /* if file has been opened for last set, close it */
					sprintf(outfile,"%s%02d.ps\0",filebase3,filecount);
					if((fpout=fopen(outfile,"w"))==NULL) {sprintf(command,"can't open \"%s\"\n\n",outfile);hux_error(command);}
				}
			} 
			
			/* open map file and isi file for this probe & cluster */
			sprintf(infile1,"%sP%02d_C%02d.ps\0",filebase1,probe,cluster);
			if((fpin1=fopen(infile1,"r"))==NULL) {sprintf(command,"can't open postscript sub-map \"%s\"\n\n",infile1);hux_error(command);}
			sprintf(infile2,"%sP%02d_C%02d.ps\0",filebase2,probe,cluster);
			if((fpin2=fopen(infile2,"r"))==NULL) {sprintf(command,"can't open postscript histogram \"%s\"\n\n",infile2);hux_error(command);}
			/*  if this is the first probe in a block of 8, print header and map title (filename) */
			if(headerprinted==0) { 
				headerprinted=1;
				fprintf(fpout,"%% START_HEADER\n");
				while(fgets(line,1000,fpin1)!=NULL && strncmp(line,"gsave",5)!=0) fprintf(fpout,"%s",line); /* print header portion of the file defining colours etc. */
				fprintf(fpout,"90 rotate\n"); /* rotate map to increase space for cells */
				if(system==1) hux_winpath(outfile);
				fprintf(fpout,"%.3f %.3f scale\n",mapscale,mapscale);  /* set scaling for maps and horzontal page offset */
				fprintf(fpout,"/Helvetica findfont %.2f scalefont setfont\n",fontsize/(mapscale*2.0));
				fprintf(fpout,"%d %d moveto\n(%s) show\n\n",mappageborder,(-1*mappageborder),outfile); /* print map title */	
				fprintf(fpout,"/Helvetica findfont %.2f scalefont setfont\n",fontsize);
				fprintf(fpout,"%% END_HEADER\n");
			}
			else while(fgets(line,1000,fpin1)!=NULL && strncmp(line,"gsave",5)!=0) {}	/* for all other files, skip header section */

			/* start by saving graphics state, then translate, write code, and restore */
			fprintf(fpout,"gsave\n\n"); 
			fprintf(fpout,"%d %d translate\n",(maptransx*(colcount-1)+mappageborder),(maptransy*linecount));

			/* insert place map code, having already read past header portion */ 
			fprintf(fpout,"%% PLOT_CODE_START_%02d_%02d\n",probe,cluster);
			while(fgets(line,1000,fpin1)!=NULL) {if(strncmp(line,"showpage",8)!=0) fprintf(fpout,"%s",line);}
			fprintf(fpout,"0 -165 translate\n"); /* within the translated coordinate system, translate for histograms */
			/* insert histogram code, skipping beginning */
			while(fgets(line,1000,fpin2)!=NULL && strncmp(line,"%start_histoplot",16)!=0) {}
			while(fgets(line,1000,fpin2)!=NULL && strncmp(line,"%end_histoplot",14)!=0) {fprintf(fpout,"%s",line);}
			fprintf(fpout,"grestore\n"); /*translation undone for each map - ie not cumulative */ 
			fprintf(fpout,"%% PLOT_CODE_END_%02d_%02d\n",probe,cluster);

			fclose(fpin1); fclose(fpin2);
			if(clean==1) {remove(infile1); remove(infile2);}
		} /* end of cluster loop */
	} /* end of probe loop */

	fprintf(fpout,"showpage\n");
	fclose(fpout);
	return;
}

