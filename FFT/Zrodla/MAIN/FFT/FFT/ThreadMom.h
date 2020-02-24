#pragma once
#include <boost/interprocess/sync/interprocess_semaphore.hpp>
#include <thread>
#include<mutex>

typedef boost::interprocess::interprocess_semaphore SemaphoreSource;

class Semaphore : public SemaphoreSource {
public:
	Semaphore() :SemaphoreSource(0) {

	}
};

struct Response {
	Semaphore* finish;
	Semaphore* accept;
};

struct Data_float {
	float* real;
	float* imag;
	int size;
};

class ThreadMom {
public:
	void reset(int amountOfThreads) {
		this->amountOfThreads = amountOfThreads - 1;
		dying = true;
		for (int i = 0; i < threads.size(); i++) {
			threadGo[i].post();
			threads[i].join();
		}
		dying = false;

		threads.clear();

		freeThreads.clear();
		freeThreads = std::vector<Semaphore>(this->amountOfThreads);

		threadGo.clear();
		threadGo = std::vector<Semaphore>(this->amountOfThreads);

		threadFin.clear();
		threadFin = std::vector<Semaphore>(this->amountOfThreads);


		threadAcceptFin.clear();
		threadAcceptFin = std::vector<Semaphore>(this->amountOfThreads);

		threadData_float.clear();
		threadData_float = std::vector<Data_float>(this->amountOfThreads);

		if (choice == 1) {
			fft_choice_method = pFft_CPP;
		}
		else {
			fft_choice_method = pFft_ASM;
		}

		for (int i = 0; i < this->amountOfThreads; i++)
		{
			freeThreads[i].post();
			threads.push_back(std::thread(
				&ThreadMom::threadMethod, this, i)
			);

		}

	}
	ThreadMom(int amountOfThreads, 
		void (*pFft_ASM)(float* real, float* imag, int N, ThreadMom& tm),
		void (*pFft_CPP)(float* real, float* imag, int N, ThreadMom& tm)
	) :amountOfThreads(amountOfThreads), dying(false) {
		this->pFft_ASM = pFft_ASM;
		this->pFft_CPP = pFft_CPP;
		choice = 1;
		reset(amountOfThreads);
	}
	~ThreadMom() {
		dying = true;
		for (int i = 0; i < threads.size(); i++) {
			threadGo[i].post();
			threads[i].join();
		}
	}
	Response askForHelp_float(Data_float data)
	{
		for (int i = 0; i < amountOfThreads; i++)
		{
			//check if thread[i] is free
			if (freeThreads[i].try_wait())
			{
				//fill container of thread[i] 
				threadData_float[i] = data;
				//start thread
				threadGo[i].post();
				//return handles to communicate with the thread
				//(check if finished, accept)
				return { &threadFin[i], &threadAcceptFin[i] };
			}
		}
		return Response{ nullptr, nullptr };
	}

	void setChoice(int x) {
		choice = x;
		reset(amountOfThreads+1);
	}


private:
	void (*pFft_ASM)(float* real, float* imag, int N, ThreadMom& tm);
	void (*pFft_CPP)(float* real, float* imag, int N, ThreadMom& tm);
	void (*fft_choice_method)(float* real, float* imag, int N, ThreadMom& tm);
	bool dying;
	int amountOfThreads;
	int choice;
	std::vector<Semaphore> freeThreads;
	std::vector<Semaphore> threadGo;
	std::vector<Semaphore> threadFin;
	std::vector<Semaphore> threadAcceptFin;
	std::vector<std::thread> threads;
	std::vector<Data_float> threadData_float;

	void threadMethod(int id)
	{
		while (1)
		{
			//wait to be summoned
			threadGo[id].wait();

			//check if the program is not shutting down
			if (dying == true) {
				return;
			}

			//call fft method chosen by the user (C++ vs ASM)
			fft_choice_method(threadData_float[id].real, threadData_float[id].imag, threadData_float[id].size, *this);
			
			//signal calculation completion
			threadFin[id].post();

			//wait for accept
			threadAcceptFin[id].wait();

			//signal thread is free for next calculation
			freeThreads[id].post();
		}
	}

};
