#include "Time.h"

Time* Time::instance = nullptr;

Time * Time::GetInstance()
{
	if (instance == nullptr)
		instance = new Time();

	return instance;
}

void Time::ReleaseInstance()
{
	if(instance != nullptr)
		delete instance;
}

void Time::UpdateTimer()
{
	__int64 now;
	QueryPerformanceCounter((LARGE_INTEGER*)&now);
	currentTime = now;

	//Calc delta time and clamp to zero
	//Could go negitive if CPU goes into power saver mode
	//of the process itself gets moves to another core
	deltaTime = max((float)((currentTime - previousTime) * perfCounterSeconds), 0.0f);
	totalTime = (float)((currentTime - startTime) * perfCounterSeconds);

	previousTime = currentTime;
}

void Time::Init()
{
	//Query preformance counter for accurate timing information
	__int64 perfFreq;
	QueryPerformanceFrequency((LARGE_INTEGER*)&perfFreq);
	perfCounterSeconds = 1.0 / (double)perfFreq;

	__int64 now;
	QueryPerformanceCounter((LARGE_INTEGER*)&now);
	startTime = now;
	currentTime = now;
	previousTime = now;
}

float Time::getTotalTimeFloat()
{
	return totalTime;
}

float Time::getDeltaTimeFloat()
{
    return deltaTime;
}