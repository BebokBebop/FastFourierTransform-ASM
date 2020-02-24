#pragma once

class ThreadMom;
extern "C"  _declspec(dllexport) void fft_ASM(float* real, float* imag, int N, ThreadMom & tm);