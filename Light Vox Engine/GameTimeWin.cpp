#include "stdafx.h"
#include "GameTime.h"

void GameTime::PerformanceFrequency( int64_t & perfFreq )
{
    QueryPerformanceFrequency( (LARGE_INTEGER*) &perfFreq );
}

void GameTime::PerformanceCounter( int64_t & now )
{
    QueryPerformanceCounter( (LARGE_INTEGER*) &now );
}
