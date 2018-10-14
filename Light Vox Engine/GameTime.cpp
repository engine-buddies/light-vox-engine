#include "GameTime.h"

GameTime* GameTime::instance = nullptr;

GameTime* GameTime::GetInstance()
{
	if (instance == nullptr)
		instance = new GameTime();

	return instance;
}

void GameTime::ReleaseInstance()
{
	if (instance != nullptr)
		delete instance;
}

void GameTime::UpdateTimer()
{
	__int64 now;
	QueryPerformanceCounter((LARGE_INTEGER*)&now);
	currentTime = static_cast<double>(now);

	//Calc delta time and clamp to zero
	//Could go negitive if CPU goes into power saver mode
	//or the process itself gets moves to another core
	deltaDoubleTime = max(((currentTime - previousTime) * perfCounterSeconds), 0.0f);
	totalDoubleTime = ((currentTime - startTime) * perfCounterSeconds);
    totalFloatTime = static_cast<float>(totalDoubleTime);
    deltaFloatTime = static_cast<float>(deltaDoubleTime);

	previousTime = currentTime;

}

void GameTime::Init()
{
	//Query preformance counter for accurate timing information
	__int64 perfFreq;
	QueryPerformanceFrequency((LARGE_INTEGER*)&perfFreq);
	perfCounterSeconds = 1.0 / (double)perfFreq;

	__int64 now;
	QueryPerformanceCounter((LARGE_INTEGER*)&now);
    double doubleNow = static_cast<double>(now);
	startTime = doubleNow;
	currentTime = doubleNow;
	previousTime = doubleNow;
}

double GameTime::GetTotalDoubleTime()
{
	return totalDoubleTime;
}

double GameTime::GetDeltaDoubleTime()
{
    return deltaDoubleTime;
}

double GameTime::GetTotalFloatTime()
{
    return totalFloatTime;
}

double GameTime::GetDeltaFloatTime()
{
    return deltaFloatTime;
}
