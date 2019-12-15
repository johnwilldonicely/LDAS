//---------------------------------------------------------------------------

#ifndef QuadRootsCodeH
#define QuadRootsCodeH
//---------------------------------------------------------------------------

// #define M_SQRT3 1.7320508075688772935274463    // sqrt(3)
 #define M_SQRT3_2 0.8660254037844386467637231    // sqrt(3)/2
// #define DBL_EPSILON  2.2204460492503131E-16    // 2^-52  typically defined in the compiler's float.h
 #define ZERO_PLUS   8.88178419700125232E-16      // 2^-50 = 4*DBL_EPSILON
 #define ZERO_MINUS -8.88178419700125232E-16

 int  QuadCubicRoots(int N, double *Coeff, double *RealPart, double *ImagPart);
 void QuadRoots(long double *P, long double *RealPart, long double *ImagPart);
 int CubicRoots(long double *P, long double *RealPart, long double *ImagPart);
 void BiQuadRoots(long double *P, long double *RealPart, long double *ImagPart);


#endif
