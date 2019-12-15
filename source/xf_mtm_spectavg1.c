/*
<TAGS>stats signal_processing</TAGS>

DESCRIPTION:

	This function is derived from "adwait" function published here:
		Jonathan M. Lees and Jeffrey Park (1995)
		"MULTIPLE-TAPER SPECTRAL ANALYSIS: A STAND-ALONE C-SUBROUTINE"
		Computers & Geoscirnces Vol. 21, No. 2, pp. 199-236

	Uses David Thomson's algorithm for calculating the adaptive spectrum estimate
	That is, averages multiple eigenspectra using adaptive weighting

	In practice this seems to better preserve sharp peaks in the spectra for known
	strong signals (eg. hippocampal theta oscillations) than simply averaging
	the spectra conventionally, although amplitude is reduced more.

USES:

DEPENDENCY TREE:
	No external dependencies

ARGUMENTS:
	double *sqr_spec : (input) - amplitude spectra [ntapers*npoints, where npoints=size of FFT window]
	double *lambda : (input) - vector of eigenvalues [ntapers]
	int ntapers : (input) - number of tapers
	int nfreq : (input) - number of frequencies to analyze, starting from zero (typically npoints/2)
	double *ares : (result) - weighted average spectrum [nfreq]
	double *degf : (result) - degrees of freedom (can be used for calculating F, later)
	double avar : (input) - variance in the original input (before FFT)

RETURN VALUE:

SAMPLE CALL:

	xf_mtm_spectavg1(sqr_spec, dcf, lambda, ntapers, nfreqs, amu, degf, avar);

*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#define ABS(a) ((a) < (0) ? (-a) : (a))

int xf_mtm_spectavg1(double *sqr_spec,double *lambda, int ntapers, int nfreq, double *ares, double *degf, double avar) {

	double as, das, tol, a1, scale, ax, fn, fx;
	double *spw,*dcf,*bias;
	double test_tol, dif;
	int jitter, i, j, k, kpoint, jloop;
	float df;

#if 0
	fprintf(stderr, "test input\n adwait: %d %d %f\n", ntapers, nfreq, avar);
	fprintf(stderr, "\n Data=\n");
	for (i = 0; i < nfreq; i++) {fprintf(stderr, "%d %f \n", i, sqr_spec[i]);}
#endif

	/* set tolerance for iterative scheme exit */
	tol = 3.0e-4;
	jitter = 0;
	scale = avar;
	/***********************************
	we scale the bias by the total variance of the frequency transform
	from zero freq to the nyquist
	in this application we scale the eigenspectra by the bias in order to avoid
	possible floating point overflow
	************************************/
	spw = (double *) malloc((size_t) ntapers * sizeof(double));
	bias = (double *) malloc((size_t) ntapers * sizeof(double));
	dcf = (double *) malloc((size_t) (ntapers*nfreq) * sizeof(double));

	for (i=0;i<ntapers;i++) bias[i]= (1.00- lambda[i]);

	//	for( i=1;i<=ntapers; i++) fprintf(stderr,"%f %f\n",lambda[i], bias[i]); fprintf(stderr,"\n");


	/* START do 100 */
	for (jloop=0;jloop<nfreq;jloop++) {

		for (i=0;i<ntapers;i++) {
			kpoint= jloop + i * nfreq;
			spw[i]= (sqr_spec[kpoint]) / scale;
		}
		/********************************************
		first guess is the average of the two
		lowest-order eigenspectral estimates
		********************************************/
		as = (spw[0] + spw[1]) / 2.00;

		/* find coefficients */
		for (k = 0; k < 20; k++) {
			fn = fx = 0.00;
			for (i = 0; i < ntapers; i++) {
				a1 = sqrt(lambda[i]) * as / (lambda[i] * as + bias[i]);
				a1 = a1 * a1;
				fn = fn + a1 * spw[i];
				fx = fx + a1;
			}
			ax = fn / fx;
			dif = ax - as;
			das = ABS(dif);
			test_tol = das / as;
			if (test_tol < tol) break;
			as = ax;
		}
		/* fprintf(stderr,"adwait: k=%d test_tol=%f\n",k, test_tol); */

		/* flag if iteration does not converge */
		if (k >= 20) jitter++;

		/* calculate average weighted value for this frequency */
		ares[jloop] = as * scale;

		/* calculate degrees of freedom */
		df = 0.0;
		for (i = 0; i < ntapers; i++) {
			kpoint = jloop + i * nfreq;
			dcf[kpoint] = sqrt(lambda[i]) * as / (lambda[i] * as + bias[i]);
			df = df + dcf[kpoint] * dcf[kpoint];
		}

		/* normalize degrees of freedom by the weight of the first eigenspectrum - this way we never have fewer than two degrees of freedom */
		degf[jloop] = df * 2. / (dcf[jloop] * dcf[jloop]);

	}

	free(spw);
	free(bias);
	free(dcf);

	return(jitter);
}
