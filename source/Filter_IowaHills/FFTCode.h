//---------------------------------------------------------------------------

#ifndef FFTCodeH
#define FFTCodeH


#define M_2PI  6.28318530717958647692
#define MAXIMUM_FFT_SIZE  1048576
#define MINIMUM_FFT_SIZE  8
//---------------------------------------------------------------------------
 // Must retain the order on the 1st line for legacy FIR code.
 enum TWindowType {wtFIRSTWINDOW, wtNONE, wtKAISER, wtSINC, wtHANNING,
				   wtHAMMING, wtBLACKMAN, wtFLATTOP, wtBLACKMAN_HARRIS,
				   wtBLACKMAN_NUTTALL, wtNUTTALL, wtKAISER_BESSEL, wtTRAPEZOID,
                   wtGAUSS, wtSINE, wtTEST };

 enum TTransFormType {FORWARD, INVERSE};


 int RequiredFFTSize(int NumPts);
 int IsValidFFTSize(int x);
 void FFT(double *InputR, double *InputI, int N, TTransFormType Type);
 void ReArrangeInput(double *InputR, double *InputI, double *BufferR, double *BufferI, int *RevBits, int N);
 void FillTwiddleArray(double *TwiddleR, double *TwiddleI, int N, TTransFormType Type);
 void Transform(double *InputR, double *InputI, double *BufferR, double *BufferI, double *TwiddleR, double *TwiddleI, int N);
 void DFT(double *InputR, double *InputI, int N, int Type);
 void RealSigDFT(double *Samples, double *OutputR, double *OutputI, int N);
 double SingleFreqDFT(double *Samples, int N, double Omega);
 double Goertzel(double *Samples, int N, double Omega);
 void WindowData(double *Data, int N, TWindowType WindowType, double Alpha, double Beta, bool UnityGain);
 double Bessel(double x);
 double Sinc(double x);

#endif



/*

// This algorithm is a bit faster than the one in the cpp file.
// For some reason, it is faster to calculate the next twiddle factor with recursion
// than it is to access the Twiddle arrays. It is also faster to use the Sum and Twid
// variables in the inner loop rather than the Output and Twiddle arrays.
// 256 pts in 540 ms
void RealSigDFT(double *Samples, double *OutputR, double *OutputI, int N)
{
 int j, k;
 double Arg, Temp, zReal, zImag, TwidR, TwidI;  // z, as in e^(j*omega)
 double SumReal, SumImag;
 static double *TwiddleReal, *TwiddleImag;
 static int M = -1;

 // This calculates the twiddle factors on the 1st call or when N changes.
 if(M != N)
  {
   if(M != -1) // M = -1 on the 1st call.
    {
     delete[] TwiddleReal;
     delete[] TwiddleImag;
    }

   M = N;
   TwiddleReal = new(std::nothrow) double[N];
   TwiddleImag = new(std::nothrow) double[N];
   if(TwiddleReal == NULL || TwiddleImag == NULL)
    {
     ShowMessage("Failed to allocate memory in FIRFreqResponse");
     return;
    }

   for(j=0; j<N; j++)
    {
     Arg = M_2PI * (double)j / (double)N;
     TwiddleReal[j] = cos(Arg);
     TwiddleImag[j] = -sin(Arg);
    }
  }

 // We have a real input, so only do the pos frequencies. i.e. j<N/2
 for(j=0; j<N/2; j++)
  {
   zReal = 1.0;
   zImag = 0.0;
   SumReal = 0.0;
   SumImag = 0.0;
   TwidR = TwiddleReal[j];
   TwidI = TwiddleImag[j];

   for(k=0; k<N; k++)
	{
     SumReal += Samples[k] * zReal;
     SumImag += Samples[k] * zImag;

      // Calc the next twiddle factor recursively.
     Temp  = zReal * TwidR - zImag * TwidI;
     zImag = zReal * TwidI + zImag * TwidR;
     zReal = Temp;
	}

   OutputR[j] = SumReal / (double)N;
   OutputI[j] = SumImag / (double)N;
  }

 // The neg freq components are the conj of the pos components because the input signal is real.
 for(j=1; j<N/2; j++)
  {
   OutputR[N-j] = OutputR[j];
   OutputI[N-j] = -OutputI[j];
  }
}


*/
