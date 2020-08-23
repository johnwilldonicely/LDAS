#define thisprog "xe-plotmerge1"
#define TITLE_STRING thisprog" v 1.6: 23.August.2020 [JRH]"
#define MAXLINELEN 1000
#define MAXFILES 256

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
<TAGS>plot</TAGS>

v 1.6: 23.August.2020 [JRH]
	- bugfix: do not apply grestore to the first of the merged plots, to avoid problems with newer versions of ghostscript - at the top of the file, there is nothing to restore!
	- updated variable names to new convention (ii,jj,nn etc)
	- resolved possible overrun issues with sprintf for basename and output file name
	- simplifed conditional loop - removed "filefound" variable
v 1.6: 2.July.2019 [JRH]
	- bugfix: longstanding issue resolved where an extra plot was always generated if ymax was reached but not exceeded
v 1.6: 29.September.2017 [JRH]
	- remove merge-type options (preserved header incompatible with newer plots)
v 1.6: 7.July.2017 [JRH]
	- change coordinate-shift variables from int to float
v 1.6: 9.April.2017 [JRH]
	- bugfix: close files after opening to test for presence
		- failure to do so was causing problems when the number of open files was large
v 1.6: 6.May.2014 [JRH]
	- fix printing of bounding box on line 2
	- now merge type-0 has option to remove internal tranlation commands in plots being merged
	- this is handy is merging a plot which has already been merged
v 1.5: 19.April.2014 [JRH]
	- clarify annotations and instructions for use of merge type zero
v 1.4: 20.June.2012 [JRH]
	- now will continue plotting if some of the files are missing
v 1.3: 13.July.2011 [JRH]
	- corrected numerical numbering of multiple output files
*/


/* external functions start */
/* external functions end */

int main (int argc, char *argv[]) {
	/* general variables */
	char line[MAXLINELEN],templine[MAXLINELEN],word[256],*matchstring=NULL,*pline,*pcol;
	long int ii,jj,kk,nn;
	int v,w,x,y,z;
	int sizeofchar=sizeof(char),sizeoflong=sizeof(long),sizeofint=sizeof(int);
	float a,b,c,d;
	double aa,bb,cc,dd;
	FILE *fpin,*fpout;
	/* program-specific variables */
	int filefound=0,lineout=0,fileopen=0;
	int page=1,nfiles=0,infilelen;
	float zx,zy;
	long *infileindex=NULL,index;

	/* arguments */
	char *infile=NULL,outbase[128],outfile[256];
	float zxstart=60,zystart=655; // starting coordinates for top left of plot
	float xinc=140,yinc=200; // pixel-shifts for each plot
	int xstep=0,ystep=0; // count how many plots have been made on each column & row
	int xstepmax=4,ystepmax=5; // no. of plots across and down the page- adjust depending on yinc & plot yscale!
	float setscale=1.0;

	int setformat=1,setnotrans=1;
	float setlow=0.0,sethigh=0.0,setbinwidth=0.0;

	snprintf(outbase,128,"temp_%s",thisprog);

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Merge postscript plot files created by _xe-plottable1\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [options] [file list]\n",thisprog);
		fprintf(stderr,"	[file list]: list of file names to merge\n");
		fprintf(stderr,"	NOTE: Any option not beginning with a \"-\" is treated as a file-name\n");
		fprintf(stderr,"VALID OPTIONS:\n");
		fprintf(stderr,"	-notrans : remove tranlation from within merged files(0=NO 1=YES) [%d] \n",setnotrans);
		fprintf(stderr,"	-xo, -yo : x and y origin (top left of page) [%g, %g] \n",zxstart,zystart);
		fprintf(stderr,"	-xinc, yinc : x and y pixel shifts between plots [%g, %g] \n",xinc,yinc);
		fprintf(stderr,"	-xmax, ymax : number of plts across and down a page [%d, %d] \n",xstepmax,ystepmax);
		fprintf(stderr,"	-scale : set the scale of the combined plot [%g] \n",setscale);
		fprintf(stderr,"	-out : define the output filebase-name [%s] \n",outbase);
		fprintf(stderr,"		NOTE: the number before the .ps is the page-number\n");
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s a.ps b.ps c.ps -scale 0.5 -xmax 6 -yo 1200\n",thisprog);
		fprintf(stderr,"	%s -scale 0.5 -xmax 6 -yo 1200 *.ps\n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	- one or more plot files named %s.[page].ps\n",outbase);
		fprintf(stderr,"	- e.g. %s.001.ps\n",outbase);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	if((infileindex=(long *)realloc(infileindex,(nfiles+2)*sizeofint))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
	nfiles=index=0;infileindex[nfiles]=0;

	/* READ THE FILENAMES AND OPTIONAL ARGUMENTS - DYNAMICALLY AND EFFICIENTLY ALLOCATE MEMORY FOR NEW FILENAMES */
	for(ii=1;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			// read optional arguments - must begin with a hyphen and be followed by a value
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-xmax")==0)    xstepmax=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-ymax")==0)    ystepmax=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-xinc")==0)    xinc=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-yinc")==0)    yinc=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-xo")==0)      zxstart=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-yo")==0)      zystart=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-scale")==0)   setscale=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-notrans")==0) setnotrans=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-out")==0)     snprintf(outbase,128,"%s",argv[++ii]);
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
		}
		else {  // store another name in the list and record the index to it
			infileindex[nfiles]=index;
			infilelen=strlen(argv[ii])+2;
			w= (index*sizeofchar) + ((infilelen+1)*sizeofchar);
			if((infile=(char *)realloc(infile,w))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
			if((infileindex=(long *)realloc(infileindex,(nfiles+2)*sizeoflong))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
			sprintf(infile+infileindex[nfiles],"%s",argv[ii]);
			index+= infilelen;
			nfiles+= 1;
	}}

	//for(i=0;i<nfiles;i++) printf("input file %d: %s\n",i,(infile+infileindex[ii]));
	if(setnotrans!=0&&setnotrans!=1) {fprintf(stderr,"\n--- Error[%s]: invlid -notrans (%d) - should be 0 or 1)\n\n",thisprog,setnotrans);exit(1);}

	/* OPEN THE OUTPUT FILE */
	snprintf(outfile,256,"%s.%03d.ps",outbase,page);
	if((fpout=fopen(outfile,"w"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" could not be opened for writing\n\n",thisprog,outfile);exit(1);}
	fileopen=1;

	/* READ INPUT FILES AND OUPUT POSTSCRIPT */
	nn= 0; zx= zxstart; zy= zystart;
	/* make sure there's at least one file to merge */
	z=0;for(ii=0;ii<nfiles;ii++) if((fpin=fopen(infile+infileindex[ii],"r"))!=0) { z++; fclose(fpin);}
	if(z==0) {fprintf(stderr,"\n--- Error[%s]: none of the files to be merged were found\n",thisprog);exit(1);}

	for(ii=0;ii<nfiles;ii++) {

		if((fpin=fopen(infile+infileindex[ii],"r"))==0) {
			fprintf(stderr,"--- Warning[%s]: file \"%s\" not found\n",thisprog,infile+infileindex[ii]);
			continue;
		}
		nn++;
		lineout=0;
		// SEARCH FOR KEYWORDS IF THIS FILE WAS GENERATED BY PLOTPOST
		while(fgets(line,MAXLINELEN,fpin)!=NULL){
			// ONLY PRINT POSTSCRIPT HEADER LINE ONCE
			if(strstr(line,"%!")>0) {
				if(nn==1) { fprintf(fpout,"%s",line); fprintf(fpout,"%%%%BoundingBox: 0 0 595 842\n\n"); }
				else fprintf(fpout,"grestore\n\n");

				fprintf(fpout,"\n%% PLOT_CODE_START : %s\n\n\n",(infile+infileindex[ii]));
				fprintf(fpout,"gsave\n");
				fprintf(fpout,"%.3f %.3f scale\n",setscale,setscale);
				fprintf(fpout,"%g %g translate\n",zx,zy);
				lineout=1;

			}
			else if(strstr(line,"showpage")>0) { lineout=0; fprintf(fpout,"\n%% PLOT_CODE_END : %s\n\n\n",(infile+infileindex[ii])); }
			else if(strstr(line,"%%BoundingBox")>0) continue;
			else if(strstr(line,"SHIFT_COORDINATES")>0 && setnotrans==1) continue;
			else if(strstr(line,"translate")>0 && strstr(line,"internal")<=0  && setnotrans==1) continue;
			else if(lineout==1) fprintf(fpout,"%s",line);
		}
		fclose(fpin);

		zx= zx+xinc;    // shift the zero-x coordinate
		xstep= xstep+1; // incriment the xstep counter

		/* DEAL WITH THE LAST PLOT ON THE ROW */
		if(xstep==xstepmax) {
			xstep= 0;     // reset the xstep counter
			zx= zxstart;  // reset the zero-x coordinate
		 	zy= zy-yinc;  // shift the zero-y coordinate
		 	ystep++;      // incriment the ystep counter
			if(ystep==ystepmax) {
				fprintf(fpout,"%% SHOW_PAGE %d\n",page);
				fprintf(fpout,"showpage\n");
				fclose(fpout);
				fileopen=0; page++; nn=ystep=0; zy=zystart;
				// if this is not the last input file, start a new plot
				if(ii<(nfiles-1)) {
					snprintf(outfile,256,"%s.%03d.ps",outbase,page);  // make a new output filename
					if((fpout=fopen(outfile,"w"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" could not be opened for writing\n\n",thisprog,outfile);exit(1);}
					fileopen=1;
				}
			} // END OF LOOP: if this is the last row of plots on the page
		} // END OF LOOP: if this is the last plot on the row
	} // END OF LOOP: for each file

	if(fileopen==1) {
		fprintf(fpout,"%% SHOW_PAGE %d\n",page);
		fprintf(fpout,"showpage\n");
		fclose(fpout);
	}

	exit(0);
}
