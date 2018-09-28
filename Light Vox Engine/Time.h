#pragma once
#include <Windows.h>
class Time
{
public:
	/// <summary>
	/// Gets singleton object of time
	/// </summary>
	static Time* GetInstance();
	/// <summary>
	/// Releases singleton object of time
	/// </summary>
	void ReleaseInstance();
	/// <summary>
	/// Intializes preformance counter, current time, previous time
	/// </summary>
	void Init();
	/// <summary>
	/// Updates the time 
	/// </summary>
	void UpdateTimer();
	/// <summary>
	/// Get total time since starting 
	/// </summary>
	float getTotalTimeFloat();
	/// <summary>
	/// Get delta time 
	/// </summary>
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

