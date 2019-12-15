/************************************************************************
read_pos_nlx reads Neuralynx position records
- requires #include "neuralynx.h" in calling function
*************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "neuralynx.h"

extern int hux_getword(char *line, char *trigger, int word, char *target);
extern void hux_error(char message[]);

int hux_readpos_nlx (
	char *infile, /* filename to read*/
	int nleds,  /* the number of LEDs to detect */
	int pled, /* which "principal" LED represents true position: 0=largest, 1,2,3 = R,G,B */
        double *pos_time, /* storage for timestamps (seconds)*/
        float *pos_x, /* storage for position (cm) */
	float *pos_y, /* storage for position (cm) */
	float *led_dist, /* storage for distance between red & blue LEDs (cm) */
	float vidres, /* from calling function: pixels/cm resolution */
	float *result /* 0=postot,1=vidxmax,2=vidymax,3=samplefreq*/
	) 
{
	int i,j,k,w,x,y,z,postot,posvalid,size,badrangetot=0;
	int headerlength=16384,timedivisor=1000000; /* values potentially obtained from Neuralynx header file */
	char *header,*target,command[256];
	float posfreq,posinterval,posinvalid=-1.0,vidxmax=641,vidymax=481,tempx,tempy,redx,redy,greenx,greeny,bluex,bluey;
	FILE *fpin;
	/* 	The following alternative video record structure is based on the neuralynx original */
	struct NlxBitField { /* define elements of the 32-bit "target" field (right to left) and size in bits */ 
		unsigned int x : 12;
		unsigned int rawblue : 1;
		unsigned int rawgreen : 1;
		unsigned int rawred: 1;
		unsigned int luminance : 1;
		unsigned int y : 12;
		unsigned int pureblue : 1;
		unsigned int puregreen : 1;
		unsigned int purered: 1;
		unsigned int reserved : 1;
	};
	struct NlxVideoRec	{ /* based on neuralynx VideoRec structure, same element names but targets element now based on the NlXBitField structure */
		unsigned __int16	swstx;		// should be 800 as from the DCDCB
		unsigned __int16	swid;		// record id: should be 0x1000 or 0x1001 as from the DCDCB
		unsigned __int16	swdata_size;	// record size: should be 800 as from the DCDCB
		unsigned __int64	qwTimeStamp;	// Timestamp in microseconds
		unsigned __int32	dwPoints[NLX_VTREC_NUM_POINTS];	// Pixel data unsigned __int32	dwPoints[NLX_VTREC_NUM_POINTS];// the points with color bit values x&y - note: this is a bit field!
		signed __int16	sncrc;		// as from the dcdcb
		signed __int32	dnextracted_x;	// from our extraction algorithm
		signed __int32	dnextracted_y;	// from our extraction algorithm
		signed __int32	dnextracted_angle;	// unimplemeted - equal to (5 * numtargets) in version 2.02
		NlxBitField targets[NLX_VTREC_NUM_TARGETS];	// 32-bit "target" field [NLX_VTREC_NUM_TARGETS];// colored targets with same format as the points
	} VtRec;

	if(nleds>5) hux_error("[hux_readpos_nlx]: cannot detect more than 5 LEDs");


	size=sizeof(struct NlxVideoRec); /* define size of video record structure */
	header = (char *) calloc((headerlength+1),sizeof(char));
	target = (char *) calloc((headerlength+1),sizeof(char));
	if(header==0||target==0) hux_error("hux_readpos_nlx out of memory storing header info");

	fpin=fopen(infile,"rb");
	if(fpin==NULL) {sprintf(command,"hux_readpos_nlx can't open \"%s\"\n\n",infile);	hux_error(command);}
	
	/* read header to determine video resolution (if unavailable, 640x480 is assumed)*/
	rewind(fpin); i=0;while(fread(&header[i],sizeof(char),1,fpin)!=0 && i<headerlength) i++;

	// get video resolution - for older versions of Neuralynx (v4.97 and below)
	if(hux_getword(header, "-Resolution", 1, target)>0) vidxmax=atoi(target);
	if(hux_getword(header, "-Resolution", 2, target)>0) vidymax=atoi(target);
	// get video resolution - for newer versions of Neuralynx (v5.01 and up)
	if(hux_getword(header, "-Resolution", 1, target)>0) vidxmax=atoi(target);
	if(hux_getword(header, "-Resolution", 3, target)>0) vidymax=atoi(target);

	// Store video data: invert y-coordinates and scale to cm
	postot = -1; posvalid=0;
	fseek(fpin,headerlength,SEEK_SET);
	while (fread(&VtRec,size,1,fpin)!=0) {
		postot++; /* position array is zero-offset - range from 0 to postot-1 */
		pos_time[postot] = (double)(VtRec.qwTimeStamp)/timedivisor; /* timestamp data is 64 bit int */
		redx=redy=greenx=greeny=bluex=bluey= posinvalid;
		pos_x[postot]=led_dist[postot]=pos_y[postot]= posinvalid; /* initialise to invalid value */

		// Find pure-colour spots in three largest targets
		w=0; //reset temporary "out of range" indicator
		for(i=nleds-1;i>=0;i--) { // if there is more than one pure spot of a given colour, position defaults to that of the largest
			// make temporary x & y holders
			tempx=(float)VtRec.targets[i].x; tempy=(float)VtRec.targets[i].y;
			z=VtRec.targets[1].luminance;
			// if either x or y is out of range, make x invalid
			if(tempx<=0||tempx>vidxmax||tempy<=0||tempy>vidymax) {tempx=posinvalid;w=1;}
			// if x is invalid so is y, and vice-versa
			if(tempx==posinvalid || tempy==posinvalid) tempx=tempy=posinvalid;
			else {tempx/=vidres; tempy=(vidymax-tempy)/vidres;}
  			// assign a colour x/y only if target is "pure"
 			z=VtRec.targets[i].purered + VtRec.targets[i].puregreen + VtRec.targets[i].pureblue;
			if(z==1) {
				if(VtRec.targets[i].purered==1) {redx=tempx; redy=tempy;}
				if(VtRec.targets[i].puregreen==1) {greenx=tempx; greeny=tempy;}
				if(VtRec.targets[i].pureblue==1) {bluex=tempx; bluey=tempy;}
		}}
		badrangetot += w;

		// decide which LED represents the true position, based on the "pled" variable
		if(pled==0) {pos_x[postot]=tempx; pos_y[postot]=tempy;}
		if(pled==1) {pos_x[postot]=redx; pos_y[postot]=redy;}
		if(pled==2) {pos_x[postot]=greenx; pos_y[postot]=greeny;}
		if(pled==3) {pos_x[postot]=bluex; pos_y[postot]=bluey;}

		if(pos_x[postot]!=posinvalid) posvalid++;

		if(redx!=posinvalid&&bluex!=posinvalid) led_dist[postot] = sqrt((redx-bluex)*(redx-bluex) + (redy-bluey)*(redy-bluey));
	}
	fclose(fpin);
	
	if(pos_time[postot]-pos_time[0]>0)	{
		posfreq = (postot+1.0)/(pos_time[postot]-pos_time[0]);
		posinterval = (pos_time[postot]-pos_time[0])/(postot+1.0);
	}
	else {posfreq = posinterval = -1.0;}

	result[0]= (float) postot+1;
	result[1]= (float) vidxmax;
	result[2]= (float) vidymax;
	result[3]= (float) posfreq;
	result[4]= (float) posinterval;
	result[5]= (float) badrangetot;

	free(header);
	free(target);
	return(posvalid);
}
