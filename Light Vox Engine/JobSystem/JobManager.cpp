#include "../stdafx.h"
#include "JobManager.h"

using namespace Jobs;

// Singleton requirement
JobManager* JobManager::instance = nullptr;

JobManager* JobManager::GetInstance()
{
    if ( instance == nullptr )
    {
        instance = new JobManager();
    }
    return instance;
}

void JobManager::ReleaseInstance()
{
    if ( instance != nullptr )
    {
        delete instance;
        instance = nullptr;
    }
}

JobManager::JobManager()
{
    const unsigned int supportedThreads = GetAmountOfSupportedThreads();

    DEBUG_PRINT( "The number of threads supported on this system is: %d\n", supportedThreads );

    isDone = false;

    // Create worker threads that check to see if there is any work to do
    for ( size_t i = 0; i < supportedThreads; ++i )
    {
        workerThreads.push_back( std::thread( &Jobs::JobManager::WorkerThread, this ) );
    }
 }

JobManager::~JobManager()
{
    isDone = true;

    // Ensure that all jobs are done (by joining the thread, 
    // thus waiting for all its work to be done)
    for ( auto &item : workerThreads )
    {
        item.join();
    }
}

void JobManager::AddJob( void( *func_ptr )( void *, int ), void * args, int Index )
{
    CpuJob aJob;
    aJob.jobArgs = args;
    aJob.index = Index;

    IJob* jobPtr = new JobFunc( func_ptr );
    aJob.jobPtr = jobPtr;

    locklessReadyQueue.enqueue( aJob );
}

void JobManager::WorkerThread()
{
    while ( true )
    {
        // Make sure that we don't need to be done now!
        if ( isDone ) return;

        CpuJob CurJob;
        bool found = locklessReadyQueue.try_dequeue( CurJob );

        if ( found )
        {
            if ( CurJob.jobPtr )
            {
                CurJob.jobPtr->invoke( CurJob.jobArgs, CurJob.index );
                // #TODO
                // make this a pooled resource
                delete CurJob.jobPtr;
            }
        }
        else
        {
            std::this_thread::sleep_for( std::chrono::milliseconds( 3 ) );
        }
    }
}

////////////////////////////////////////
// Accessors

inline const size_t JobManager::GetThreadCount() const
{
    return workerThreads.size();
}

inline int JobManager::GetAmountOfSupportedThreads()
{
    return std::thread::hardware_concurrency();
}
