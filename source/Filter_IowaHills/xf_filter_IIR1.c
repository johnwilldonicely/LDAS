#define REG_SIZE 100
int NumSections;  // The number of biquad sections
double Reg0[REG_SIZE], Reg1[REG_SIZE], Reg2[REG_SIZE];     // Used in the Form 2 code
double a2[REG_SIZE], a1[REG_SIZE], a0[REG_SIZE], b2[REG_SIZE], b1[REG_SIZE], b0[REG_SIZE]; // The 2nd order IIR coefficients.

//---------------------------------------------------------------------------

// Form 2 Biquad
// This uses one set of shift registers, Reg0, Reg1, and Reg2 in the center.
void RunIIRBiquadForm2(double *Input, double *Output, long NumSigPts)
{
 double aa;
 long ii,jj;

 for(ii=0; ii<REG_SIZE; ii++) // Init the shift registers.
  {
   Reg0[ii] = 0.0;
   Reg1[ii] = 0.0;
   Reg2[ii] = 0.0;
  }

 for(ii=0;ii<NumSigPts;ii++)
  {
  aa = SectCalcForm2(0, Input[ii]);
   for(jj=1; jj<NumSections; jj++)
	{
	 aa = SectCalcForm2(jj, aa);
	}
   Output[ii] = aa;
  }
}

//---------------------------------------------------------------------------

// Form 2 Biquad Section Calc, called by RunIIRBiquadForm2.
double SectCalcForm2(long jj, double aa)
{
 double bb;

 Reg0[jj] = aa - a1[jj] * Reg1[jj] - a2[jj] * Reg2[jj];
 bb = b0[jj] * Reg0[jj] + b1[jj] * Reg1[jj] + b2[jj] * Reg2[jj];

 // Shift the register values
 Reg2[jj] = Reg1[jj];
 Reg1[jj] = Reg0[jj];

 return(bb);
}

//---------------------------------------------------------------------------


/*
The contents of the header file for this code

#define REG_SIZE 100

int NumSections; // The number of biquad sections. e.g. PoleCount/2 for even PoleCount.
double Reg0[REG_SIZE], Reg1[REG_SIZE], Reg2[REG_SIZE];		   // Used in the Form 2 code
double a2[REG_SIZE], a1[REG_SIZE], a0[REG_SIZE], b2[REG_SIZE], b1[REG_SIZE], b0[REG_SIZE]; // The 2nd order IIR coefficients.

void RunIIRBiquadForm2(double *Input, double *Output, int NumSigPts);
double SectCalcForm2(int k, double x);

*/
