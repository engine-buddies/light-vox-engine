#pragma once
#include <Windows.h>
class Time
{
public:
	Time();
	void UpdateTimer();
	~Time();

private:
	__int64 startTime;
	__int64 currentTime;
	__int64 previousTime;

	float totalTime;
	float deltaTime;

};

