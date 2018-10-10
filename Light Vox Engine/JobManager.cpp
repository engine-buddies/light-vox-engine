#include "JobManager.h"

void TestJob( void* myTestArgs, int myTestIndex )
{
    printf( "Test job here bub: %s \n", (char*) myTestArgs );
}



JobManager::JobManager()
{
    const unsigned int supportedThreads = boost::thread::hardware_concurrency();
    printf( "The number of threads supported on this system is: %d\n", supportedThreads );

    // Add a test job to the ready queue
    AddJob( &TestJob );

    // Create worker threads that check to see if there is any work to do
    for ( unsigned int i = 0; i < supportedThreads; ++i )
    {
        WorkerThreads.push_back( boost::thread( &JobManager::WorkerThread, this ) );
    }

    printf( "Created %d worker threads!\n", (int) WorkerThreads.size() );

}

// Private destructor
JobManager::~JobManager()
{
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
}


void JobManager::WorkerThread()
{
    printf( "\tEnter Job Thread! \n" );

    // I want to sleep the thread if no jobs exist
    
    printf( "\t\tEXIT Job Thread! \n" );
}

////////////////////////////////////////
// Accessors

inline const size_t JobManager::GetThreadCount() const
{
    return WorkerThreads.size();
}