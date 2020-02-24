#include "FFT.h"

FFT::FFT(string const& _path, int const& _bufferSize) : FFT() {
	path = _path;
	if (!buffer.loadFromFile(path)) cout << "Unable to load buffer" << endl;

	sound.setBuffer(buffer);
	sound.setLoop(true);
	sound.play();

	sampleRate = buffer.getSampleRate() * buffer.getChannelCount();
	sampleCount = buffer.getSampleCount();
	if (_bufferSize < sampleCount) bufferSize = _bufferSize;
	else bufferSize = sampleCount;
	mark = 0;

	for (int i(0); i < bufferSize; i++) window.push_back(0.54 - 0.46 * cos(2 * PI * i / (float)bufferSize));

	sample_float.resize(bufferSize);
	VA1.resize(bufferSize);
}

FFT::FFT() : tm(1, &fft_ASM, &fft_CPP) {
	threadCount = 1;
	VA1.setPrimitiveType(LineStrip);
	VA2.setPrimitiveType(Lines);
	VA3.setPrimitiveType(LineStrip);
	choice = 1;
}

void FFT::readFile(string const& _path, int const& _bufferSize) {

	path = _path;
	if (!buffer.loadFromFile(path)) {
		cout << "Unable to load buffer" << endl;
		exit(0);
	}

	sound.setBuffer(buffer);
	sound.setLoop(true);
	sound.play();
	sampleRate = buffer.getSampleRate() * buffer.getChannelCount();
	sampleCount = buffer.getSampleCount();
	length = sampleCount / sampleRate;
	if (_bufferSize < sampleCount) bufferSize = _bufferSize;
	else bufferSize = sampleCount;
	mark = 0;
	if (window.size() > 0)
	{
		window.clear();
	}
	for (int i(0); i < bufferSize; i++) window.push_back(0.54 - 0.46 * cos(2 * PI * i / (float)bufferSize));

	sample_float.resize(bufferSize);
	VA1.resize(bufferSize);
}

void FFT::hammingWindow()
{
	mark = sound.getPlayingOffset().asSeconds() * sampleRate;
	if (mark + bufferSize < sampleCount)
	{
		for (int i(mark); i < bufferSize + mark; i++)
		{
			sample_float[i - mark] = Complex_float(buffer.getSamples()[i] * window[i - mark], 0);
			VA1[i - mark] = Vertex(
				Vector2f(20, HEIGHT * 0.3) + Vector2f((i - mark) /
				(float)bufferSize * WIDTH * 2 / 3, sample_float[i - mark].real() * 0.005 * (boost / 5.0 + 0.8)),
				Color::Color(255, 0, 0, 50)
			);
		}
	}
}

void FFT::setBoost(float x) {
	boost = x;
}

void FFT::setChoice(int x) {
	tm.setChoice(x);
	choice = x;
}

void FFT::pause() {
	sound.pause();
}

void FFT::unpause() {
	sound.play();
}

void FFT::stop() {
	sound.stop();
	cascade.clear();

	for (int i = 0; i < bufferSize; i++)
	{
		sample_float[i] = 0;
		VA1[i] = Vertex(Vector2f(20, 250) + Vector2f((i - mark) / (float)bufferSize * 700, 0), Color::Color(255, 0, 0, 50));
	}

	VA2.clear();
	VA3.clear();

	bin_float = CArray_float(sample_float.data(), bufferSize);
	fft(bin_float, 0, bin_float.size(), tm);
	float max = 100000000;
	lines(max);
	bars(max);

}

bool FFT::isPlaying() {
	if (sound.getStatus() == sf::Sound::Status::Playing) {
		return true;
	}
	return false;
}

double FFT::getProgress() {
	return sound.getPlayingOffset().asMilliseconds() / 1000.0 / length;
}

void FFT::setThreadCount(int x)
{
	threadCount = x;
	tm.reset(x);
}

void FFT::draw(RenderWindow& window) {
	window.draw(VA1);
	window.draw(VA3);
	window.draw(VA2);
}

void FFT::update()
{
	hammingWindow();

	VA2.clear();
	VA3.clear();

	bin_float = CArray_float(sample_float.data(), bufferSize);

	fft(bin_float, 0, bin_float.size(), tm);
	float max = 100000000;
	lines(max);
	bars(max);
}

void FFT::bars(float const& max) {
	VA2.setPrimitiveType(Lines);
	Vector2f position(0, HEIGHT * 0.9);
	for (float i(3); i < min(bufferSize / 2.f, 20000.f); i *= 1.01)
	{
		Vector2f samplePosition(log(i) / log(min(bufferSize / 2.f, 20000.f)), boost * abs(bin_float[(int)i]));
		VA2.append(Vertex(position + Vector2f(samplePosition.x * WIDTH * 0.9, -samplePosition.y / max * 500), Color::White));
		VA2.append(Vertex(position + Vector2f(samplePosition.x * WIDTH * 0.9, 0), Color::White));
		VA2.append(Vertex(position + Vector2f(samplePosition.x * WIDTH * 0.9, 0), Color::Color(255, 255, 255, 100)));
		VA2.append(Vertex(position + Vector2f(samplePosition.x * WIDTH * 0.9, samplePosition.y / max * 500 / 2.f), Color::Color(255, 255, 255, 0)));
	}
}

void FFT::lines(float const& max) {
	VA3.setPrimitiveType(LineStrip);
	Vector2f position(0, HEIGHT * 0.9);
	Vector2f samplePosition;
	float colorDecay = 1;

	for (float i(std::max((double)0, cascade.size() - 3e5)); i < cascade.size(); i++)
	{
		cascade[i].position -= Vector2f(-0.8, 1);
		if (cascade[i].color.a != 0) cascade[i].color = Color(255, 255, 255, 20);
	}
	samplePosition = Vector2f(log(3) / log(min(bufferSize / 2.f, 20000.f)), boost * abs(bin_float[(int)3]));
	cascade.push_back(Vertex(position + Vector2f(samplePosition.x * WIDTH * 0.9, -samplePosition.y / max * 500), Color::Transparent));
	for (float i(3); i < bufferSize / 2.f; i *= 1.02)
	{
		samplePosition = Vector2f(log(i) / log(min(bufferSize / 2.f, 20000.f)), boost * abs(bin_float[(int)i]));
		cascade.push_back(Vertex(position + Vector2f(samplePosition.x * WIDTH * 0.9, -samplePosition.y / max * 500), Color::Color(255, 255, 255, 20)));
	}
	cascade.push_back(Vertex(position + Vector2f(samplePosition.x * WIDTH * 0.9, -samplePosition.y / max * 500), Color::Transparent));
	if (cascade.size() > 5)
		cascade.pop_back();
	VA3.clear();
	for (int i(std::max((double)0, cascade.size() - 3e5)); i < cascade.size(); i++) VA3.append(cascade[i]);
}

void FFT::fft(std::valarray<std::complex<float>>& bin, int a, int b, ThreadMom& tm)
{
	int N = b - a;
	float* real, * imag;
	real = new float[N];
	imag = new float[N];
	for (int i = a; i < b; i++)
	{
		real[i - a] = bin[i].real();
		imag[i - a] = bin[i].imag();
	}
	if (choice == 0) {

		fft_ASM(real, imag, N, tm);
	}
	else {
		fft_CPP(real, imag, N, tm);
	}
	for (int i = a; i < b; i++)
	{
		bin[i].operator=(Complex_float(real[i - a], imag[i - a]));
	}
	delete[] real;
	delete[] imag;
}