#pragma once
#define _SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING  1
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <complex>
#include <cmath>
#include <valarray>
#include <thread>
#include <mutex>
#include <queue>
#include <tuple>
#include <condition_variable>
#include <boost/interprocess/sync/interprocess_semaphore.hpp>
#include "ThreadMom.h"
#include "fft_CPP.h"
#include "fft_ASM.h"
#define WIDTH 800
#define HEIGHT 900

using namespace std;
using namespace sf;

typedef complex<double> Complex;
typedef valarray<Complex> CArray;
typedef complex<float> Complex_float;
typedef valarray<Complex_float> CArray_float;

const double PI = 3.141592653589793238460;

class FFT
{
public:
	FFT(string const& _path, int const& _bufferSize);
	FFT();
	~FFT() {}
	void readFile(string const& _path, int const& _bufferSize);
	void hammingWindow();

	void setBoost(float x);
	void setChoice(int x);
	void pause();
	void unpause();
	void stop();
	bool isPlaying();
	double getProgress();
	void setThreadCount(int x);
	void draw(RenderWindow& window);
	void update();
private:
	void bars(float const& max);
	void lines(float const& max);

	int choice = 0; //1 - cpp, 0 - asm
	ThreadMom tm;
	int threadCount;
	string path;
	SoundBuffer buffer;
	Sound sound;

	vector<Complex_float> sample_float;
	vector<float> window;
	CArray_float bin_float;

	VertexArray VA1;
	VertexArray VA2;
	VertexArray VA3;
	vector<Vertex> cascade;

	double length;
	float boost = 1;
	int sampleRate;
	int sampleCount;
	int bufferSize;
	int mark;

	//method invoking specific library
	void fft(std::valarray<std::complex<float>>& bin, int a, int b, ThreadMom& tm);
};

