#include "JobManager.h"

void TestJob( void* myTestArgs, int myTestIndex )
{
    printf( "Test job here bub: %s \n", (char*) myTestArgs );
}



JobManager::JobManager()
{
    const unsigned int supportedThreads = boost::thread::hardware_concurrency();
    printf( "The number of threads supported on this system is: %d\n", supportedThreads );

    IsDone = false;

    // Create worker threads that check to see if there is any work to do
    for ( unsigned int i = 0; i < supportedThreads; ++i )
    {
        WorkerThreads.push_back( boost::thread( &JobManager::WorkerThread, this ) );
    }

    printf( "Created %d worker threads!\n", (int) WorkerThreads.size() );

    // Add a test job to the ready queue
    AddJob( &TestJob );

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
    boost::mutex::scoped_lock workerLock ( ReadyQueueMutex );
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
            printf( "** THere is a job in there dude!\n" );
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