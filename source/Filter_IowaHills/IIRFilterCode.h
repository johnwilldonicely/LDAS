//---------------------------------------------------------------------------

#ifndef IIRFilterCodeH
#define IIRFilterCodeH
//---------------------------------------------------------------------------
 #define ARRAY_DIM 25

 #define OVERFLOW_LIMIT  1.0E20

 enum TIIRPassTypes {iirLPF, iirHPF, iirBPF, iirNOTCH};

 // The code relies on ftELLIPTIC_00 coming after the all pole filters, and being the 1st Elliptic.
 enum TIIRFilterTypes {ftBUTTERWORTH, ftGAUSSIAN, ftBESSEL, ftADJGAUSS, ftCHEBYSHEV, ftINVERSE_CHEBY,
                       ftELLIPTIC_00, ftELLIPTIC_02, ftELLIPTIC_04, ftELLIPTIC_06,  ftELLIPTIC_08,
                       ftELLIPTIC_10, ftELLIPTIC_12, ftELLIPTIC_14, ftELLIPTIC_16, ftELLIPTIC_18,
                       ftELLIPTIC_20, ftCOUNT, ftNOT_IIR};

 struct TSPlaneCoeff { double A[ARRAY_DIM]; double B[ARRAY_DIM]; double C[ARRAY_DIM];
                       double D[ARRAY_DIM]; double E[ARRAY_DIM]; double F[ARRAY_DIM];
                       int NumSections; };

 struct TIIRCoeff {double a0[ARRAY_DIM]; double a1[ARRAY_DIM]; double a2[ARRAY_DIM]; double a3[ARRAY_DIM]; double a4[ARRAY_DIM];
				   double b0[ARRAY_DIM]; double b1[ARRAY_DIM]; double b2[ARRAY_DIM]; double b3[ARRAY_DIM]; double b4[ARRAY_DIM];
                   int NumSections; };

 TIIRCoeff CalcIIRFilterCoeff(TIIRFilterTypes ProtoType, double Beta, TIIRPassTypes PassType,
                              int NumPoles, double OmegaC, double BW);

 TSPlaneCoeff GetSPlaneCoefficients(TIIRFilterTypes FilterType, int NumPoles, double Beta);


 void FilterWithIIR(TIIRCoeff IIRCoeff, double *Signal, double *FilteredSignal, int NumSigPts);
 double SectCalc(int j, int k, double x, TIIRCoeff IIRCoeff);
 void IIRFreqResponse(TIIRCoeff IIRCoeff, int NumSections, double *RealHofZ, double *ImagHofZ, int NumPts);
 void ExampleIIRCall(void);

#endif
