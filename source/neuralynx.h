
#ifndef _NLX_DATATYPE_H
#define _NLX_DATATYPE_H

#define __int16 	short
#define __int32	int
#define __int64	long long


//#pragma HP_ALIGN NOPADDING 
//#pragma HP_ALIGN NOPADDING PUSH

#pragma pack(push, before_nlx_datatypes)
#pragma pack( 1 )

//////////////////
// Spike Datatypes
////////////////// 

//#define TT_NUMELECTRODES 4;
//#define ST_NUMELECTRODES 2; 
//#define SE_NUMELECTRODES 1; 
//#define MAX_PARAMS 8;
//#define SPIKE_NUMPOINTS 32;

const int TT_NUMELECTRODES=4;
const int ST_NUMELECTRODES=2; 
const int SE_NUMELECTRODES=1; 
const int MAX_PARAMS 	  =8;
const int SPIKE_NUMPOINTS =32;




struct TetPoint {
	__int16		snADVal[4];
};

struct StereoPoint {
	__int16		snADVal[2];
};

struct SinglePoint {
	__int16		snADVal[1];
};

struct TTRec	{
		unsigned __int64	qwTimeStamp;			// TS
 		unsigned __int32	dwScNumber;			// Channel number
		unsigned __int32	dwCellNumber;			// What cell was this calculated to be? filled in by online cluster analysis
		signed __int32		dnParams[8];			// signed __int32

		struct TetPoint		snData[32];			//TetPoint
};

struct STRec	{
		unsigned __int64	qwTimeStamp;			// See TTRec, above
		unsigned __int32	dwScNumber;
		unsigned __int32	dwCellNumber;
		signed __int32		dnParams[8];			//
		struct StereoPoint	snData[32];			//
};

struct SERec	{
		unsigned __int64	qwTimeStamp;			// See TTRec, above
		unsigned __int32	dwScNumber;
		unsigned __int32	dwCellNumber;
		signed __int32		dnParams[8];			
		struct SinglePoint	snData[32];
};


struct SCRec	{
		unsigned __int64	qwTimeStamp;
		unsigned __int32	dwScNumber;
		unsigned __int32	dwCellNumber;
		signed __int32	 	dnParams[8];	
};


struct SCMin {
		unsigned __int64	qwTimeStamp;
		unsigned __int32	dwCellNumber;		
};


const int MAX_CSC_SAMPLES = 512;


struct CRRec	{
		unsigned __int64	qwTimeStamp;			// TS
		unsigned __int32	dwChannelNum;			// Channel number
		unsigned __int32	dwSampleFreq;			// freq in hertz of sampling rate
		unsigned __int32	dwNumValidSamples;		// number of snSamples containing useful data
		signed __int16	 	snSamples[512];			//signed __int16	 	snSamples[MAX_CSC_SAMPLES];	// the A-D data samples
};


//////////////////
// Event Datatypes
//////////////////

const int EVENT_NUM_EXTRAS = 8;
const int NLX_EventRecStringSize = 128;

// record for EventAcqEnt objects
struct EventRec	{
  short   	nstx;             			// always 800 from DCDCB
  short   	npkt_id;          			// DCDCB ID  1002, etc.
  short   	npkt_data_size;   			// always 2
  long long	qwTimeStamp;
  short   	nevent_id;       			// just an id value
  short   	nttl;            			// TTL input value
  short   	ncrc;            			// from the DCDCB
  short   	ndummy1;          			// just a place holder
  short   	ndummy2;          			// just a place holder
  unsigned int	dnExtra[8];       			// unsigned int	dnExtra[EVENT_NUM_EXTRAS]; 		// extra "bit values"
  char		EventString[128]; 	// char		EventString[NLX_EventRecStringSize];// char string for user input events
};


//////////////////////////
// Video Tracker Datatypes
//////////////////////////

const int NLX_VTREC_NUM_POINTS = 400;
const int NLX_VTREC_NUM_TARGETS = 50;

struct VideoRec	{
  unsigned __int16	swstx;		// should be 800 as from the DCDCB
  unsigned __int16	swid;		// should be 0x1000 or 0x1001 as from the DCDCB
  unsigned __int16	swdata_size;	// should be 800 as from the DCDCB
  unsigned __int64	qwTimeStamp;	// TS
  unsigned __int32	dwPoints[400];	//unsigned __int32	dwPoints[NLX_VTREC_NUM_POINTS];// the points with color bit values x&y - note: this is a bit field!
  signed __int16	sncrc;		// as from the dcdcb
  signed __int32	dnextracted_x;	// from our extraction algorithm
  signed __int32	dnextracted_y;	// from our extraction algorithm
  signed __int32	dnextracted_angle;	// unimplemeted - equal to (5 * numtargets) in version 2.02
  signed __int32	dntargets[50];	//signed __int32	dntargets[NLX_VTREC_NUM_TARGETS];// colored targets with same format as the points
};



const unsigned int VREC_COLOR_MASK = 0x7000F000;  // logical OR of all the colors
const unsigned int VREC_LU_MASK = 0x8000;	// luminance mask
const unsigned int VREC_RR_MASK = 0x4000;	// pure & raw RGB masks
const unsigned int VREC_RG_MASK = 0x2000;
const unsigned int VREC_RB_MASK = 0x1000;
const unsigned int VREC_PR_MASK = 0x40000000;
const unsigned int VREC_PG_MASK = 0x20000000;
const unsigned int VREC_PB_MASK = 0x10000000;
const unsigned int VREC_RS_MASK = 0x80000000;	// reserved bit mask
const unsigned int VREC_X_MASK = 0x00000FFF;	// x value mask
const unsigned int VREC_Y_MASK = 0x0FFF0000;	// y val



//#pragma HP_ALIGN NOPADDING POP

#endif  //_NLX_DATATYPE_H

