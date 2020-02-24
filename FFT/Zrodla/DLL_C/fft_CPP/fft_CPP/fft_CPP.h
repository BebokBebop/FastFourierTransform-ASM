#pragma once

class ThreadMom;
const float piHalff = 1.57079632679489661923;
const float negPi2f = -6.28318530717958647692;

extern "C" _declspec(dllexport) 
	void fft_CPP(float* real, float* imag, int N, ThreadMom & tm);
