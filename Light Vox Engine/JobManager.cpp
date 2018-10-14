#include "JobManager.h"

using namespace Jobs;


// Singleton requirement
JobManager* JobManager::instance = nullptr;

void TestJob( void* myTestArgs, int myTestIndex )
{
#ifdef _DEBUG
    printf( "Test job here bub: %s \n", (char*) myTestArgs );
#endif
}

void TestJobB( void* myTestArgs, int myTestIndex )
{
#ifdef _DEBUG
    printf( "Test job TWOOOOO here bub: %s \n", (char*) myTestArgs );
#endif
}

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

#ifdef _DEBUG
    printf( "The number of threads supported on this system is: %d\n", supportedThreads );
#endif // DEBUG

    isDone = false;

    // Create worker threads that check to see if there is any work to do
    for ( size_t i = 0; i < supportedThreads; ++i )
    {
        workerThreads.push_back( std::thread( &JobManager::WorkerThread, this ) );
    }
#ifdef _DEBUG
    printf( "Created %d worker threads!\n", (int) workerThreads.size() );


    AddJob( &TestJob, "Test RAgs for Job A 1" );
    AddJob( &TestJobB, "test args for B 2 " );
    AddJob( &TestJobB, "test args for B 3 " );
    AddJob( &TestJob, "test args for A 4 " );
    AddJob( &TestJob, "test args for A 5 " );
    AddJob( &TestJob, "test args for A 6 " );
    AddJob( &TestJob, "test args for A 7 " );
    


#endif // DEBUG
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

void JobManager::AddJob( void( *func_ptr )( void *aArgs, int index ), void* jobArgs )
{
    CpuJob tempJob { };
    
    tempJob.func_ptr = func_ptr;
    tempJob.args = jobArgs;

    readyQueue.emplace_back( tempJob );

    // Let one of the worker threads know that there is a job available
    jobAvailableCondition.notify_one();

}

void JobManager::WorkerThread()
{
    std::unique_lock<std::mutex> workerLock ( readyQueueMutex );

    while ( true )
    {
        // Wait for a job to become available
        jobAvailableCondition.wait( workerLock );

        // Make sure that we don't need to be done now!
        if ( isDone ) return;

        // If there is a job available, than work on it
        if ( !readyQueue.empty() )
        {
            CpuJob CurJob;
            readyQueue.pop_front( CurJob );
            
            CurJob.func_ptr( CurJob.args, CurJob.index );

            // Notify other threads that a job has been taken and we should probably
            // check to make sure that there isn;t more
            jobAvailableCondition.notify_one();
        }
    }
#ifdef _DEBUG
    printf( "\t\tEXIT Job Thread! \n" );
#endif
}


////////////////////////////////////////
// Accessors

inline const size_t JobManager::GetThreadCount() const
{
    return workerThreads.size();
}