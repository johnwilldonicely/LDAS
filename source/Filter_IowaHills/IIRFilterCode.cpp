
// By Iowa Hills Software, LLC  IowaHills.com
// If you find a problem, please leave a note at:
// http://www.iowahills.com/feedbackcomments.html
// December 3, 2014

// This code calculates the coefficients for IIR filters from a set of 2nd order s plane coefficients.
// The s plane coefficients for Butterworth, Chebyshev, Gauss, Bessel, Adj Gauss, Inverse Chebyshev,
// and Elliptic filters are tabulated in the FilterCoefficients.hpp file for up to 10 poles.
// The s plane filters were frequency scaled so their 3 dB frequency is at s = omega = 1.
// We also ordered the poles and zeros in the tables in a manner appropriate for IIR filters.
// For a derivation of the formulas used here, see the IIREquationDerivations.pdf

// In addition to calculating the IIR coefficients, this file also contains a Form 1 Biquad
// filter implementation and the code to calculate the frequency response of an IIR filter.

/*
Changes: 4 October 2015 (John Huxter)
	- replace ShowMessage function call with simple print to stderr
	- the above removes necessity for the vcl.h header, which is not included in the code archive
*/

#include "IIRFilterCode.h"
#include "QuadRootsCode.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



//---------------------------------------------------------------------------
// This shows some examples of how to use the functions in this file.
// These define's and include's are only needed for ExampleIIRCall.
#define NUM_PLOT_PTS 1024
#include <stdio.h>
#include "FFTCode.h"


void ExampleIIRCall(void)
{
 int NumPoles = 6;                // 2 - 10 for all filters except the Elliptics, which are 4 - 10
 double Ripple = 0.1;             // 0.0 - 1.0 dB for Chebyshev in increments of 0.10  0.0 - 0.2 for the Elliptics in increments 0f 0.02
 double StopBanddB = 60;          // 20 - 100 dB for the Inv Cheby  40 - 100 for the Elliptics
 double OmegaC = 0.2;             // 0.0 < OmegaC < 1.0
 double BW = 0.1;                 // 0.0 < BandWidth < 1.0
 double Gamma = 0.0;              // -1.0 <= Gamma <= 1.0  Gamma controls the transition BW on the Adjustable Gauss
 double RealHofZ[NUM_PLOT_PTS];   // Real and imag parts of H(z). Used with the function IIRFreqResponse.
 double ImagHofZ[NUM_PLOT_PTS];

 TIIRCoeff IIRCoeff;                    // These next 3 are defined in the header file.
 TIIRPassTypes PassType = iirLPF;       // iirLPF, iirHPF, iirBPF, iirNOTCH
 TIIRFilterTypes FiterType = ftELLIPTIC_00; // ftBUTTERWORTH, ftGAUSSIAN, ftBESSEL, ftADJGAUSS, ftCHEBYSHEV, ftINVERSE_CHEBY, ftELLIPTIC_00

 // If using an Elliptic, we must select the ELLIPTIC according to the Ripple value.
 // ftELLIPTIC_00 is the 1st ELLIPTIC in TIIRFilterTypes.
 if(FiterType >= ftELLIPTIC_00)
  {
   if(Ripple >= 0.19)       FiterType = ftELLIPTIC_20; // Ripple = 0.20 dB
   else if(Ripple >= 0.17)  FiterType = ftELLIPTIC_18; // Ripple = 0.18 dB
   else if(Ripple >= 0.15)  FiterType = ftELLIPTIC_16; // Ripple = 0.16 dB
   else if(Ripple >= 0.13)  FiterType = ftELLIPTIC_14; // Ripple = 0.14 dB
   else if(Ripple >= 0.11)  FiterType = ftELLIPTIC_12; // Ripple = 0.12 dB
   else if(Ripple >= 0.09)  FiterType = ftELLIPTIC_10; // Ripple = 0.10 dB
   else if(Ripple >= 0.07)  FiterType = ftELLIPTIC_08; // Ripple = 0.08 dB
   else if(Ripple >= 0.05)  FiterType = ftELLIPTIC_06; // Ripple = 0.06 dB
   else if(Ripple >= 0.03)  FiterType = ftELLIPTIC_04; // Ripple = 0.04 dB
   else if(Ripple >= 0.01)  FiterType = ftELLIPTIC_02; // Ripple = 0.02 dB
   else                     FiterType = ftELLIPTIC_00; // Ripple = 0.00 dB
  }

 // Calling CalcIIRFilterCoeff() fills the IIRCoeff struct.
 // Beta, the 2nd function argument, has a different meaning, depending on the filter type.
 // Beta is also range checked in GetSPlaneCoefficients()

 // Beta is ignored and set to 0
 if(FiterType == ftBUTTERWORTH || FiterType == ftBESSEL || FiterType == ftGAUSSIAN)
  {
   IIRCoeff = CalcIIRFilterCoeff(FiterType, 0, PassType, NumPoles, OmegaC, BW);
  }

 // Beta = Ripple in dB  >= 0
 if(FiterType == ftCHEBYSHEV)
  {
   if(Ripple < 0.0)Ripple = -Ripple;
   IIRCoeff = CalcIIRFilterCoeff(FiterType, Ripple, PassType, NumPoles, OmegaC, BW);
  }

 // Beta = StopBanddB >= 0
 if(FiterType == ftINVERSE_CHEBY || FiterType >= ftELLIPTIC_00)
  {
   if(StopBanddB < 0.0)StopBanddB = -StopBanddB;
   IIRCoeff = CalcIIRFilterCoeff(FiterType, StopBanddB, PassType, NumPoles, OmegaC, BW);
  }

 // Beta = Gamma, which adjusts the width of the transition band.  -1 <= Gamma <= 1
 if(FiterType == ftADJGAUSS)
  {
   if(Gamma < -1.0)Gamma = -1.0;
   if(Gamma > 1.0) Gamma = 1.0;
   IIRCoeff = CalcIIRFilterCoeff(FiterType, Gamma, PassType, NumPoles, OmegaC, BW);
  }

/*
 This will print the IIR coefficients to a text file. Be sure to set the path before using.
 FILE *OutPutFile;
 OutPutFile = fopen("C:\\Documents and Settings\\Dan\\Desktop\\IIRCoeff.txt","w");
 for(int j=0; j<IIRCoeff.NumSections; j++)
  {
   fprintf(OutPutFile,"\n Section %d", j);
   fprintf(OutPutFile,"\n a0= %9.9f  a1= %9.9f  a2= %9.9f", IIRCoeff.a0[j], IIRCoeff.a1[j], IIRCoeff.a2[j]);
   fprintf(OutPutFile,"\n b0= %9.9f  b1= %9.9f  b2= %9.9f ",IIRCoeff.b0[j], IIRCoeff.b1[j], IIRCoeff.b2[j]);
   fprintf(OutPutFile,"\n ");
  }
 fclose(OutPutFile);
*/

 // This calculates the frequency response of the filter by doing a DFT of the IIR coefficients.
 IIRFreqResponse(IIRCoeff, IIRCoeff.NumSections, RealHofZ, ImagHofZ, NUM_PLOT_PTS);

/*
 // This is an alternative way to calculate the filter's frequency response.
 // We send an impulse through the filter, and calc the FFT of the filters output.
 // Since the FFT scales the output of a forward transform by 1/N,
 // we use N = NUM_PLOT_PTS instead of 1 for the impulse.

 for(k=0; k<NUM_PLOT_PTS; k++)Samples[k] = RealHofZ[k] = ImagHofZ[k] = 0.0;
 Samples[0] = NUM_PLOT_PTS;                                 // The impulse.
 FilterWithIIR(IIRCoeff, Samples, RealHofZ, NUM_PLOT_PTS);  // Filter the impulse. RealHofZ is used to store the filtered output.
 FFT(RealHofZ, ImagHofZ, NUM_PLOT_PTS, FORWARD);            // The FFT's results are returned in input arrays, RealHofZ and ImagHofZ.
*/


}

//---------------------------------------------------------------------------
// H(s) = ( Ds^2 + Es + F ) / ( As^2 + Bs + C )
// H(z) = ( b2z^2 + b1z + b0 ) / ( a2z^2 + a1z + a0 )
TIIRCoeff CalcIIRFilterCoeff(TIIRFilterTypes ProtoType, double Beta, TIIRPassTypes PassType, int NumPoles, double OmegaC, double BW)
{
 int j, k;
 double Scalar;
 double Coeff[5], RealRoot[4], ImagRoot[4];
 double A, B, C, D, E, F, T, Q, Arg;
 double a2[ARRAY_DIM], a1[ARRAY_DIM], a0[ARRAY_DIM], b2[ARRAY_DIM], b1[ARRAY_DIM], b0[ARRAY_DIM];

 TSPlaneCoeff SPlaneCoeff;
 TIIRCoeff IIR; // This gets returned.

 // Init the IIR structure. a3, a4, b3,and b4 are only used to calculate the 2nd order
 // sections for the bandpass and notch filters. They get discarded.
 for(j=0; j<ARRAY_DIM; j++)
  {
   IIR.a0[j] = 0.0;  IIR.b0[j] = 0.0;
   IIR.a1[j] = 0.0;  IIR.b1[j] = 0.0;
   IIR.a2[j] = 0.0;  IIR.b2[j] = 0.0;
   IIR.a3[j] = 0.0;  IIR.b3[j] = 0.0;
   IIR.a4[j] = 0.0;  IIR.b4[j] = 0.0;
  }
 IIR.NumSections = 0;

 // This gets the 2nd order s plane filter coefficients from the tables in FilterCoefficients.hpp.
 // If you have code that computes these coefficients, then use that instead.
 SPlaneCoeff = GetSPlaneCoefficients(ProtoType, NumPoles, Beta);
 IIR.NumSections = SPlaneCoeff.NumSections;  // NumSections = (NumPoles + 1)/2

 // T sets the filter's corner frequency, or center freqency.
 // The Bilinear transform is defined as:  s = 2/T * tan(Omega/2) = 2/T * (1 - z)/(1 + z)
 T = 2.0 * tan(OmegaC * M_PI_2);
 Q = 1.0 + OmegaC;
 if(Q > 1.95)Q = 1.95;
 Q = 0.8 * tan(Q * M_PI_4); // This is a correction factor for Q. Q is used with band pass and notch flters.
 Q = OmegaC / BW / Q;       // This is the corrected Q.

 k = 0;
 for(j=0; j<SPlaneCoeff.NumSections; j++)
  {
   A = SPlaneCoeff.A[j];   // We use A - F to make the code easier to read.
   B = SPlaneCoeff.B[j];
   C = SPlaneCoeff.C[j];
   D = SPlaneCoeff.D[j];
   E = SPlaneCoeff.E[j];
   F = SPlaneCoeff.F[j];

  // b's are the numerator  a's are the denominator
  if(PassType == iirLPF)
   {
	if(A == 0.0 && D == 0.0) // 1 pole case
	 {
	  Arg = (2.0*B + C*T);
	  IIR.a2[j] = 0.0;
	  IIR.a1[j] = (-2.0*B + C*T) / Arg;
	  IIR.a0[j] = 1.0;     // The filter implementation depends on a0 = 1.

	  IIR.b2[j] = 0.0;
	  IIR.b1[j] = (-2.0*E + F*T) / Arg * C/F;
	  IIR.b0[j] = ( 2.0*E + F*T) / Arg * C/F;
	 }
	else // 2 poles
	 {
	  Arg = (4.0*A + 2.0*B*T + C*T*T);
	  IIR.a2[j] = (4.0*A - 2.0*B*T + C*T*T) / Arg;
	  IIR.a1[j] = (2.0*C*T*T - 8.0*A) / Arg;
	  IIR.a0[j] = 1.0;     // The filter implementation depends on a0 = 1.

	  // With all pole filters, our LPF numerator is (z+1)^2, so all our Z Plane zeros are at -1
	  IIR.b2[j] = (4.0*D - 2.0*E*T + F*T*T) / Arg * C/F;
	  IIR.b1[j] = (2.0*F*T*T - 8.0*D) / Arg * C/F;
	  IIR.b0[j] = (4*D + F*T*T + 2.0*E*T) / Arg * C/F;
	 }
   }

  if(PassType == iirHPF)
   {
	if(A == 0.0 && D == 0.0) // 1 pole
	 {
	  Arg = 2.0*C + B*T;
	  IIR.a2[j] = 0.0;
	  IIR.a1[j] = (B*T - 2.0*C) / Arg;
	  IIR.a0[j] = 1.0;     // The filter implementation depends on a0 = 1.

	  IIR.b2[j] = 0.0;
	  IIR.b1[j] = (E*T - 2.0*F) / Arg * C/F;
	  IIR.b0[j] = (E*T + 2.0*F) / Arg * C/F;
	 }
	else  // 2 poles
	 {
	  Arg = A*T*T + 4.0*C + 2.0*B*T;
	  IIR.a2[j] = (A*T*T + 4.0*C - 2.0*B*T) / Arg;
	  IIR.a1[j] = (2.0*A*T*T - 8.0*C) / Arg;
	  IIR.a0[j] = 1.0;

	  // With all pole filters, our HPF numerator is (z-1)^2, so all our Z Plane zeros are at 1
	  IIR.b2[j] = (D*T*T - 2.0*E*T + 4.0*F) / Arg * C/F;
	  IIR.b1[j] = (2.0*D*T*T - 8.0*F) / Arg * C/F;
	  IIR.b0[j] = (D*T*T + 4.0*F + 2.0*E*T) / Arg * C/F;
	 }
   }

  if(PassType == iirBPF)
   {
	if(A == 0.0 && D == 0.0) // 1 pole
	 {
	  Arg = 4.0*B*Q + 2.0*C*T + B*Q*T*T;
	  a2[k] = (B*Q*T*T + 4.0*B*Q - 2.0*C*T) / Arg;
	  a1[k] = (2.0*B*Q*T*T - 8.0*B*Q) / Arg ;
	  a0[k] = 1.0;

	  b2[k] = (E*Q*T*T + 4.0*E*Q - 2.0*F*T) / Arg * C/F;
	  b1[k] = (2.0*E*Q*T*T - 8.0*E*Q) / Arg * C/F;
	  b0[k] = (4.0*E*Q + 2.0*F*T + E*Q*T*T) / Arg * C/F;
	  k++;
	 }
	else //2 Poles
	 {
	  IIR.a4[j] = (16.0*A*Q*Q + A*Q*Q*T*T*T*T + 8.0*A*Q*Q*T*T - 2.0*B*Q*T*T*T - 8.0*B*Q*T + 4.0*C*T*T) * F;
	  IIR.a3[j] = (4.0*T*T*T*T*A*Q*Q - 4.0*Q*T*T*T*B + 16.0*Q*B*T - 64.0*A*Q*Q) * F;
	  IIR.a2[j] = (96.0*A*Q*Q - 16.0*A*Q*Q*T*T + 6.0*A*Q*Q*T*T*T*T - 8.0*C*T*T) * F;
	  IIR.a1[j] = (4.0*T*T*T*T*A*Q*Q + 4.0*Q*T*T*T*B - 16.0*Q*B*T - 64.0*A*Q*Q) * F;
	  IIR.a0[j] = (16.0*A*Q*Q + A*Q*Q*T*T*T*T + 8.0*A*Q*Q*T*T + 2.0*B*Q*T*T*T + 8.0*B*Q*T + 4.0*C*T*T) * F;

	  // With all pole filters, our BPF numerator is (z-1)^2 * (z+1)^2 so the zeros come back as +/- 1 pairs
	  IIR.b4[j] = (8.0*D*Q*Q*T*T - 8.0*E*Q*T + 16.0*D*Q*Q - 2.0*E*Q*T*T*T + D*Q*Q*T*T*T*T + 4.0*F*T*T) * C;
	  IIR.b3[j] = (16.0*E*Q*T - 4.0*E*Q*T*T*T - 64.0*D*Q*Q + 4.0*D*Q*Q*T*T*T*T) * C;
	  IIR.b2[j] = (96.0*D*Q*Q - 8.0*F*T*T + 6.0*D*Q*Q*T*T*T*T - 16.0*D*Q*Q*T*T) * C;
	  IIR.b1[j] = (4.0*D*Q*Q*T*T*T*T - 64.0*D*Q*Q + 4.0*E*Q*T*T*T - 16.0*E*Q*T) * C;
	  IIR.b0[j] = (16.0*D*Q*Q + 8.0*E*Q*T + 8.0*D*Q*Q*T*T + 2.0*E*Q*T*T*T + 4.0*F*T*T + D*Q*Q*T*T*T*T) * C;

      // T = 2 makes these values approach 0.0 (~ 1.0E-12) The root solver needs 0.0 for numerical reasons.
      if(fabs(T-2.0) < 0.0005)
       {
        IIR.a3[j] = 0.0;
        IIR.a1[j] = 0.0;
        IIR.b3[j] = 0.0;
        IIR.b1[j] = 0.0;
       }

      // We now have a 4th order poly in the form a4*s^4 + a3*s^3 + a2*s^2 + a2*s + a0
	  // We find the roots of this so we can form two 2nd order polys.
	  Coeff[0] = IIR.a4[j];
	  Coeff[1] = IIR.a3[j];
	  Coeff[2] = IIR.a2[j];
	  Coeff[3] = IIR.a1[j];
	  Coeff[4] = IIR.a0[j];
      QuadCubicRoots(4, Coeff, RealRoot, ImagRoot);

	  // In effect, the root finder scales the poly by 1/a4 so we have to apply
	  // this factor back into the two 2nd order equations we are forming.
	  if(IIR.a4[j] < 0.0)Scalar = -sqrt(-IIR.a4[j]);
	  else               Scalar = sqrt(IIR.a4[j]);

      // Form the 2nd order polys from the roots.
	  a2[k] = Scalar;
	  a1[k] = -(RealRoot[0] + RealRoot[1]) * Scalar;
	  a0[k] =  (RealRoot[0] * RealRoot[1] - ImagRoot[0] * ImagRoot[1]) * Scalar;
	  k++;
	  a2[k] = Scalar;
	  a1[k] = -(RealRoot[2] + RealRoot[3]) * Scalar;
	  a0[k] =  (RealRoot[2] * RealRoot[3] - ImagRoot[2] * ImagRoot[3]) * Scalar;
	  k--;

	  // Now do the same with the numerator.
	  Coeff[0] = IIR.b4[j];
	  Coeff[1] = IIR.b3[j];
	  Coeff[2] = IIR.b2[j];
	  Coeff[3] = IIR.b1[j];
	  Coeff[4] = IIR.b0[j];
	  QuadCubicRoots(4, Coeff, RealRoot, ImagRoot);

	  if(IIR.b4[j] < 0.0)Scalar = -sqrt(-IIR.b4[j]);
	  else               Scalar = sqrt(IIR.b4[j]);

	  b2[k] = Scalar;
	  if(ProtoType == ftINVERSE_CHEBY || ProtoType >= ftELLIPTIC_00)
	   b1[k] = -(RealRoot[0] + RealRoot[1]) * Scalar;
	  else  // else the prototype is an all pole filter
	   b1[k] = 0.0;  // b1 = 0 for all pole filters, but the addition above won't always equal zero exactly.
	  b0[k] =  (RealRoot[0] * RealRoot[1] - ImagRoot[0] * ImagRoot[1]) * Scalar;
	  k++;

	  b2[k] = Scalar;
	  if(ProtoType == ftINVERSE_CHEBY || ProtoType >= ftELLIPTIC_00)
	   b1[k] = -(RealRoot[2] + RealRoot[3]) * Scalar;
	  else
	   b1[k] = 0.0;
	  b0[k] =  (RealRoot[2] * RealRoot[3] - ImagRoot[2] * ImagRoot[3]) * Scalar;
	  k++;
	  // Go below to see where we store these 2nd order polys back into IIR
	 }
   }

  if(PassType == iirNOTCH)
   {
	if(A == 0.0 && D == 0.0) // 1 pole
	 {
	  Arg = 2.0*B*T + C*Q*T*T + 4.0*C*Q;
	  a2[k] = (4.0*C*Q - 2.0*B*T + C*Q*T*T) / Arg;
	  a1[k] = (2.0*C*Q*T*T - 8.0*C*Q) / Arg;
	  a0[k] = 1.0;

	  b2[k] = (4.0*F*Q - 2.0*E*T + F*Q*T*T) / Arg * C/F;
	  b1[k] = (2.0*F*Q*T*T - 8.0*F*Q) / Arg *C/F;
	  b0[k] = (2.0*E*T + F*Q*T*T +4.0*F*Q) / Arg *C/F;
	  k++;
	 }
	else
	 {
	  IIR.a4[j] = (4.0*A*T*T - 2.0*B*T*T*T*Q + 8.0*C*Q*Q*T*T - 8.0*B*T*Q + C*Q*Q*T*T*T*T + 16.0*C*Q*Q) * -F;
	  IIR.a3[j] = (16.0*B*T*Q + 4.0*C*Q*Q*T*T*T*T - 64.0*C*Q*Q - 4.0*B*T*T*T*Q) * -F;
	  IIR.a2[j] = (96.0*C*Q*Q - 8.0*A*T*T - 16.0*C*Q*Q*T*T + 6.0*C*Q*Q*T*T*T*T) * -F;
	  IIR.a1[j] = (4.0*B*T*T*T*Q - 16.0*B*T*Q - 64.0*C*Q*Q + 4.0*C*Q*Q*T*T*T*T) * -F;
	  IIR.a0[j] = (4.0*A*T*T + 2.0*B*T*T*T*Q + 8.0*C*Q*Q*T*T + 8.0*B*T*Q + C*Q*Q*T*T*T*T + 16.0*C*Q*Q) * -F;

	  // Our Notch Numerator isn't simple. [ (4+T^2)*z^2 - 2*(4-T^2)*z + (4+T^2) ]^2
	  IIR.b4[j] = (2.0*E*T*T*T*Q - 4.0*D*T*T - 8.0*F*Q*Q*T*T + 8.0*E*T*Q - 16.0*F*Q*Q - F*Q*Q*T*T*T*T) * C;
	  IIR.b3[j] = (64.0*F*Q*Q + 4.0*E*T*T*T*Q - 16.0*E*T*Q - 4.0*F*Q*Q*T*T*T*T) * C;
	  IIR.b2[j] = (8.0*D*T*T - 96.0*F*Q*Q + 16.0*F*Q*Q*T*T - 6.0*F*Q*Q*T*T*T*T) * C;
	  IIR.b1[j] = (16.0*E*T*Q - 4.0*E*T*T*T*Q + 64.0*F*Q*Q - 4.0*F*Q*Q*T*T*T*T) * C;
	  IIR.b0[j] = (-4.0*D*T*T - 2.0*E*T*T*T*Q - 8.0*E*T*Q - 8.0*F*Q*Q*T*T - F*Q*Q*T*T*T*T - 16.0*F*Q*Q) * C;

      // T = 2 makes these values approach 0.0 (~ 1.0E-12) The root solver needs 0.0 for numerical reasons.
      if(fabs(T-2.0) < 0.0005)
       {
        IIR.a3[j] = 0.0;
        IIR.a1[j] = 0.0;
        IIR.b3[j] = 0.0;
        IIR.b1[j] = 0.0;
       }

	  // We now have a 4th order poly in the form a4*s^4 + a3*s^3 + a2*s^2 + a2*s + a0
	  // We find the roots of this so we can form two 2nd order polys.
	  Coeff[0] = IIR.a4[j];
	  Coeff[1] = IIR.a3[j];
	  Coeff[2] = IIR.a2[j];
	  Coeff[3] = IIR.a1[j];
	  Coeff[4] = IIR.a0[j];
	  QuadCubicRoots(4, Coeff, RealRoot, ImagRoot);

      // In effect, the root finder scales the poly by 1/a4 so we have to apply
	  // this factor back into the two 2nd order equations we are forming.
	  if(IIR.a4[j] < 0.0)Scalar = -sqrt(-IIR.a4[j]);
	  else               Scalar = sqrt(IIR.a4[j]);
	  a2[k] = Scalar;
	  a1[k] = -(RealRoot[0] + RealRoot[1]) * Scalar;
	  a0[k] =  (RealRoot[0] * RealRoot[1] - ImagRoot[0] * ImagRoot[1]) * Scalar;
	  k++;
	  a2[k] = Scalar;
	  a1[k] = -(RealRoot[2] + RealRoot[3]) * Scalar;
	  a0[k] =  (RealRoot[2] * RealRoot[3] - ImagRoot[2] * ImagRoot[3]) * Scalar;
	  k--;

	  // Now do the same with the numerator.
	  Coeff[0] = IIR.b4[j];
	  Coeff[1] = IIR.b3[j];
	  Coeff[2] = IIR.b2[j];
	  Coeff[3] = IIR.b1[j];
	  Coeff[4] = IIR.b0[j];
	  QuadCubicRoots(4, Coeff, RealRoot, ImagRoot);

	  if(IIR.b4[j] < 0.0)Scalar = -sqrt(-IIR.b4[j]);
	  else               Scalar = sqrt(IIR.b4[j]);

	  b2[k] = Scalar;
	  b1[k] = -(RealRoot[0] + RealRoot[1]) * Scalar;
	  b0[k] =  (RealRoot[0] * RealRoot[1] - ImagRoot[0] * ImagRoot[1]) * Scalar;
	  k++;
	  b2[k] = Scalar;
	  b1[k] = -(RealRoot[2] + RealRoot[3]) * Scalar;
	  b0[k] =  (RealRoot[2] * RealRoot[3] - ImagRoot[2] * ImagRoot[3]) * Scalar;
	  k++;
	 }
   }
 }

 if(PassType == iirBPF || PassType == iirNOTCH)
  {
   // In the calcs above for the BPF and Notch, we didn't set a0=1, so we do it here.
   // k will equal the number of poles.
   for(j=0; j<k; j++)
	{
	 b2[j] /= a0[j];
	 b1[j] /= a0[j];
	 b0[j] /= a0[j];
	 a2[j] /= a0[j];
	 a1[j] /= a0[j];
	 a0[j] = 1.0;
	}

   for(j=0; j<k; j++)
	{
	 IIR.a0[j] = a0[j];
	 IIR.a1[j] = a1[j];
	 IIR.a2[j] = a2[j];
	 IIR.b0[j] = b0[j];
	 IIR.b1[j] = b1[j];
	 IIR.b2[j] = b2[j];
	}
  }

 return(IIR);  // IIR is the structure containing the coefficients.
}

//---------------------------------------------------------------------------


TSPlaneCoeff GetSPlaneCoefficients(TIIRFilterTypes FilterType, int NumPoles, double Beta)
{

 #include "FilterCoefficients.hpp" // This file contains all the data arrays used below.

 int j, n, p, z, MinNumPoles, MaxNumPoles;
 int ArrayNumber, OuterArrayDim, NumSections;
 double LoopBeta, BetaMin, BetaStep, BetaMax;
 TSPlaneCoeff SPlaneCoeff;

 BetaMin =  BetaMinArray[FilterType];
 BetaStep = BetaStepArray[FilterType];
 BetaMax =  BetaMaxArray[FilterType];
 MinNumPoles = MinNumPolesArray[FilterType];
 MaxNumPoles = MaxNumPolesArray[FilterType];

 // Range check
 if(NumPoles < MinNumPoles)NumPoles = MinNumPoles;
 if(NumPoles > MaxNumPoles)NumPoles = MaxNumPoles;
 if(Beta < BetaMin)Beta = BetaMin;
 if(Beta > BetaMax)Beta = BetaMax;

 // Need the array number that corresponds to Beta.
 // The max array size was originally determined with this piece of code,
 // so we use it here so we can range check ArrayNumber.
 OuterArrayDim = 0;
 for(LoopBeta=BetaMin; LoopBeta<=BetaMax; LoopBeta+=BetaStep)OuterArrayDim++;

 ArrayNumber = 0;
 for(LoopBeta=BetaMin; LoopBeta<=BetaMax; LoopBeta+=BetaStep)
  {
   if( LoopBeta >= Beta - BetaStep/1.9999 &&  LoopBeta <= Beta + BetaStep/1.9999)break;
   ArrayNumber++;
  }
 if(ArrayNumber > OuterArrayDim-1)ArrayNumber = OuterArrayDim-1; // i.e. If the array is dimensioned to N, the highest we can access is N-1

 // n is the array location for the given pole count.
 // The arrays start at 0. MinNumPoles is either 2 or 4.
 n = NumPoles - MinNumPoles;
 if(n < 0)n = 0;

 // NumSections is the number of biquad sections for a given pole count.
 NumSections = (NumPoles + 1) / 2;
 p = z = 0;

 switch(FilterType)
  {
   case ftBUTTERWORTH:
   for(j=0; j<NumSections; j++)
    {
     SPlaneCoeff.A[j] = ButterworthDenominator[n][p++];
     SPlaneCoeff.B[j] = ButterworthDenominator[n][p++];
     SPlaneCoeff.C[j] = ButterworthDenominator[n][p++];
     SPlaneCoeff.D[j] = 0.0;
     SPlaneCoeff.E[j] = 0.0;
     SPlaneCoeff.F[j] = 1.0;
    }
   break;

   case ftGAUSSIAN:
   for(j=0; j<NumSections; j++)
    {
     SPlaneCoeff.A[j] = GaussDenominator[n][p++];
     SPlaneCoeff.B[j] = GaussDenominator[n][p++];
     SPlaneCoeff.C[j] = GaussDenominator[n][p++];
     SPlaneCoeff.D[j] = 0.0;
     SPlaneCoeff.E[j] = 0.0;
     SPlaneCoeff.F[j] = 1.0;
    }
   break;

   case ftBESSEL:
   for(j=0; j<NumSections; j++)
    {
     SPlaneCoeff.A[j] = BesselDenominator[n][p++];
     SPlaneCoeff.B[j] = BesselDenominator[n][p++];
     SPlaneCoeff.C[j] = BesselDenominator[n][p++];
     SPlaneCoeff.D[j] = 0.0;
     SPlaneCoeff.E[j] = 0.0;
     SPlaneCoeff.F[j] = 1.0;
    }
   break;

   case ftADJGAUSS:
   for(j=0; j<NumSections; j++)
    {
     SPlaneCoeff.A[j] = AdjGaussDenominator[ArrayNumber][n][p++];
     SPlaneCoeff.B[j] = AdjGaussDenominator[ArrayNumber][n][p++];
     SPlaneCoeff.C[j] = AdjGaussDenominator[ArrayNumber][n][p++];
     SPlaneCoeff.D[j] = 0.0;
     SPlaneCoeff.E[j] = 0.0;
     SPlaneCoeff.F[j] = 1.0;
    }
   break;

   case ftCHEBYSHEV:
   for(j=0; j<NumSections; j++)
    {
     SPlaneCoeff.A[j] = ChebyshevDenominator[ArrayNumber][n][p++];
     SPlaneCoeff.B[j] = ChebyshevDenominator[ArrayNumber][n][p++];
     SPlaneCoeff.C[j] = ChebyshevDenominator[ArrayNumber][n][p++];
     SPlaneCoeff.D[j] = 0.0;
     SPlaneCoeff.E[j] = 0.0;
     SPlaneCoeff.F[j] = 1.0;
    }
   break;

   case ftINVERSE_CHEBY:
   for(j=0; j<NumSections; j++)
    {
     SPlaneCoeff.A[j] = InvChebyDenominator[ArrayNumber][n][p++];
     SPlaneCoeff.B[j] = InvChebyDenominator[ArrayNumber][n][p++];
     SPlaneCoeff.C[j] = InvChebyDenominator[ArrayNumber][n][p++];
     SPlaneCoeff.D[j] = InvChebyNumerator[ArrayNumber][n][z++];
     SPlaneCoeff.E[j] = InvChebyNumerator[ArrayNumber][n][z++];
     SPlaneCoeff.F[j] = InvChebyNumerator[ArrayNumber][n][z++];
    }
   break;

   case ftELLIPTIC_00:
   for(j=0; j<NumSections; j++)
    {
     SPlaneCoeff.A[j] = Elliptic_00Denominator[ArrayNumber][n][p++];
     SPlaneCoeff.B[j] = Elliptic_00Denominator[ArrayNumber][n][p++];
     SPlaneCoeff.C[j] = Elliptic_00Denominator[ArrayNumber][n][p++];
     SPlaneCoeff.D[j] = Elliptic_00Numerator[ArrayNumber][n][z++];
     SPlaneCoeff.E[j] = Elliptic_00Numerator[ArrayNumber][n][z++];
     SPlaneCoeff.F[j] = Elliptic_00Numerator[ArrayNumber][n][z++];
    }
   break;

   case ftELLIPTIC_02:
   for(j=0; j<NumSections; j++)
    {
     SPlaneCoeff.A[j] = Elliptic_02Denominator[ArrayNumber][n][p++];
     SPlaneCoeff.B[j] = Elliptic_02Denominator[ArrayNumber][n][p++];
     SPlaneCoeff.C[j] = Elliptic_02Denominator[ArrayNumber][n][p++];
     SPlaneCoeff.D[j] = Elliptic_02Numerator[ArrayNumber][n][z++];
     SPlaneCoeff.E[j] = Elliptic_02Numerator[ArrayNumber][n][z++];
     SPlaneCoeff.F[j] = Elliptic_02Numerator[ArrayNumber][n][z++];
    }
   break;

   case ftELLIPTIC_04:
   for(j=0; j<NumSections; j++)
    {
     SPlaneCoeff.A[j] = Elliptic_04Denominator[ArrayNumber][n][p++];
     SPlaneCoeff.B[j] = Elliptic_04Denominator[ArrayNumber][n][p++];
     SPlaneCoeff.C[j] = Elliptic_04Denominator[ArrayNumber][n][p++];
     SPlaneCoeff.D[j] = Elliptic_04Numerator[ArrayNumber][n][z++];
     SPlaneCoeff.E[j] = Elliptic_04Numerator[ArrayNumber][n][z++];
     SPlaneCoeff.F[j] = Elliptic_04Numerator[ArrayNumber][n][z++];
    }
   break;

   case ftELLIPTIC_06:
   for(j=0; j<NumSections; j++)
    {
     SPlaneCoeff.A[j] = Elliptic_06Denominator[ArrayNumber][n][p++];
     SPlaneCoeff.B[j] = Elliptic_06Denominator[ArrayNumber][n][p++];
     SPlaneCoeff.C[j] = Elliptic_06Denominator[ArrayNumber][n][p++];
     SPlaneCoeff.D[j] = Elliptic_06Numerator[ArrayNumber][n][z++];
     SPlaneCoeff.E[j] = Elliptic_06Numerator[ArrayNumber][n][z++];
     SPlaneCoeff.F[j] = Elliptic_06Numerator[ArrayNumber][n][z++];
    }
   break;

   case ftELLIPTIC_08:
   for(j=0; j<NumSections; j++)
    {
     SPlaneCoeff.A[j] = Elliptic_08Denominator[ArrayNumber][n][p++];
     SPlaneCoeff.B[j] = Elliptic_08Denominator[ArrayNumber][n][p++];
     SPlaneCoeff.C[j] = Elliptic_08Denominator[ArrayNumber][n][p++];
     SPlaneCoeff.D[j] = Elliptic_08Numerator[ArrayNumber][n][z++];
     SPlaneCoeff.E[j] = Elliptic_08Numerator[ArrayNumber][n][z++];
     SPlaneCoeff.F[j] = Elliptic_08Numerator[ArrayNumber][n][z++];
    }
   break;

   case ftELLIPTIC_10:
   for(j=0; j<NumSections; j++)
    {
     SPlaneCoeff.A[j] = Elliptic_10Denominator[ArrayNumber][n][p++];
     SPlaneCoeff.B[j] = Elliptic_10Denominator[ArrayNumber][n][p++];
     SPlaneCoeff.C[j] = Elliptic_10Denominator[ArrayNumber][n][p++];
     SPlaneCoeff.D[j] = Elliptic_10Numerator[ArrayNumber][n][z++];
     SPlaneCoeff.E[j] = Elliptic_10Numerator[ArrayNumber][n][z++];
     SPlaneCoeff.F[j] = Elliptic_10Numerator[ArrayNumber][n][z++];
    }
   break;

   case ftELLIPTIC_12:
   for(j=0; j<NumSections; j++)
    {
     SPlaneCoeff.A[j] = Elliptic_12Denominator[ArrayNumber][n][p++];
     SPlaneCoeff.B[j] = Elliptic_12Denominator[ArrayNumber][n][p++];
     SPlaneCoeff.C[j] = Elliptic_12Denominator[ArrayNumber][n][p++];
     SPlaneCoeff.D[j] = Elliptic_12Numerator[ArrayNumber][n][z++];
     SPlaneCoeff.E[j] = Elliptic_12Numerator[ArrayNumber][n][z++];
     SPlaneCoeff.F[j] = Elliptic_12Numerator[ArrayNumber][n][z++];
    }
   break;

   case ftELLIPTIC_14:
   for(j=0; j<NumSections; j++)
    {
     SPlaneCoeff.A[j] = Elliptic_14Denominator[ArrayNumber][n][p++];
     SPlaneCoeff.B[j] = Elliptic_14Denominator[ArrayNumber][n][p++];
     SPlaneCoeff.C[j] = Elliptic_14Denominator[ArrayNumber][n][p++];
     SPlaneCoeff.D[j] = Elliptic_14Numerator[ArrayNumber][n][z++];
     SPlaneCoeff.E[j] = Elliptic_14Numerator[ArrayNumber][n][z++];
     SPlaneCoeff.F[j] = Elliptic_14Numerator[ArrayNumber][n][z++];
    }
   break;

   case ftELLIPTIC_16:
   for(j=0; j<NumSections; j++)
    {
     SPlaneCoeff.A[j] = Elliptic_16Denominator[ArrayNumber][n][p++];
     SPlaneCoeff.B[j] = Elliptic_16Denominator[ArrayNumber][n][p++];
     SPlaneCoeff.C[j] = Elliptic_16Denominator[ArrayNumber][n][p++];
     SPlaneCoeff.D[j] = Elliptic_16Numerator[ArrayNumber][n][z++];
     SPlaneCoeff.E[j] = Elliptic_16Numerator[ArrayNumber][n][z++];
     SPlaneCoeff.F[j] = Elliptic_16Numerator[ArrayNumber][n][z++];
    }
   break;

   case ftELLIPTIC_18:
   for(j=0; j<NumSections; j++)
    {
     SPlaneCoeff.A[j] = Elliptic_18Denominator[ArrayNumber][n][p++];
     SPlaneCoeff.B[j] = Elliptic_18Denominator[ArrayNumber][n][p++];
     SPlaneCoeff.C[j] = Elliptic_18Denominator[ArrayNumber][n][p++];
     SPlaneCoeff.D[j] = Elliptic_18Numerator[ArrayNumber][n][z++];
     SPlaneCoeff.E[j] = Elliptic_18Numerator[ArrayNumber][n][z++];
     SPlaneCoeff.F[j] = Elliptic_18Numerator[ArrayNumber][n][z++];
    }
   break;

   case ftELLIPTIC_20:
   for(j=0; j<NumSections; j++)
    {
     SPlaneCoeff.A[j] = Elliptic_20Denominator[ArrayNumber][n][p++];
     SPlaneCoeff.B[j] = Elliptic_20Denominator[ArrayNumber][n][p++];
     SPlaneCoeff.C[j] = Elliptic_20Denominator[ArrayNumber][n][p++];
     SPlaneCoeff.D[j] = Elliptic_20Numerator[ArrayNumber][n][z++];
     SPlaneCoeff.E[j] = Elliptic_20Numerator[ArrayNumber][n][z++];
     SPlaneCoeff.F[j] = Elliptic_20Numerator[ArrayNumber][n][z++];
    }
   break;

 }

 SPlaneCoeff.NumSections = NumSections;
 return(SPlaneCoeff);

}

//---------------------------------------------------------------------------

// This code implements an IIR filter as a Form 1 Biquad.
// It uses 2 sets of shift registers, RegX on the input side and RegY on the output side.
// There are many ways to implement an IIR filter, some very good, and some extremely bad.
// For numerical reasons, a Form1 Biquad implementation is among the best.
void FilterWithIIR(TIIRCoeff IIRCoeff, double *Signal, double *FilteredSignal, int NumSigPts)
{
 double y;
 int j, k;

 for(j=0; j<NumSigPts; j++)
  {
   k = 0;
   y = SectCalc(j, k, Signal[j], IIRCoeff);
   for(k=1; k<IIRCoeff.NumSections; k++)
    {
     y = SectCalc(j, k, y, IIRCoeff);
    }
   FilteredSignal[j] = y;
  }

}
//---------------------------------------------------------------------------

// This gets used with the function above, FilterWithIIR()
double SectCalc(int j, int k, double x, TIIRCoeff IIRCoeff)
{
 double y, CenterTap;
 static double RegX1[ARRAY_DIM], RegX2[ARRAY_DIM], RegY1[ARRAY_DIM], RegY2[ARRAY_DIM], MaxRegVal;
 static bool MessageShown = false;

 // Init the vars on the 1st call or on an overflow condition. The overflow limit used
 // here is small for double variables, but a filter that reaches this threshold is broken.
 if( (j == 0 && k == 0) || MaxRegVal > OVERFLOW_LIMIT)
  {
   if(MaxRegVal > OVERFLOW_LIMIT && !MessageShown)
    {
     fprintf(stderr,"ERROR: Math Over Flow in IIR Section Calc. \nThe register values exceeded 1.0E20 \n");
     MessageShown = true; // So this message doesn't get shown thousands of times.
    }

   MaxRegVal = 1.0E-12;
   for(int i=0; i<ARRAY_DIM; i++)
    {
     RegX1[i] = 0.0;
     RegX2[i] = 0.0;
     RegY1[i] = 0.0;
     RegY2[i] = 0.0;
    }
  }

 CenterTap = x * IIRCoeff.b0[k] + IIRCoeff.b1[k] * RegX1[k] + IIRCoeff.b2[k] * RegX2[k];
 y = IIRCoeff.a0[k] * CenterTap - IIRCoeff.a1[k] * RegY1[k] - IIRCoeff.a2[k] * RegY2[k];

 RegX2[k] = RegX1[k];
 RegX1[k] = x;
 RegY2[k] = RegY1[k];
 RegY1[k] = y;

 // MaxRegVal is used above to prevent overflow.  Overflow seldom occurs, but will
 // if the filter has faulty coefficients. MaxRegVal is usually less than 100.0
 if( fabs(CenterTap)  > MaxRegVal ) MaxRegVal = fabs(CenterTap);
 if( fabs(y)  > MaxRegVal ) MaxRegVal = fabs(y);
 return(y);
}
//---------------------------------------------------------------------------

// This function calculates the frequency response of an IIR filter.
// Probably the easiest way to determine the frequency response of an IIR filter is to send
// an impulse through the filter and do an FFT on the output. This method does a DFT on
// the coefficients of each biquad section. The results from the cascaded sections are
// then multiplied together. A clearer example, which uses complex variables, is given just below.

// This approach works better than an FFT when the filter is very narrow. To analyze highly selective
// filters with an FFT can require a very large number of points, which can be quite cumbersome.
// This approach allows you to set the range of frequencies to be analyzed by modifying the statement
// Arg = M_PI * (double)j / (double)NumPts; .
void IIRFreqResponse(TIIRCoeff IIRCoeff, int NumSections, double *RealHofZ, double *ImagHofZ, int NumPts)
{
 int j, n;
 double Arg, MagSq, RealDFT, ImagDFT, TempR,  TempI;
 double RealZ, ImagZ, RealZ2, ImagZ2;
 double RealNumerator, ImagNumerator, RealDenominator, ImagDenominator; // Real and Imag parts of the numerator and denominator.

 for(j=0; j<NumPts; j++)
  {
   // We evaluate from 0 to Pi. i.e. The positive frequencies.
   Arg = M_PI * (double)j / (double)NumPts;
   RealZ = cos(Arg);
   ImagZ = -sin(Arg);
   RealZ2 = RealZ * RealZ - ImagZ * ImagZ;  // Z2 is Z^2
   ImagZ2 = RealZ * ImagZ + RealZ * ImagZ;

   RealNumerator = RealDenominator = 1.0;
   ImagNumerator = ImagDenominator = 0.0;
   for(n=0; n<NumSections; n++)
	{
     // These 1st 2 lines are a DFT of the numerator coeff.
     RealDFT = IIRCoeff.b0[n] + IIRCoeff.b1[n] * RealZ + IIRCoeff.b2[n] * RealZ2;
     ImagDFT = IIRCoeff.b1[n] * ImagZ + IIRCoeff.b2[n] * ImagZ2;
     TempR = RealNumerator;
     TempI = ImagNumerator;
     RealNumerator = RealDFT * TempR - ImagDFT * TempI;  // Num *= the DFT
     ImagNumerator = RealDFT * TempI + ImagDFT * TempR;

     // These next 2 lines are a DFT of the denominator coeff.
     RealDFT = IIRCoeff.a0[n] + IIRCoeff.a1[n] * RealZ + IIRCoeff.a2[n] * RealZ2;
     ImagDFT = IIRCoeff.a1[n] * ImagZ + IIRCoeff.a2[n] * ImagZ2;
     TempR = RealDenominator;
     TempI = ImagDenominator;
     RealDenominator = RealDFT * TempR - ImagDFT * TempI; // Denom *= the DFT
     ImagDenominator = RealDFT * TempI + ImagDFT * TempR;
    }

   // HofZ[j] =  Numerator / Denominator;
   MagSq = RealDenominator * RealDenominator + ImagDenominator * ImagDenominator; // Magnitude Squared
   if(MagSq > 0.0)
    {
     RealHofZ[j] = (RealNumerator * RealDenominator + ImagNumerator * ImagDenominator) / MagSq;
     ImagHofZ[j] = (ImagNumerator * RealDenominator - RealNumerator * ImagDenominator) / MagSq;
     if( (RealHofZ[j] * RealHofZ[j] + ImagHofZ[j] * ImagHofZ[j]) < 1.0E-12)RealHofZ[j] = 1.0E-12; // This happens in the nulls. Need to prevent a problem when plotting dB.
    }
   else
    {
     RealHofZ[j] = 1.0E-12;
     ImagHofZ[j] = 0.0;
    }
 }

}

/*
 This version of IIRFreqResponse uses complex variables. It is a bit easier to read.
 for(j=0; j<NumPts; j++)
  {
   Arg = M_PI * (double)j / (double)NumPts;
   z1 = ComplexD(cos(Arg), -sin(Arg));
   z2 = z1 * z1;

   Numerator   = ComplexD(1.0,0.0);
   Denominator = ComplexD(1.0,0.0);
   for(n=0; n<NumSections; n++)
	{
	 Numerator   *= IIR.b0[n] + IIR.b1[n] * z1 + IIR.b2[n] * z2;  // DFT
	 Denominator *= IIR.a0[n] + IIR.a1[n] * z1 + IIR.a2[n] * z2;
	}
   HofZ[j] =  Numerator / Denominator;
  }
*/

//---------------------------------------------------------------------------


