
# COMPILE: g++ -D_FILE_OFFSET_BITS=64 IIRFilterCode.2.cpp QuadRootsCode.cpp WindowDataCode.cpp -g -lm 

# CONTENTS: 
# QuadRootsCode.cpp
	 QuadCubicRoots
	 	QuadRoots
	 	CubicRoots
	 	BiQuadRoots
# FFTCode.cpp
	RequiredFFTSize
	FFT
		IsValidFFTSize
		ReArrangeInput
		FillTwiddleArray
		Transform
	DFT
	RealSigDFT
	SingleFreqDFT
	Goertzel
	WindowData
		Bessel
		Sinc


# TO DO: 
[] write a main function to read data and call the function
[] get a function to actually apply the coeffcients, once calculated
[] write a general main filter program which reads a filter bank??


################################################################################
# 4 October 2015
# replicate previous filter results 
################################################################################
in1=/home/huxprog/lnSampleData2/Synthetic_Filter/makesignal2_005s-400Hz-001-200.txt

xe-filter_butterworth1 $in1 -sf 400 -low 15 -res 1.4142 | 
	xe-fftpow2 stdin -sf 400 -min 1 -max 150 -w 800 -s 2 -t 1 | 
	xe-plottable1 stdin -line 1 -ps 0 -xmin 0 -xint 25 -xscale .4 -ymin 0 -hline 1 -vline 15,30,45 -out temp1.ps 
	
# output coefficients: 	
	omega=0.118358
	a0=0.84646
	a1=-1.69292
	a2=0.84646
	b1=-1.66921
	b2=0.716636	
	
# write program to read data and include IIR functions

xs-progcompile xe-filter_IIR.1.c -c g++ -o ./

./xe-filter_IIR $in1 -filt butterworth -poles 10 -pt 3 -bw 1 -freq 140 -sf 400 -out 1 | 
	xe-fftpow2 stdin -sf 400 -min 1 -max 200 -w 800 -s 2 -t 1 | 
	xe-plottable1 stdin -line 1 -ps 0 -xmin 0 -xint 25 -xscale .4 -ymin 0 -hline 1 -vline 15,30,45 -out temp2.ps 


# RESULTS:
- BP filter looks good
- LP and HP look a bit weird - not sure why
- probably need to try inputing coefficients directly from filter designer 
