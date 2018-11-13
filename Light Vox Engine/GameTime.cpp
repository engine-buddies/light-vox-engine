#include "GameTime.h"

GameTime* GameTime::instance = nullptr;

GameTime* GameTime::GetInstance()
{
    if ( instance == nullptr )
        instance = new GameTime();

    return instance;
}

void GameTime::ReleaseInstance()
{
    if ( instance != nullptr )
        delete instance;
}

void GameTime::UpdateTimer()
{
    __int64 now;
#if defined(_WIN32) || defined(_WIN64)
    QueryPerformanceCounter( (LARGE_INTEGER*) &now );
#else
    static_assert( false, "Unimplemented alternative to 'QueryPerformanceFrequency'" );
#endif
    currentTime = static_cast<double>( now );

    //Calc delta time and clamp to zero
    //Could go negitive if CPU goes into power saver mode
    //or the process itself gets moves to another core
    deltaDoubleTime = glm::max( ( ( currentTime - previousTime ) * perfCounterSeconds ), 0.0 );
    totalDoubleTime = ( ( currentTime - startTime ) * perfCounterSeconds );
    totalFloatTime = static_cast<float>( totalDoubleTime );
    deltaFloatTime = static_cast<float>( deltaDoubleTime );

    previousTime = currentTime;

}

void GameTime::Init()
{
    //Query preformance counter for accurate timing information
    __int64 perfFreq;
#if defined(_WIN32) || defined(_WIN64)
    QueryPerformanceFrequency( (LARGE_INTEGER*) &perfFreq );
#else
    static_assert( false, "Unimplemented alternative to 'QueryPerformanceFrequency'" );
#endif

    perfCounterSeconds = 1.0 / (double) perfFreq;

    __int64 now;
#if defined(_WIN32) || defined(_WIN64)
    QueryPerformanceCounter( (LARGE_INTEGER*) &now );
#else
    static_assert( false, "Unimplemented alternative to 'QueryPerformanceFrequency'" );
#endif

    double doubleNow = static_cast<double>( now );
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

float GameTime::GetTotalFloatTime()
{
    return totalFloatTime;
}

float GameTime::GetDeltaFloatTime()
{
    return deltaFloatTime;
}
