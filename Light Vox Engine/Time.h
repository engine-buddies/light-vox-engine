#pragma once
#include <Windows.h>
class Time
{
public:
	static Time* GetInstance();
	void ReleaseInstance();
	void Init();
	void UpdateTimer();
	float getTotalTimeFloat();
	float getDeltaTimeFloat();

private:
	static Time* instance;

	__int64 startTime;
	__int64 currentTime;
	__int64 previousTime;

	float totalTime;
	float deltaTime;
	double perfCounterSeconds;

};

