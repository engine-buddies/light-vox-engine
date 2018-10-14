#include "JobManager.h"

// Singleton requirement
JobManager* JobManager::Instance = nullptr;

void TestJob( void* myTestArgs, int myTestIndex )
{
    printf( "Test job here bub: %s \n", (char*) myTestArgs );
}

void TestJobB( void* myTestArgs, int myTestIndex )
{
    printf( "Test job TWOOOOO here bub: %s \n", (char*) myTestArgs );
}

JobManager* JobManager::GetInstance()
{
    if ( Instance == nullptr ) 
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

JobManager::JobManager()
{
    const unsigned int supportedThreads = std::thread::hardware_concurrency();
    printf( "The number of threads supported on this system is: %d\n", supportedThreads );

    IsDone = false;



    // Create worker threads that check to see if there is any work to do
    for ( unsigned int i = 0; i < supportedThreads; ++i )
    {
        WorkerThreads.push_back( std::thread( &JobManager::WorkerThread, this ) );
    }

    printf( "Created %d worker threads!\n", (int) WorkerThreads.size() );

    CpuJob testFuncA {};
    testFuncA.priority = 1;
    testFuncA.func_ptr = &TestJob;

    CpuJob testFuncB {};
    testFuncB.priority = 2;
    testFuncB.func_ptr = &TestJobB;

    CpuJob testFuncC {};
    testFuncC.priority = 3;
    testFuncC.func_ptr = &TestJob;

    AddJob( testFuncA );
    // Add a test job to the ready queue
    AddJob( testFuncB );
    // Add a test job to the ready queue
    AddJob( testFuncC );

}

JobManager::~JobManager()
{
    IsDone = true;

    JobAvailable.notify_all();

    // Ensure that all jobs are done (by joining the thread, 
    // thus waiting for all its work to be done)
    for ( auto &item : WorkerThreads )
    {
        item.join();
    }

    printf( "\tJob Manager dtor!\n" );
}

// Temporary add job function
void JobManager::AddJob( CpuJob aJob )
{
    ReadyQueue.emplace_back( aJob );

    // Let one of the worker threads know that there is a job available
    JobAvailable.notify_one();
}

void JobManager::WorkerThread()
{
    std::unique_lock<std::mutex> workerLock ( ReadyQueueMutex );
    printf( "\tEnter Job Thread! \n" );

    while ( true )
    {
        // Wait for a job to become available
        JobAvailable.wait( workerLock );

        // Make sure that we don't need to be done now!
        if ( IsDone ) return;

        // If there is a job available, than work on it
        if ( !ReadyQueue.empty() )
        {
            CpuJob CurJob;
            ReadyQueue.pop_front( CurJob );
            
            CurJob.func_ptr( "Test boy hey there", CurJob.priority );
        }
    }

    printf( "\t\tEXIT Job Thread! \n" );
}

////////////////////////////////////////
// Accessors

inline const size_t JobManager::GetThreadCount() const
{
    return WorkerThreads.size();
}