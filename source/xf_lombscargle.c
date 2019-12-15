/**************************************************************************
�* Copyright 2010 Pim Schellart. All rights reserved.    �
�*                  *
�* Redistribution and use in source and binary forms, with or  �
�* without modification, are permitted provided that the following  *
�* conditions are met:             *
�*                  *
�* 1. Redistributions of source code must retain the above  �
�* copyright notice, this list of conditions and the following � *
�* disclaimer.             � *
�*                  *
�* 2. Redistributions in binary form must reproduce the above  � *
�* copyright notice, this list of conditions and the following � *
�* disclaimer in the documentation and/or other materials
�* provided with the distribution.        � *
�*                  *
�* THIS SOFTWARE IS PROVIDED BY PIM SCHELLART ``AS IS'' AND ANY
�* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE � *
�* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR�
�* PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL PIM SCHELLART OR  �
�* CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,� *
�* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, *
�* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR�
�* PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY *
�* OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
�* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE � *
�* USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF   *
�* SUCH DAMAGE.
�*                  *
�* The views and conclusions contained in the software and documentation� *
�* are those of the authors and should not be interpreted as representing *
�* official policies, either expressed or implied, of Pim Schellart. � *
�**************************************************************************/

/**
<TAGS>signal_processing</TAGS>
DESCRIPTION:
	Purpose
	=======
	Computes the Lomb-Scargle periodogram as developed by Lomb (1976)
	and further extended by Scargle (1982) to find, and test the
	significance of weak periodic signals with uneven temporal sampling.

	This subroutine calculates the periodogram using a slightly
	modified algorithm due to Townsend (2010) which allows the
	periodogram to be calculated using only a single pass through
	the input samples.
	This requires Nw(2Nt+3) trigonometric function evaluations (where
	Nw is the number of frequencies and Nt the number of input samples),
	giving a factor of ~2 speed increase over the straightforward
	implementation.

	Arguments
	=========
	t(input) double precision array, dimension (Nt)	
	 Sample times

	x(input) double precision array, dimension (Nt)
	 Measurement values

	w(input) double precision array, dimension (Nt)
	 Angular frequencies for output periodogram

	P(output) double precision array, dimension (Nw)
	 Lomb-Scargle periodogram

	Nt (input) integer
	 Dimension of input arrays

	Nw (output) integer
	 Dimension of output array

	Further details
	===============

	P[i] takes a value of A^2*N/4 for a harmonic signal with
	frequency w(i).
**/

/*
ADDITIONAL NOTES [JRH, 7 JAN 2013]

- input should be the autocorrelogram for a process (interval,count)
- input data X[] should be normalized to mean=0 and s.d.=1
- only positive time values t[] should be passed

- frequency values ( w[] ) should range from
	min: 2*PI*(1.0/t[Nt])
	max: 2*PI*(1.0/binwidth) * 0.25
	- binwidth is the interval between data for the autocorrelogram, and represents the highest frequency measurable
	- the periodogram repeats for frequencies greater than 0.5 * the sample rate
	- and in fact, 0.25 to 0.5 is a mirror image of 0 to 0.25 - hence only frequencies up to 0.25 * sample rate are of interest
	- note also that the frequency values need to be divided by 2*PI after the function is called to get "proper" frequency values

- a value of 100 for Nw produces aesthetically pleasing periodograms
- scaling for P seems to be required but not sure
- significance level is not computed here

*/

#include <stdio.h>
#include <math.h>

void xf_lombscargle(double* t, double* x, double* freq, double* P, int Nt, int Nfreq) {

	int i,j;
	double f,c, s, xc, xs, cc, ss, cs;
	double tau, c_tau, s_tau, c_tau2, s_tau2, cs_tau, term0, term1;
	double twopi=2.0*M_PI;

	/* Get pointers to input arrays (faster than array indexing).
	Note that all arrays should be contiguous which is taken care of by f2py
	*/
	double* tp = t;
	double* xp = x;

	/* Use x86 decrement and compare to zero instruction (faster than ++) */
	i = Nfreq;
	while(--i) {
		xc = xs = cc = ss = cs = 0.0;
		f= *freq * twopi;
		/* (Re)set pointers to start of input arrays */
		tp= t;	xp= x;	j= Nt;
		while (--j) {
			c = cos(f * *tp);
			s = sin(f * *tp);
			xc += *xp * c;
			xs += *xp * s;
			cc += c * c;
			ss += s * s;
			cs += c * s;
			/* Next element in input arrays */
			++tp;	++xp;
		}

		tau = atan(2 * cs / (cc - ss)) / (2 * f);
		c_tau = cos(f * tau);
		s_tau = sin(f * tau);

		c_tau2 = c_tau * c_tau;
		s_tau2 = s_tau * s_tau;
		cs_tau = 2 * c_tau * s_tau;

		term0 = c_tau * xc + s_tau * xs;
		term1 = c_tau * xs - s_tau * xc;

		*P = 0.5 * (((term0 * term0) / (c_tau2 * cc + cs_tau * cs + s_tau2 * ss)) + ((term1 * term1) / (c_tau2 * ss - cs_tau * cs + s_tau2 * cc)));

		/* Next frequency */
		++freq;
		++P;
	}
}
