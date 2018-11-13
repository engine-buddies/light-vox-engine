#pragma once
#include "stdafx.h"

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
    double GetTotalDoubleTime();
    /// <summary>
    /// Return delta time as double
    /// </summary>
    double GetDeltaDoubleTime();
    /// <summary>
    /// return total time (time since starting program) as float 
    /// </summary>
    float GetTotalFloatTime();
    /// <summary>
    /// Return delta time as float
    /// </summary>
    float GetDeltaFloatTime();

private:
    static GameTime* instance;

    double startTime;
    double currentTime;
    double previousTime;

    double totalDoubleTime;
    double deltaDoubleTime;
    double perfCounterSeconds;

    float totalFloatTime;
    float deltaFloatTime;
};

