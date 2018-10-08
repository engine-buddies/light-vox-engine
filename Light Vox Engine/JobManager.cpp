#include "JobManager.h"

// Singleton requirement
JobManager* JobManager::Instance = nullptr;

// private constructor
JobManager::JobManager()
{
    unsigned int supportedThreads = std::thread::hardware_concurrency();
    printf( "The number of threads supported on this system is: %d\n", supportedThreads );

    // Create the worker threads 

    // Create the Job Queue here if we need to

}

// Private destructor
JobManager::~JobManager()
{
    // TODO: Any cleanup of worker threads here
    printf( "\tJob Manager dtor!\n" );
}


JobManager* JobManager::GetInstance()
{
    if ( Instance == nullptr)
    {
        Instance = new JobManager();
    }
    return Instance;
}

void JobManager::ReleaseInstance()
{
    if ( Instance != nullptr )
    {
        delete Instance;
        Instance = nullptr;
    }
}