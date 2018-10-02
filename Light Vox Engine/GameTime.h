#pragma once
#include <Windows.h>
class GameTime
{
public:
	/// <summary>
	/// Gets singleton object of time
	/// </summary>
	static GameTime* GetInstance();
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
	/// return total time (time since starting program) as double 
	/// </summary>
    double getTotalTime();
	/// <summary>
	/// Return delta time as double
	/// </summary>
	double getDeltaTime();

private:
	static GameTime* instance;

	__int64 startTime;
	__int64 currentTime;
	__int64 previousTime;

	double totalTime;
	double deltaTime;
	double perfCounterSeconds;

};

