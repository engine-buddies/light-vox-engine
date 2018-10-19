#include "JobManager.h"

using namespace Jobs;

void TestBoi()
{
    printf( "TEST BOIIII \n" );

}

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
    const unsigned int supportedThreads = std::thread::hardware_concurrency();


    DEBUG_PRINT( "The number of threads supported on this system is: %d\n", supportedThreads );

    isDone = false;

    // Create worker threads that check to see if there is any work to do
    for ( size_t i = 0; i < supportedThreads; ++i )
    {
        workerThreads.push_back( std::thread( &Jobs::JobManager::WorkerThread, this ) );
    }

    // Ohhhhhh yee
    AddJob_Generic( std::bind( &TestBoi ) );

    AddJob_Generic( std::bind( &Jobs::JobManager::TestBoiMember, this ) );

}

void JobManager::TestBoiMember()
{
    printf( "\tTEST MEMMMBER\n\n" );
}

JobManager::~JobManager()
{
    isDone = true;

    jobAvailableCondition.notify_all();

    // Ensure that all jobs are done (by joining the thread, 
    // thus waiting for all its work to be done)
    for ( auto &item : workerThreads )
    {
        item.join();
    }

    printf( "\tJob Manager dtor!\n" );
}

void Jobs::JobManager::AddJob( std::function<void( void*, int )> aFunc, void* jobArgs )
{
    CpuJob tempJob { };

    tempJob.job_func = aFunc;
    tempJob.args = jobArgs;

    readyQueue.emplace_back( tempJob );

    jobAvailableCondition.notify_one();
}

void JobManager::WorkerThread()
{
    std::unique_lock<std::mutex> workerLock( readyQueueMutex );

    while ( true )
    {
        // Wait for a job to become available
        jobAvailableCondition.wait( workerLock );

        // Make sure that we don't need to be done now!
        if ( isDone ) return;

        // If there is a job available, than work on it
        /*if ( !readyQueue.empty() )
        {
            CpuJob CurJob;
            readyQueue.pop_front( CurJob );

            CurJob.job_func( CurJob.args, CurJob.index );

            // Notify other threads that a job has been taken and we should probably
            // check to make sure that there isn;t more
            jobAvailableCondition.notify_one();
        }*/

        if ( !jobQueue.empty() )
        {
            job_t curJob;
            jobQueue.pop_front( curJob );
            printf( "Inside worky" );
            curJob();
        }
    }
}


////////////////////////////////////////
// Accessors

inline const size_t JobManager::GetThreadCount() const
{
    return workerThreads.size();
}