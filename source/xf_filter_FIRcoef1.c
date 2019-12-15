/*
<TAGS>signal_processing filter</TAGS>

DESCRIPTION:

	Calculate Finite Impulse Response (FIR) filter coefficients
	Derived from the Iowa Hills Software collection:

		Iowa Hills Software, LLC
		http://www.iowahills.com/5FIRFiltersPage.html
		November 19, 2014

	This implimentation:
		- combines multiple functions and header-definitions into a single function
		- assumes corection to coefficients based on accurate 3dB cutoffs is desired
		- currently only includes a single windowing option (Kaiser), or "none"


USES:
	Calculate Finite Impulse Response (FIR) filter coefficients

DEPENDENCIES (included here):
	void BasicFIR(double *FIRcoef, double *tempcoef, int NumTaps, int PassType, double OmegaC, double BW, int WindowType, double WinBeta);
	void FIRFreqError(double *Coeff, int NumTaps, int PassType, double *OmegaC, double *BW);
	double Goertzel(double *Samples, int N, double Omega);
	double Bessel(double x);
	double Sinc(double x) ;
	No dependencies

ARGUMENTS:
	int NumTaps      : desired number of taps for the filter - 3-256, 41 is typical, more taps = better filtering
	int PassType     : 1=LP, 2=HP, 3=BP, 4=notch
	double OmegaC    : corner (LP,HP) or central (BP, notch) frequency as a proportion of Pi (2.0 * Hz/samplerate)
	double BW        : bandwidth as a proportion of Pi (2.0 * Hz/samplerate)
	char *WindowType : none,kaiser,sinc
	double WinBeta   : 0-10, used for the Kaiser window
	char *message    : character array to hold message

RETURN VALUE:
	pointer to array of coefficients, or NULL on fail

*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#define NUM_FREQ_ERR_PTS  1000    // these are only used in the FIRFreqError function.
#define dNUM_FREQ_ERR_PTS 1000.0

void BasicFIR(double *FIRcoef, double *tempcoef, int NumTaps, int PassType, double OmegaC, double BW, char *WindowType, double WinBeta);
void FIRFreqError(double *Coeff, int NumTaps, int PassType, double *OmegaC, double *BW);
double Goertzel(double *Samples, int N, double Omega);
double Bessel(double x);
double Sinc(double x) ;

double *xf_filter_FIRcoef1(int NumTaps, int PassType, double OmegaC, double BW, char *WindowType, double WinBeta, char *message) {

	long i,j,k;
	char *thisfunc="xf_filter_FIRcoef1\0";
	double OrigOmega = OmegaC;
	double OrigBW = BW;
	double *FIRcoef=NULL, *tempcoef=NULL;

	sprintf(message,"%s (success)",thisfunc);

	if((FIRcoef=(double*)calloc(NumTaps,sizeof(double)))==NULL) {
		sprintf(message,"%s [ERROR]: insufficient memory",thisfunc);
		return(NULL);
	}
	if((tempcoef=(double*)calloc(NumTaps,sizeof(double)))==NULL) {
		sprintf(message,"%s [ERROR]: insufficient memory",thisfunc);
		return(NULL);
	}
	if(
		strcmp(WindowType,"none")!=0 &&
		strcmp(WindowType,"kaiser")!=0 &&
		strcmp(WindowType,"sync")!=0
	) { sprintf(message,"%s [ERROR]: invalid WindowType specified (%s)",thisfunc,WindowType);return(NULL); }

	// First pass at generating coefficients
	BasicFIR(FIRcoef,tempcoef, NumTaps, PassType, OmegaC, BW, WindowType, WinBeta);

	// This function corrects OmegaC for LPF and HPF. It corrects BW for BPF and Notch.
	FIRFreqError(FIRcoef, NumTaps, PassType, &OmegaC, &BW);

	// Recalculate the filter with the corrected OmegaC and BW values.
	BasicFIR(FIRcoef, tempcoef, NumTaps, PassType, OmegaC, BW, WindowType, WinBeta);

	OmegaC = OrigOmega; // Restore these in case they are needed.
	BW = OrigBW;

	free(tempcoef);
	return(FIRcoef);

}




//---------------------------------------------------------------------------
/*
 This code generates the impulse response for a rectangular low pass, high pass,
 band pass, or notch filter. Then a window, such as the Kaiser, is applied to it.

 double FIRcoef[MAXNUMTAPS];
 int NumTaps;                        NumTaps can be even or odd and < MAXNUMTAPS
 int PassType;                       1=LP, 2=HP, 3=BP, 4=Notch
 double OmegaC  0.0 < OmegaC < 1.0   The corner freq, or center freq if BPF or NOTCH
 double BW      0.0 < BW < 1.0       The band width if BPF or NOTCH
 int WindowType;                     unused at present - Kaiser is default
 double Beta;  0 <= Beta <= 10.0     Beta is used with the Kaiser, Sinc windows.
*/
void BasicFIR(double *FIRcoef, double *tempcoef, int NumTaps, int PassType, double OmegaC, double BW, char *WindowType, double WinBeta) {
	int j,M,TopWidth;
	double Arg,OmegaLow,OmegaHigh,dM,Alpha;

	Alpha=0.0;
	if(WinBeta < 0.0)WinBeta = 0.0;
	if(WinBeta > 10.0)WinBeta = 10.0;

	if(PassType==1){ // firLPF:
		for(j=0; j<NumTaps; j++) {
		 Arg = (double)j - (double)(NumTaps-1) / 2.0;
		 FIRcoef[j] = OmegaC * Sinc(OmegaC * Arg * M_PI);
	}}
	else if(PassType==2){ // firHPF:
		if(NumTaps % 2 == 1) {// Odd tap counts
			for(j=0; j<NumTaps; j++) {
				Arg = (double)j - (double)(NumTaps-1) / 2.0;
				FIRcoef[j] = Sinc(Arg * M_PI) - OmegaC * Sinc(OmegaC * Arg * M_PI);
		}}
		else { // Even tap counts
			for(j=0; j<NumTaps; j++) {
				Arg = (double)j - (double)(NumTaps-1) / 2.0;
				if(Arg == 0.0)FIRcoef[j] = 0.0;
				else FIRcoef[j] = cos(OmegaC * Arg * M_PI) / M_PI / Arg  + cos(Arg * M_PI);
		}}
	}
	else if(PassType==3){ // firBPF:
		OmegaLow  = OmegaC - BW/2.0;
		OmegaHigh = OmegaC + BW/2.0;
		for(j=0; j<NumTaps; j++) {
			Arg = (double)j - (double)(NumTaps-1) / 2.0;
			if(Arg == 0.0)FIRcoef[j] = 0.0;
			else FIRcoef[j] =  ( cos(OmegaLow * Arg * M_PI) - cos(OmegaHigh * Arg * M_PI) ) / M_PI / Arg ;
	}}
	else if(PassType==4){ // firNOTCH:
		// If NumTaps is even for Notch filters, the response at Pi is attenuated.
		OmegaLow  = OmegaC - BW/2.0;
		OmegaHigh = OmegaC + BW/2.0;
		for(j=0; j<NumTaps; j++) {
			Arg = (double)j - (double)(NumTaps-1) / 2.0;
			FIRcoef[j] =  Sinc(Arg * M_PI) - OmegaHigh * Sinc(OmegaHigh * Arg * M_PI) - OmegaLow * Sinc(OmegaLow * Arg * M_PI);
	}}


	// WINDOW THE COEFFICIENTS
	if(strcmp(WindowType,"none")!=0) {

		if(strcmp(WindowType,"kaiser")==0) {
			// Calculate the Kaiser window for N/2 points, then fold the window over (at the bottom)
			// a simple approximation to the DPSS window based upon Bessel functions, generally known as the Kaiser window (or Kaiser-Bessel window)
			dM = NumTaps + 1;
			for(j=0;j<NumTaps;j++) {
				Arg = WinBeta * sqrt(1.0 - pow( ((double)(2*j+2) - dM) / dM, 2.0) );
				tempcoef[j] = Bessel(Arg) / Bessel(WinBeta);
			}
		}
		if(strcmp(WindowType,"sync")==0) {
			dM = NumTaps + 1;
			for(j=0;j<NumTaps;j++) tempcoef[j]= Sinc((double)(2*j+1-NumTaps)/dM * M_PI );
   			for(j=0;j<NumTaps;j++) tempcoef[j]= pow(tempcoef[j],WinBeta);
		}

		// Fold the coefficients over.
		for(j=0;j<NumTaps/2;j++) tempcoef[NumTaps-j-1] = tempcoef[j];
		// Apply the window
		for(j=0;j<NumTaps;j++) FIRcoef[j] *= tempcoef[j];
	}

// END OF INTERNAL FUNCTION
}



//---------------------------------------------------------------------------
// We normally specify the 3 dB frequencies when specifing a filter. The Parks McClellan routine
// uses OmegaC and BW to set the 0 dB band edges, so its final OmegaC and BW values are not close to -3 dB.
// The Windowed filters are better for high tap counts, but for low tap counts, their 3 dB frequencies
// are also well off the mark.

// To use this function, first calculate a set of FIR coefficients, then pass them here, along with OmegaC
// and BW. This calculates a corrected OmegaC for low and high pass filters. It calcultes a corrected BW
// for band pass and notch filters. Use these corrected values to recalculate the FIR filter.

// The Goertzel algorithm is used here to calculate the filter's magnitude response at loop omgega.
// This function finds the 3 dB freq by starting in the pass band and working out. The loops
// stop looking (break) at the -20 dB frequency.

void FIRFreqError(double *Coeff, int NumTaps, int PassType, double *OmegaC, double *BW) {
	int j, J3dB, CenterJ;
	double Omega, CorrectedOmega, CorrectedBW, Omega1=NAN, Omega2=NAN, Mag;

	// In these loops, we break well past the 3 dB pt so that large ripple is ignored.
	if(PassType == 1) { // PassType == firLPF
		J3dB = 10;
		for(j=0; j<NUM_FREQ_ERR_PTS; j++) {
			Omega = (double)j / dNUM_FREQ_ERR_PTS;
			Mag = Goertzel(Coeff, NumTaps, Omega);
			if(Mag > 0.707)J3dB = j;
			if(Mag < 0.1)break;
		}
		Omega = (double)J3dB / dNUM_FREQ_ERR_PTS;
	}

	else if(PassType == 2) {// PassType == firHPF
		J3dB = NUM_FREQ_ERR_PTS - 10;
		for(j=NUM_FREQ_ERR_PTS-1; j>=0; j--) {
			Omega = (double)j / dNUM_FREQ_ERR_PTS;
			Mag = Goertzel(Coeff, NumTaps, Omega);
			if(Mag > 0.707)J3dB = j;
			if(Mag < 0.1)break;
		}
		Omega = (double)J3dB / dNUM_FREQ_ERR_PTS;
	}

	else if(PassType == 3) { // PassType == firBPF
		CenterJ = dNUM_FREQ_ERR_PTS * *OmegaC;
		J3dB = CenterJ;
		for(j=CenterJ; j>=0; j--) {
			Omega = (double)j / dNUM_FREQ_ERR_PTS;
			Mag = Goertzel(Coeff, NumTaps, Omega);
			if(Mag > 0.707)J3dB = j;
			if(Mag < 0.1)break;
		}
		Omega1 = (double)J3dB / dNUM_FREQ_ERR_PTS;
		J3dB = CenterJ;
		for(j=CenterJ; j<NUM_FREQ_ERR_PTS; j++) {
			Omega = (double)j / dNUM_FREQ_ERR_PTS;
			Mag = Goertzel(Coeff, NumTaps, Omega);
			if(Mag > 0.707)J3dB = j;
			if(Mag < 0.1)break;
		}
		Omega2 = (double)J3dB / dNUM_FREQ_ERR_PTS;
	}

	// The code above starts in the pass band. This starts in the stop band.
	else if(PassType == 4) { // PassType == firNOTCH
		CenterJ = dNUM_FREQ_ERR_PTS * *OmegaC;
		J3dB = CenterJ;
		for(j=CenterJ; j>=0; j--) {
			Omega = (double)j / dNUM_FREQ_ERR_PTS;
			Mag = Goertzel(Coeff, NumTaps, Omega);
			if(Mag <= 0.707)J3dB = j;
			if(Mag > 0.99)break;
		}
		Omega1 = (double)J3dB/dNUM_FREQ_ERR_PTS;

		J3dB = CenterJ;
		for(j=CenterJ; j<NUM_FREQ_ERR_PTS; j++) {
			Omega = (double)j / dNUM_FREQ_ERR_PTS;
			Mag = Goertzel(Coeff, NumTaps, Omega);
			if(Mag <= 0.707)J3dB = j;
			if(Mag > 0.99)break;
		}
		Omega2 = (double)J3dB / dNUM_FREQ_ERR_PTS;
	}


	// This calculates the corrected OmegaC and BW and error checks the values.
	if(PassType == 1 || PassType == 2 ) {
		CorrectedOmega = *OmegaC * 2.0 - Omega;  // This is usually OK.
		if(CorrectedOmega < 0.001)CorrectedOmega = 0.001;
		if(CorrectedOmega > 0.99)CorrectedOmega = 0.99;
		*OmegaC = CorrectedOmega;
	}

	else { // PassType == firBPF || PassType == firNOTCH
		CorrectedBW = *BW * 2.0 - (Omega2 - Omega1);  // This routinely goes neg with Notch.
		if(CorrectedBW < 0.01)CorrectedBW = 0.01;
		if(CorrectedBW > *BW * 2.0)CorrectedBW = *BW * 2.0;
		if(CorrectedBW > 0.98)CorrectedBW = 0.98;
		*BW = CorrectedBW;
	}
// END OF INTERNAL FUNCTION
}



//-----------------------------------------------------------------------------
// This is used in FIRFreqError
// Goertzel is essentially a single frequency DFT, but without phase information.
// Its simplicity allows it to run about 3 times faster than a single frequency DFT.
// It is typically used to find a tone embedded in a signal. A DTMF tone for example.
// 256 pts in 6 us
double Goertzel(double *Samples, int N, double Omega) {
	int j;
	double Reg0, Reg1, Reg2;        // 3 shift registers
	double CosVal, Mag;

	Reg1 = Reg2 = 0.0;

	CosVal = 2.0 * cos(M_PI * Omega );
	for (j=0; j<N; j++) {
		Reg0 = Samples[j] + CosVal * Reg1 - Reg2;
		Reg2 = Reg1;  // Shift the values.
		Reg1 = Reg0;
	}

	Mag = Reg2 * Reg2 + Reg1 * Reg1 - CosVal * Reg1 * Reg2;

	if(Mag > 0.0)Mag = sqrt(Mag);
	else Mag = 1.0E-12;

	return(Mag);
}


//-----------------------------------------------------------------------------
// This gets used with the Kaiser window in BasicFIR
double Bessel(double x) {
	double Sum=0.0, XtoIpower;
	int i, j, Factorial;
	for(i=1; i<10; i++) {
		XtoIpower = pow(x/2.0, (double)i);
		Factorial = 1;
		for(j=1; j<=i; j++) Factorial *= j;
		Sum += pow(XtoIpower / (double)Factorial, 2.0);
	}
	return(1.0 + Sum);
}


//---------------------------------------------------------------------------
// This gets used in numerous places in BasicFIR
double Sinc(double x) {
	if(x > -1.0E-5 && x < 1.0E-5)return(1.0);
	return(sin(x)/x);
}
