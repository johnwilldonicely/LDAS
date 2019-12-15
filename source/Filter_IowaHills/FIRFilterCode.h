//---------------------------------------------------------------------------

#ifndef FIRFilterCodeH
#define FIRFilterCodeH
#include "FFTCode.h"   // For the definition of TWindowType
//---------------------------------------------------------------------------

 #define MAX_NUMTAPS 256
 #define M_2PI  6.28318530717958647692
 #define NUM_FREQ_ERR_PTS  1000    // these are only used in the FIRFreqError function.
 #define dNUM_FREQ_ERR_PTS 1000.0

 enum TFIRPassTypes {firLPF, firHPF, firBPF, firNOTCH, ftNOT_FIR};

 void ExampleFIRCall(void);
 void FilterWithFIR(double *FirCoeff, int NumTaps, double *Signal, double *FilteredSignal, int NumSigPts);
 void FilterWithFIR2(double *FirCoeff, int NumTaps, double *Signal, double *FilteredSignal, int NumSigPts);
 void BasicFIR(double *FirCoeff, int NumTaps, TFIRPassTypes PassType, double OmegaC, double BW, TWindowType WindowType, double WinBeta);
 void WindowData(double *Data, int N, TWindowType WindowType, double Alpha, double Beta, bool UnityGain);
 double Sinc(double x);
 void FIRFreqError(double *Coeff, int NumTaps, int PassType, double *OmegaC, double *BW);

#endif
