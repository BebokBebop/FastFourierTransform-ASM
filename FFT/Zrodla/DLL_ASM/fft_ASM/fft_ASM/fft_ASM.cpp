#include "fft_ASM.h"
#include "ThreadMom.h"
const float piHalff = 1.57079632679489661923;
const float negPi2f = -6.28318530717958647692;

extern "C" float sineASM_float(float x);
extern "C" void funcASM_float_v(int N, float* real, float* imag, float* realOdd, float* imagOdd, float* realEven, float* imagEven);

void fft_ASM(float* real, float* imag, int N, ThreadMom& tm) {
	if (N <= 1) return;

	float* realEven = new float[N / 2 + N % 2];
	float* imagEven = new float[N / 2 + N % 2];
	float* realOdd = new float[N / 2];
	float* imagOdd = new float[N / 2];
	//if (!(N / 2) % 2)
	//{
	//	return;
	//}
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

	Response r1 = tm.askForHelp_float({ realEven, imagEven, N / 2 });
	Response r2 = tm.askForHelp_float({ realOdd, imagOdd, N / 2 });

	if (r1.finish == nullptr && r2.finish == nullptr)
	{
		fft_ASM(realEven, imagEven, N / 2, tm);
		fft_ASM(realOdd, imagOdd, N / 2, tm);
	}
	else if (r1.finish != nullptr && r2.finish == nullptr) {
		fft_ASM(realOdd, imagOdd, N / 2, tm);
		r1.finish->wait();
		r1.accept->post();
	}
	else if (r1.finish == nullptr && r2.finish != nullptr) {
		fft_ASM(realEven, imagEven, N / 2, tm);
		r2.finish->wait();
		r2.accept->post();
	}
	else {
		r1.finish->wait();
		r1.accept->post();
		r2.finish->wait();
		r2.accept->post();
	}

	//funcASM_float(N, real, imag, realOdd, imagOdd, realEven, imagEven);
	funcASM_float_v(N, real, imag, realOdd, imagOdd, realEven, imagEven);
	//
	//for (int k = 0; k < N / 2; k++)
	//{
	//	float polarReal = sineASM(-2 * PI * k / N + PI / 2);
	//	float polarImag = sineASM(-2 * PI * k / N);

	//	float realTemp = polarReal * realOdd[k] - polarImag * imagOdd[k];
	//	float imagTemp = polarReal * imagOdd[k] + polarImag * realOdd[k];
	//	
	//	//x[k] = even[k] + t;
	//	real[k] = realEven[k] + realTemp;
	//	imag[k] = imagEven[k] + imagTemp;
	//	//x[k + N / 2] = even[k] - t;
	//	real[k + N / 2] = realEven[k] - realTemp;
	//	imag[k + N / 2] = imagEven[k] - imagTemp;
	//}
	delete[] realEven;
	delete[] imagEven;
	delete[] realOdd;
	delete[] imagOdd;

}
