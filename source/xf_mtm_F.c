/*
<TAGS>stats signal_processing</TAGS>

DESCRIPTION:
	Calculate the F-statistic for a multi-taper power spectrum estimate

	This function is derived from the "get_F_values" function published here:
		Jonathan M. Lees and Jeffrey Park (1995)
		"MULTIPLE-TAPER SPECTRAL ANALYSIS: A STAND-ALONE C-SUBROUTINE"
		Computers & Geoscirnces Vol. 21, No. 2, pp. 199-236

	Changes (JRH, January 2013)
		- first argument is now a structure (complex Kiss-FFT output fft[].r fft[].i)  instead of separate arrays holding real and imaginary FFT output

USES:

DEPENDENCY TREE:
	requires "kiss_fftr.h" in order to define the complex data type "fft"

ARGUMENTS:
	double *sr	: (input) - real component of the FFT results
	double *si	: (input) - imaginary component of the FFT results
	int nf		: (input) - number of frequencies to be analyzed (typically 0.5 x length of the data the FFT was performed on)
	int nwin	: (input) - number of tapers
	float *Fval : (output) - holds the f-values [nf]
	double *tapsum	: (input) - sum of the taper values for each taper

RETURN VALUE:

SAMPLE CALL:
		get_F_values(ReSpec, ImSpec, num_freqs, nwin, Fval, tapsum);

*/
#include "kiss_fftr.h"
#define SQR(a) ((a) == 0.0 ? 0.0 : (a)*(a))

void xf_mtm_F(kiss_fft_cpx *fft, int nf, int nwin, float *Fval, double *tapsum) {

	/*
	 tapsum is the FFT of slepian eigentapers at zero freq
	 sr si are the eigenspectra
	 amu contains line frequency estimates and f-test parameter
	*/

	int i, j, k;
	double *amur, *amui, sum, sumr, sumi, sum2;

	sum = 0.;
	amur = (double *) malloc((size_t) nf * sizeof(double));
	amui = (double *) malloc((size_t) nf * sizeof(double));

	for (i=0;i<nwin;i++) sum+= tapsum[i] * tapsum[i];

	for (i = 0; i < nf; i++) {
		amur[i] = 0.;
		amui[i] = 0.;
		for (j = 0; j < nwin; j++) {
			k = i + j * nf;
			amur[i] = amur[i] + fft[k].r * tapsum[j];
			amui[i] = amui[i] + fft[k].i * tapsum[j];
		}
		amur[i] = amur[i] / sum;
		amui[i] = amui[i] / sum;
		sum2 = 0.;
		for (j = 0; j < nwin; j++) {
			k = i + j * nf;
			sumr = fft[k].r - amur[i] * tapsum[j];
			sumi = fft[k].i - amui[i] * tapsum[j];
			sum2 = sum2 + sumr * sumr + sumi * sumi;
		}
		Fval[i] = (float) (nwin - 1) * (SQR(amui[i]) + SQR(amur[i])) * sum / sum2;
	}
	free(amui);
	free(amur);
	return;
}
