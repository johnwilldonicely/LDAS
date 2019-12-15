
/*
 By Iowa Hills Software, LLC  IowaHills.com
 If you find a problem, please leave a note at:
 http://www.iowahills.com/feedbackcomments.html
 December 5, 2014

 This root solver code is needed for the IIR band pass and IIR notch filters to
 reduce the 4th order terms to biquads.

 It is composed of the functions: QuadCubicRoots, QuadRoots, CubicRoots, and BiQuadRoots.
 This code originated at: http://www.netlib.org/toms/  Algorithm 326
 Roots of Low Order Polynomials by  Terence R.F.Nonweiler  CACM  (Apr 1968) p269
 Original C translation by: M.Dow  Australian National University, Canberra, Australia

 We made extensive modifications to the TOMS code in order to improve numerical accuracy.

 The polynomial format is: Coeff[0]*x^4 + Coeff[1]*x^3 + Coeff[2]*x^2 + Coeff[3]*x + Coeff[4]
 The roots are returned in RootsReal and RootsImag. N is the polynomial's order.  2 <= N <= 4
 N is modified if there are leading or trailing zeros.  N is returned.
 Coeff needs to be length N+1. RealRoot and ImagRoot need to be length N.

 Do not call QuadRoots, CubicRoots, and BiQuadRoots directly. They assume that QuadCubicRoots
 has removed leading and trailing zeros and normalized P.

---------------------------------------------------------------------------
Modifications by JRH: 4 October 2015
- remove requirement for vcl.h header for "ShowMessage" - just print to stderr instead
---------------------------------------------------------------------------

*/

#include "QuadRootsCode.h"
#include <math.h>
#include <stdio.h>

//---------------------------------------------------------------------------
int QuadCubicRoots(int N, double *Coeff, double *RootsReal, double *RootsImag)
{
 if(N <= 1 || N > 4)
  {
   fprintf(stderr,"Invalid Poly Order in QuadCubicRoots()");
   return(0);
  }

 int j, k;
 long double P[5], RealRoot[4], ImagRoot[4];

 // Must init to zero, in case N is reduced.
 for(j=0; j<4; j++)RealRoot[j] = ImagRoot[j] = 0.0;
 for(j=0; j<5; j++)P[j] = 0.0;

 // Reduce the order if there are trailing zeros.
 for(k=N; k>=0; k--)
  {
   if(fabs(Coeff[k]) > ZERO_PLUS)break;  // break on the 1st nonzero coeff
   Coeff[k] = 0.0;
   N--;
  }

 // Mandatory to remove leading zeros.
 while( fabs(Coeff[0]) < ZERO_PLUS && N>0)
  {
   for(k=0; k<N; k++)
    {
     Coeff[k] = Coeff[k+1];
    }
   Coeff[N] = 0.0;
   N--;
  }

 // The functions below modify the coeff array, so we pass P instead of Coeff.
 for(j=0; j<=N; j++)P[j] = Coeff[j];

 // Mandatory to normalize the coefficients
 if(P[0] != 1.0)
  {
   for(k=1; k<=N; k++)
    {
     P[k] /= P[0];
    }
   P[0] = 1.0;
  }

 if(N==4)BiQuadRoots(P, RealRoot, ImagRoot);
 if(N==3)CubicRoots(P, RealRoot, ImagRoot);
 if(N==2)QuadRoots(P, RealRoot, ImagRoot);
 if(N==1)
  {
   RealRoot[0] = -P[1]/P[0];
   ImagRoot[0] = 0.0;
  }

 // We used separate long double arrays in the function calls for interface reasons.
 // if N==0 all zeros are returned (we init RealRoot and ImagRoot to 0).
 // for(j=0; j<4; j++)Roots[j] = ComplexD(RealRoot[j], ImagRoot[j]);
 for(j=0; j<4; j++)RootsReal[j] = RealRoot[j];
 for(j=0; j<4; j++)RootsImag[j] = ImagRoot[j];
 return(N);
}

//---------------------------------------------------------------------------
// This function is the quadratic formula with P[0] = 1
// y = P[0]*x^2 + P[1]*x + P[2]
void QuadRoots(long double *P, long double *RealRoot, long double *ImagRoot)
{
 long double g;

 g = P[1]*P[1] - 4.0*P[2];
 if(fabsl(g) < ZERO_PLUS)g = 0.0;

 if(g >= 0.0)  // 2 real roots
  {
   RealRoot[0] = (-P[1] + sqrtl(g)) / 2.0;
   RealRoot[1] = (-P[1] - sqrtl(g)) / 2.0;
   ImagRoot[0] = 0.0;
   ImagRoot[1] = 0.0;
  }
 else // 2 complex roots
  {
   RealRoot[0] = -P[1] / 2.0;
   RealRoot[1] = -P[1] / 2.0;
   ImagRoot[0] = sqrtl(-g) / 2.0;
   ImagRoot[1] = -sqrtl(-g) / 2.0;
  }
}

//---------------------------------------------------------------------------
// This finds the roots of y = P0x^3 + P1x^2 + P2x+ P3   P[0] = 1
// The return value indicates the location of the largest positive root which is used by BiQuadRoots.
int CubicRoots(long double *P, long double *RealRoot, long double *ImagRoot)
{
 long double  s, t, b, c, d;

 s = P[1] / 3.0;
 b = (6.0*P[1]*P[1]*P[1] - 27.0*P[1]*P[2] + 81.0*P[3]) / 162.0;
 t = (P[1]*P[1] - 3.0*P[2]) / 9.0;
 c = t * t * t;
 d = 2.0*P[1]*P[1]*P[1] - 9.0*P[1]*P[2] + 27.0*P[3];
 d = d * d / 2916.0 - c;

 if(d > ZERO_PLUS) // 1 complex and 1 real root
  {
   d = powl( (sqrtl(d) + fabsl(b)), 1.0/3.0);
   if(d != 0.0)
    {
     if(b>0) b = -d;
     else b = d;
     c = t / b;
    }
   d = M_SQRT3_2 * (b-c);
   b = b + c;
   c = -b/2.0 - s;

   RealRoot[0] = c;
   ImagRoot[0] = -d;
   RealRoot[1] = c;
   ImagRoot[1] = d;
   RealRoot[2] = b-s;
   if( fabsl(RealRoot[2]) < ZERO_PLUS)RealRoot[2] = 0.0;
   ImagRoot[2] = 0.0;
   return(2); // Return 2 because it contains the real root.
  }

 else // d < 0.0  3 real roots
  {
   if(b == 0.0)d = M_PI_2 / 3.0; //  b can be as small as 1.0E-25
   else d = atanl(sqrtl(fabsl(d))/fabsl(b)) / 3.0;

   if(b < 0.0) b =  2.0 * sqrtl(fabsl(t));
   else        b = -2.0 * sqrtl(fabsl(t));

   c = cosl(d) * b;
   t = -M_SQRT3_2 * sinl(d) * b - 0.5 * c;

   RealRoot[0] = t - s;
   RealRoot[1] = c - s;
   RealRoot[2] = -(t + c + s);
   ImagRoot[0] = 0.0;
   ImagRoot[1] = 0.0;
   ImagRoot[2] = 0.0;

   if( fabsl(RealRoot[0]) < ZERO_PLUS)RealRoot[0] = 0.0;
   if( fabsl(RealRoot[1]) < ZERO_PLUS)RealRoot[1] = 0.0;
   if( fabsl(RealRoot[2]) < ZERO_PLUS)RealRoot[2] = 0.0;

   int MaxK = 0;
   if(RealRoot[1] > RealRoot[MaxK])MaxK = 1;
   if(RealRoot[2] > RealRoot[MaxK])MaxK = 2;
   return(MaxK);  // Return the index with the largest real root.
 }

}

//---------------------------------------------------------------------------
// This finds the roots of  y = P0x^4 + P1x^3 + P2x^2 + P3x + P4    P[0] = 1
// This function calls CubicRoots
void BiQuadRoots(long double *P, long double *RealRoot, long double *ImagRoot)
{
 int k, MaxK;
 long double a, b, c, d, e, g, P1, P3Limit;

 P1 = P[1];

 e = P[1]*0.25;
 d = P[1]*P[1]*0.1875;   // 0.1875 = 3/16
 b = P[3] + P[1]*P[1]*P[1]*0.125 - P[1]*P[2]*0.5;
 c = 256.0*P[4] + 16.0*P[1]*P[1]*P[2];
 c += -3.0*P[1]*P[1]*P[1]*P[1] - 64.0*P[1]*P[3];
 c *= 0.00390625;             // 0.00390625 = 1/256
 a = P[2] - P[1]*P[1]*0.375;  // 0.375 = 3/8

 P[1] = P[2]*0.5 - P[1]*P[1]*0.1875;  // 0.1875 = 3/16
 P[2] = 16.0*P[2]*P[2]  - 16.0*P1*P1*P[2] + 3.0*P1*P1*P1*P1;
 P[2] += -64.0*P[4] + 16.0*P1*P[3];
 P[2] *= 3.90625E-3;    // 3.90625E-3 = 1/256
 P[3] = -b*b*0.015625;  // 0.015625 = 1/64
 if(P[3] > 0.0)P[3] = 0.0; // Only numerical errors make P[3] > 0


/* Any inaccuracies in this algorithm are directly attributable to the magnitude of P[3]
 which takes on values of:  -1E30 < P[3] <= 0.0
 If P[3] = 0 we use the quadratic formula to get the roots (i.e. the 3rd root is at 0).
 Inaccuracies are caused in CubicRoots when P[3] is either huge or tiny because of the loss of
 significance that ocurrs there in the calculation of b and d. P[3] can also cause problems when it
 should have calculated to zero (just above) but it is instead ~ -1.0E-17 because of round off errors.
 Consequently, we need to determine whether a tiny P[3] is due to roundoff, or if it is legitimately small.
 It can legitimately have values of ~-1E-28 . When this happens, we assume P2 should also be small.
 We use the following criteria to test for a legitimate tiny P[3], which we must to send to CubicRoots */


 // if P[3] is tiny && legitimately tiny we use 0 as the threshold.
 // else P[3] must be more negative than roundoff errors cause.
 if(P[3] > ZERO_MINUS && fabs(P[2]) < 1.0E-6) P3Limit = 0.0;
 else P3Limit = ZERO_MINUS;

 if(P[3] < P3Limit)
  {
   MaxK = CubicRoots(P, RealRoot, ImagRoot);
   if(RealRoot[MaxK] > 0.0)  // MaxK is the index of the largest real root.
    {
     d = 4.0*RealRoot[MaxK];
     a += d;
     if(a*b < 0.0)P[1] = -sqrtl(d);
     else         P[1] = sqrtl(d);
     b = 0.5 * (a + b/P[1]);
     goto QUAD;
    }
  }

 if(P[2] < -1.0E-8)  // 2 sets of equal imag roots
  {
   b = sqrtl(fabsl(c));
   d = b + b - a;
   if(d > 0.0)P[1] = sqrtl(fabsl(d));
   else       P[1] = 0.0;
  }
 else
  {
   if(P[1] > 0.0)b =  2.0*sqrtl(fabsl(P[2])) + P[1];
   else          b = -2.0*sqrtl(fabsl(P[2])) + P[1];

   if(fabsl(b) < 10.0*ZERO_PLUS) // 4 equal real roots.  Was originally if(b == 0.0)
    {
     for(k=0; k<4; k++)
      {
       RealRoot[k] = -e;
       ImagRoot[k] = 0.0;
      }
     return;
    }
   else P[1] = 0.0;
  }

 // Calc the roots from two 2nd order polys and subtract e from the real part.
 QUAD:
 P[2] = c/b;
 QuadRoots(P, RealRoot, ImagRoot);

 P[1] = -P[1];
 P[2] = b;
 QuadRoots(P, RealRoot+2, ImagRoot+2);

 for(k=0; k<4; k++)RealRoot[k] -= e;

}

//---------------------------------------------------------------------------

