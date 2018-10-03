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
	currentTime = (double)now;

	//Calc delta time and clamp to zero
	//Could go negitive if CPU goes into power saver mode
	//or the process itself gets moves to another core
	deltaTime = max(((currentTime - previousTime) * perfCounterSeconds), 0.0f);
	totalTime = ((currentTime - startTime) * perfCounterSeconds);

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
	startTime = now;
	currentTime = now;
	previousTime = now;
}

double GameTime::GetTotalTime()
{
	return totalTime;
}

double GameTime::GetDeltaTime()
{
    return deltaTime;
}