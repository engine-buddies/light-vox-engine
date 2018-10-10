#include "JobManager.h"

void TestJob( void* myTestArgs, int myTestIndex )
{
    printf( "Test job here bub: %s \n", (char*) myTestArgs );
}

JobManager::JobManager()
{
    unsigned int supportedThreads = std::thread::hardware_concurrency();
    printf( "The number of threads supported on this system is: %d\n", supportedThreads );

    // Create the Job Queue here if we need to

    // test a function here and see if my work threads are up
    ReadyQueue.emplace_back( &TestJob );

    // Create worker threads that check to see if there is any work to do
    for ( unsigned int i = 0; i < supportedThreads; ++i )
    {
        WorkerThreads.push_back( std::thread( &JobManager::AskForWork, this ) );
    }

    printf( "Created %d worker threads!\n", (int) WorkerThreads.size() );

}

// Private destructor
JobManager::~JobManager()
{
    lock_t lock( ReadyQueueMutex );

    // TODO: Any cleanup of worker threads here
    
    SignalDone();

    lock.unlock();

    // Ensure that all jobs are done (by joining the thread, 
    // thus waiting for all its work to be done)
    for ( auto &item : WorkerThreads )
    {
        item.join();
    }

    printf( "\tJob Manager dtor!\n" );

}

// Temporary add job function
void JobManager::AddJob( int aIndex )
{
    lock_t lock { ReadyQueueMutex };

    ReadyQueue.emplace_back( &TestJob );

    Condition_m.notify_one();
}

void JobManager::SignalDone()
{
    if ( IsDone.exchange( true ) )
    {
        return;
    }

    Condition_m.notify_all();
}

void JobManager::AskForWork()
{
    printf( "\tEnter Job Thread! \n" );

   /* for ( int i = 0; ; ++i )
    {        
        lock_t lock { ReadyQueueMutex };

        Condition_m.wait( lock, [ = ]()
            { 
                return IsDone || !empty(); 
            } 
        );

        // Return if the manager has been marked as done
        if ( IsDone ) return;
        
        // Try Pop Unsafe here

        if ( !ReadyQueue.empty() )
        {
            CpuJob myjob = ReadyQueue.back();            
            ReadyQueue.pop_front();

            myjob( "Hello this is a job BOI\n", 1 );
        }
    }
    */
    printf( "\t\tEXIT Job Thread! \n" );
}


////////////////////////////////////////
// Accessors

inline const size_t JobManager::GetThreadCount() const
{
    return WorkerThreads.size();
}