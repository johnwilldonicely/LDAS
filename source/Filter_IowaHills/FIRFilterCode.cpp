
// By Iowa Hills Software, LLC  IowaHills.com
// If you find a problem, please leave a note at:
// http://www.iowahills.com/feedbackcomments.html
// November 19, 2014

#include "FIRFilterCode.h"
#include <math.h>
#include "FFTCode.h"  // The Kaiser and Sinc windows are defined here.

//---------------------------------------------------------------------------

// This shows an example of how to use the functions in this and the ParksMcClellan file.
// These define's and include's are only needed for ExampleFIRCall
// Main and TopForm are used to test the code kit at Iowa Hills.
#define NUM_PLOT_PTS 2048
#include <stdio.h>
#include "NewParksMcClellan.h"
#include "FFTCode.h"


void ExampleFIRCall(void)
{
 int k;
 bool FreqCorrection = true;
 double RealHofZ[NUM_PLOT_PTS];       // Real and imag parts of H(z). Used with the FFT.
 double ImagHofZ[NUM_PLOT_PTS];
 int NumTaps = 40;                    // 4 <= NumTaps < 256 for windowed FIR
                                      // Should be odd for windowed high pass.
                                      // 9 <= NumTaps < 127 for Parks McClellan
                                      // Must be odd for Parks high Pass and notch (checked in the PM code)
 double FIRCoeff[MAX_NUMTAPS];        // FIR filter coefficients.  MAX_NUMTAPS = 256
 double OmegaC = 0.2;                 // 0.0 < OmegaC < 1.0
 double BW = 0.1;                     // 0.0 < BandWidth < 1.0
 TFIRPassTypes PassType = firLPF;     // firLPF, firHPF, firBPF, firNOTCH  See the header file.
 TWindowType WindowType = wtKAISER;   // wtNONE, wtKAISER, wtSINC, and others.   See the FFT header file.
 double WinBeta = 4.0;                // 0 <= WinBeta <= 10.0  Transition bandwidth for Kaiser and Sinc windows.
 double ParksWidth = 0.1;             // 0.01 <= ParksWidth <= 0.3 The transition bandwidth.
                                      // 0.01 <= ParksWidth <= 0.15 if BPF or NOTCH or if NumTaps > 70


 // Use BasicFIR or NewParksMcClellan to calculate the FIR Coefficients.
 // It is possible to use a window with Parks McClellan, but not recommended. Use WindowType = wtNONE.
 // Frequncy correction is optional, but usually necessary.

 BasicFIR(FIRCoeff, NumTaps, PassType, OmegaC, BW, WindowType, WinBeta);
 //NewParksMcClellan(FIRCoeff, NumTaps, PassType, OmegaC, BW, ParksWidth, wtNONE, WinBeta);

 if(FreqCorrection)
  {
   double OrigOmega = OmegaC;
   double OrigBW = BW;

   // This function corrects OmegaC for LPF and HPF. It corrects BW for BPF and Notch.
   FIRFreqError(FIRCoeff, NumTaps, PassType, &OmegaC, &BW);

   // Recalculate the filter with the corrected OmegaC and BW values.
   BasicFIR(FIRCoeff, NumTaps, PassType, OmegaC, BW, WindowType, WinBeta);
   //NewParksMcClellan(FIRCoeff, NumTaps, PassType, OmegaC, BW, ParksWidth, wtNONE, WinBeta);

   OmegaC = OrigOmega; // Restore these in case they are needed.
   BW = OrigBW;
  }

 // Calculate the frequency response of the filter with an FFT.
 for(k=0; k<NUM_PLOT_PTS; k++)RealHofZ[k] = ImagHofZ[k] = 0.0;             // Init the arrays
 for(k=0; k<NumTaps; k++)RealHofZ[k] = FIRCoeff[k] * (double)NUM_PLOT_PTS; // Need to do this scaling to account for the 1/N scaling done by the forward FFT.
 FFT(RealHofZ, ImagHofZ, NUM_PLOT_PTS, FORWARD);                           // The FFT's results are returned in input arrays, RealHofZ and ImagHofZ.

/*
 //This will print the FIR coefficients to a text file. Be sure to set the path before using.
 FILE *OutPutFile;
 OutPutFile = fopen("C:\\Documents and Settings\\Dan\\Desktop\\FIRCoeff.txt","w");
 for(int j=0; j<NumTaps; j++)
  {
   fprintf(OutPutFile,"\n %9.9f ", FIRCoeff[j]);
  }
 fclose(OutPutFile);
*/


}

//---------------------------------------------------------------------------

/*
 This code generates the impulse response for a rectangular low pass, high pass,
 band pass, or notch filter. Then a window, such as the Kaiser, is applied to it.

 double FirCoeff[MAXNUMTAPS];
 int NumTaps;                        NumTaps can be even or odd and < MAXNUMTAPS
 TPassTypeName PassType;             PassType is defined in the header file. firLPF, firHPF, firBPF, firNOTCH
 double OmegaC  0.0 < OmegaC < 1.0   The corner freq, or center freq if BPF or NOTCH
 double BW      0.0 < BW < 1.0       The band width if BPF or NOTCH
 TWindowType WindowType;             WindowType is defined in the header file.  wtNONE, wtKAISER, wtSINC, .... and others.
 double Beta;  0 <= Beta <= 10.0     Beta is used with the Kaiser, Sinc windows.
*/
void BasicFIR(double *FirCoeff, int NumTaps, TFIRPassTypes PassType, double OmegaC, double BW, TWindowType WindowType, double WinBeta)
{
 int j;
 double Arg, OmegaLow, OmegaHigh;

 switch(PassType)
  {
   case firLPF:
	for(j=0; j<NumTaps; j++)
     {
      Arg = (double)j - (double)(NumTaps-1) / 2.0;
      FirCoeff[j] = OmegaC * Sinc(OmegaC * Arg * M_PI);
     }
    break;

   case firHPF:
    if(NumTaps % 2 == 1) // Odd tap counts
     {
      for(j=0; j<NumTaps; j++)
       {
        Arg = (double)j - (double)(NumTaps-1) / 2.0;
        FirCoeff[j] = Sinc(Arg * M_PI) - OmegaC * Sinc(OmegaC * Arg * M_PI);
       }
     }

    else  // Even tap counts
      {
       for(j=0; j<NumTaps; j++)
        {
         Arg = (double)j - (double)(NumTaps-1) / 2.0;
         if(Arg == 0.0)FirCoeff[j] = 0.0;
         else FirCoeff[j] = cos(OmegaC * Arg * M_PI) / M_PI / Arg  + cos(Arg * M_PI);
        }
      }
   break;

   case firBPF:
    OmegaLow  = OmegaC - BW/2.0;
    OmegaHigh = OmegaC + BW/2.0;
	for(j=0; j<NumTaps; j++)
     {
      Arg = (double)j - (double)(NumTaps-1) / 2.0;
      if(Arg == 0.0)FirCoeff[j] = 0.0;
      else FirCoeff[j] =  ( cos(OmegaLow * Arg * M_PI) - cos(OmegaHigh * Arg * M_PI) ) / M_PI / Arg ;
     }
   break;

   case firNOTCH:  // If NumTaps is even for Notch filters, the response at Pi is attenuated.
    OmegaLow  = OmegaC - BW/2.0;
    OmegaHigh = OmegaC + BW/2.0;
	for(j=0; j<NumTaps; j++)
     {
      Arg = (double)j - (double)(NumTaps-1) / 2.0;
      FirCoeff[j] =  Sinc(Arg * M_PI) - OmegaHigh * Sinc(OmegaHigh * Arg * M_PI) - OmegaLow * Sinc(OmegaLow * Arg * M_PI);
     }
   break;
  }

 // WindowData is in FFTCode.cpp
 WindowData(FirCoeff, NumTaps, WindowType, 0.0, WinBeta, false);

}

//---------------------------------------------------------------------------

// This gets used in numerous places above.
double Sinc(double x)
{
 if(x > -1.0E-5 && x < 1.0E-5)return(1.0);
 return(sin(x)/x);
}

//---------------------------------------------------------------------------

// This implements an FIR filter. The register shifts are done by rotating the indexes.
void FilterWithFIR(double *FirCoeff, int NumTaps, double *Signal, double *FilteredSignal, int NumSigPts)
{
 int j, k, n, Top = 0;
 double y, Reg[MAX_NUMTAPS];

 for(j=0; j<NumTaps; j++)Reg[j] = 0.0;

 for(j=0; j<NumSigPts; j++)
  {
   Reg[Top] = Signal[j];
   y = 0.0;
   n = 0;

   // The FirCoeff index increases while the Reg index decreases.
   for(k=Top; k>=0; k--)
    {
     y += FirCoeff[n++] * Reg[k];
    }
   for(k=NumTaps-1; k>Top; k--)
    {
     y += FirCoeff[n++] * Reg[k];
    }
   FilteredSignal[j] = y;

   Top++;
   if(Top >= NumTaps)Top = 0;
  }

}

//---------------------------------------------------------------------------

// This code is equivalent to the code above. It uses register shifts, which makes it
// less efficient, but it is easier to follow.

void FilterWithFIR2(double *FirCoeff, int NumTaps, double *Signal, double *FilteredSignal, int NumSigPts)
{
 int j, k;
 double y, Reg[MAX_NUMTAPS];

 for(j=0; j<NumTaps; j++)Reg[j] = 0.0; // Init the delay registers.

 for(j=0; j<NumSigPts; j++)
 {
  // Shift the register values down and set Reg[0].
  for(k=NumTaps; k>1; k--)Reg[k-1] = Reg[k-2];
  Reg[0] = Signal[j];

  y = 0.0;
  for(k=0; k<NumTaps; k++)y += FirCoeff[k] * Reg[k];
  FilteredSignal[j] = y;
 }

}

//---------------------------------------------------------------------------

// We normally specify the 3 dB frequencies when specifing a filter. The Parks McClellan routine
// uses OmegaC and BW to set the 0 dB band edges, so its final OmegaC and BW values are not close to -3 dB.
// The Windowed filters are better for high tap counts, but for low tap counts, their 3 dB frequencies
// are also well off the mark.

// To use this function, first calculate a set of FIR coefficients, then pass them here, along with OmegaC
// and BW. This calculates a corrected OmegaC for low and high pass filters. It calcultes a corrected BW
// for band pass and notch filters. Use these corrected values to recalculate the FIR filter.

// The Goertzel algorithm is used here to calculate the filter's magnitude response at loop omgea.
// This function finds the 3 dB freq by starting in the pass band and working out. The loops
// stop looking (break) at the -20 dB frequency.

void FIRFreqError(double *Coeff, int NumTaps, int PassType, double *OmegaC, double *BW)
{
 int j, J3dB, CenterJ;
 double Omega, CorrectedOmega, CorrectedBW, Omega1, Omega2, Mag;

 // In these loops, we break well past the 3 dB pt so that large ripple is ignored.
 if(PassType == firLPF)
  {
   J3dB = 10;
   for(j=0; j<NUM_FREQ_ERR_PTS; j++)
	{
     Omega = (double)j / dNUM_FREQ_ERR_PTS;
     Mag = Goertzel(Coeff, NumTaps, Omega);
	 if(Mag > 0.707)J3dB = j;
	 if(Mag < 0.1)break;
	}
   Omega = (double)J3dB / dNUM_FREQ_ERR_PTS;
  }

 else if(PassType == firHPF)
  {
   J3dB = NUM_FREQ_ERR_PTS - 10;
   for(j=NUM_FREQ_ERR_PTS-1; j>=0; j--)
	{
     Omega = (double)j / dNUM_FREQ_ERR_PTS;
     Mag = Goertzel(Coeff, NumTaps, Omega);
	 if(Mag > 0.707)J3dB = j;
	 if(Mag < 0.1)break;
	}
   Omega = (double)J3dB / dNUM_FREQ_ERR_PTS;
  }

 else if(PassType == firBPF)
  {
   CenterJ = dNUM_FREQ_ERR_PTS * *OmegaC;
   J3dB = CenterJ;
   for(j=CenterJ; j>=0; j--)
	{
     Omega = (double)j / dNUM_FREQ_ERR_PTS;
     Mag = Goertzel(Coeff, NumTaps, Omega);
	 if(Mag > 0.707)J3dB = j;
	 if(Mag < 0.1)break;
	}
   Omega1 = (double)J3dB / dNUM_FREQ_ERR_PTS;

   J3dB = CenterJ;
   for(j=CenterJ; j<NUM_FREQ_ERR_PTS; j++)
	{
     Omega = (double)j / dNUM_FREQ_ERR_PTS;
     Mag = Goertzel(Coeff, NumTaps, Omega);
	 if(Mag > 0.707)J3dB = j;
	 if(Mag < 0.1)break;
	}
   Omega2 = (double)J3dB / dNUM_FREQ_ERR_PTS;
  }

 // The code above starts in the pass band. This starts in the stop band.
 else // PassType == firNOTCH
  {
   CenterJ = dNUM_FREQ_ERR_PTS * *OmegaC;
   J3dB = CenterJ;
   for(j=CenterJ; j>=0; j--)
	{
     Omega = (double)j / dNUM_FREQ_ERR_PTS;
     Mag = Goertzel(Coeff, NumTaps, Omega);
	 if(Mag <= 0.707)J3dB = j;
	 if(Mag > 0.99)break;
	}
   Omega1 = (double)J3dB/dNUM_FREQ_ERR_PTS;

   J3dB = CenterJ;
   for(j=CenterJ; j<NUM_FREQ_ERR_PTS; j++)
	{
     Omega = (double)j / dNUM_FREQ_ERR_PTS;
     Mag = Goertzel(Coeff, NumTaps, Omega);
	 if(Mag <= 0.707)J3dB = j;
	 if(Mag > 0.99)break;
	}
   Omega2 = (double)J3dB / dNUM_FREQ_ERR_PTS;
  }


 // This calculates the corrected OmegaC and BW and error checks the values.
 if(PassType == firLPF || PassType == firHPF )
  {
   CorrectedOmega = *OmegaC * 2.0 - Omega;  // This is usually OK.
   if(CorrectedOmega < 0.001)CorrectedOmega = 0.001;
   if(CorrectedOmega > 0.99)CorrectedOmega = 0.99;
   *OmegaC = CorrectedOmega;
  }

 else // PassType == firBPF || PassType == firNOTCH
  {
   CorrectedBW = *BW * 2.0 - (Omega2 - Omega1);  // This routinely goes neg with Notch.
   if(CorrectedBW < 0.01)CorrectedBW = 0.01;
   if(CorrectedBW > *BW * 2.0)CorrectedBW = *BW * 2.0;
   if(CorrectedBW > 0.98)CorrectedBW = 0.98;
   *BW = CorrectedBW;
  }

}

//-----------------------------------------------------------------------------

