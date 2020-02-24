#include "fft_CPP.h"
#include "ThreadMom.h"

extern "C" float sineASM_float(float x); //sinus function written in asm - used in both implementations for a proper comparison
void fft_CPP(float* real, float* imag, int N, ThreadMom& tm)
{	
	if (N <= 1) return; //algorythm condition to finish recurrence

	//must copy given array into two smaller ones - even and odd
	float* realEven = new float[N / 2 + N % 2];
	float* imagEven = new float[N / 2 + N % 2];
	float* realOdd = new float[N / 2];
	float* imagOdd = new float[N / 2];

	//copying arrays
	int i = 0;
	int k = 0;
	for (; k < N / 2; )
	{
		realEven[k] = real[i];
		imagEven[k] = imag[i];
		i++;
		realOdd[k] = real[i];
		imagOdd[k] = imag[i];
		i++; k++;
	}
	if (i < N)
	{
		realEven[k] = real[i];
		imagEven[k] = imag[i];
	}

	//ask for a free thread, to do the calculation even or odd part of the sample
	Response r1 = tm.askForHelp_float({ realEven, imagEven, N / 2 });
	Response r2 = tm.askForHelp_float({ realOdd, imagOdd, N / 2 });

	if (r1.finish == nullptr && r2.finish == nullptr)
	{ //no free thread - calculate in this one
		fft_CPP(realEven, imagEven, N / 2, tm);
		fft_CPP(realOdd, imagOdd, N / 2, tm);
	}
	else if (r1.finish != nullptr && r2.finish == nullptr) 
	{ //only one thread free - wait for it to finish after calculating the other half in this one
		fft_CPP(realOdd, imagOdd, N / 2, tm);
		r1.finish->wait();
		r1.accept->post();
	}
	else if (r1.finish == nullptr && r2.finish != nullptr) 
	{ //only one thread free - wait for it to finish after calculating the other half in this one
		fft_CPP(realEven, imagEven, N / 2, tm);
		r2.finish->wait();
		r2.accept->post();
	}
	else {//two threads free - wait for them to finish
		r1.finish->wait();
		r1.accept->post();
		r2.finish->wait();
		r2.accept->post();
	}

	for (int k = 0; k < N / 2; k++)
	{ //main calculation X[k] = X_even [k] + X_odd[k] * e^(-2k*pi/n)
		float polarReal = sineASM_float(negPi2f * k / N + piHalff);
		float polarImag = sineASM_float(negPi2f * k / N);

		float realTemp = polarReal * realOdd[k] - polarImag * imagOdd[k];
		float imagTemp = polarReal * imagOdd[k] + polarImag * realOdd[k];

		//x[k] = even[k] + t;
		real[k] = realEven[k] + realTemp;
		imag[k] = imagEven[k] + imagTemp;
		//x[k + N / 2] = even[k] - t;
		real[k + N / 2] = realEven[k] - realTemp;
		imag[k + N / 2] = imagEven[k] - imagTemp;
	}
	delete[] realEven;
	delete[] imagEven;
	delete[] realOdd;
	delete[] imagOdd;
}
