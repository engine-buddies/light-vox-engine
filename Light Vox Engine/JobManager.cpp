#include "JobManager.h"

// Singleton requirement
JobManager* JobManager::instance = nullptr;

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
    printf( "The number of threads supported on this system is: %d\n", supportedThreads );

    isDone = false;

    // Create worker threads that check to see if there is any work to do
    for ( unsigned int i = 0; i < supportedThreads; ++i )
    {
        workerThreads.push_back( std::thread( &JobManager::WorkerThread, this ) );
    }

    printf( "Created %d worker threads!\n", (int) workerThreads.size() );

    CpuJob testFuncA {};
    testFuncA.func_ptr = &TestJob;

    CpuJob testFuncB {};
    testFuncB.func_ptr = &TestJobB;

    CpuJob testFuncC {};
    testFuncC.func_ptr = &TestJob;

    AddJob( testFuncA );
    // Add a test job to the ready queue
    AddJob( testFuncB );
    // Add a test job to the ready queue
    AddJob( testFuncC );

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

// Temporary add job function
void JobManager::AddJob( CpuJob aJob )
{
    readyQueue.emplace_back( aJob );

    // Let one of the worker threads know that there is a job available
    jobAvailableCondition.notify_one();
}

void JobManager::AddJob( void( *func_ptr )( void *aArgs, int index ) )
{
    CpuJob tempJob { };
    
    tempJob.func_ptr = func_ptr;
    //tempJob.args = aArgs;

    readyQueue.emplace_back( tempJob );

    // Let one of the worker threads know that there is a job available
    jobAvailableCondition.notify_one();

}

void JobManager::WorkerThread()
{
    std::unique_lock<std::mutex> workerLock ( readyQueueMutex );
    printf( "\tEnter Job Thread! \n" );

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
        }
    }

    printf( "\t\tEXIT Job Thread! \n" );
}

////////////////////////////////////////
// Accessors

inline const size_t JobManager::GetThreadCount() const
{
    return workerThreads.size();
}