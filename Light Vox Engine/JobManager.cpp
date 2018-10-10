#include "JobManager.h"

void TestJob( void* myTestArgs, int myTestIndex )
{
    printf( "Test job here bub: %s \n", (char*) myTestArgs );
}

void TestJobB( void* myTestArgs, int myTestIndex )
{
    printf( "Test job TWOOOOO here bub: %s \n", (char*) myTestArgs );
}


JobManager::JobManager()
{
    const unsigned int supportedThreads = boost::thread::hardware_concurrency();

#if defined(_DEBUG) || defined(DEBUG)
    printf( "The number of threads supported on this system is: %d\n", supportedThreads );
#endif
    
    IsDone = false;

    // Create worker threads that check to see if there is any work to do
    for ( unsigned int i = 0; i < supportedThreads; ++i )
    {
        WorkerThreads.push_back( boost::thread( &JobManager::WorkerThread, this ) );
    }

#if defined(_DEBUG) || defined(DEBUG)
    printf( "Created %d worker threads!\n", (int) WorkerThreads.size() );
#endif

    CpuJob testFuncA { };
    testFuncA.priority = 1;
    testFuncA.func_ptr = &TestJob;


    CpuJob testFuncB { };
    testFuncA.priority = 1;
    testFuncA.func_ptr = &TestJobB;

    CpuJob testFuncC { };
    testFuncA.priority = 1;
    testFuncA.func_ptr = &TestJob;


    // Add a test job to the ready queue
    AddJob( testFuncA );

    // Add a test job to the ready queue
    AddJob( testFuncB );

    // Add a test job to the ready queue
    AddJob( testFuncC );

}

JobManager::~JobManager()
{
    IsDone = true;

    // Notify all threads that they should end
    JobAvailable.notify_all();

    // Ensure that all jobs are done (by joining the thread, 
    // thus waiting for all its work to be done)
    for ( auto &item : WorkerThreads )
    {
        item.join();
    }
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
    printf( "\tEnter Worker Thread! \n" );

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
            
            // Current bug here about not being a way to access something
            // Unsure what is goin on at this moment in time

            //CpuJob CurJob = ReadyQueue.wait_pop_front();
            
            //CurJob.func_ptr("Test Function Pointers", 1);
        }
    }


    printf( "\t\tEXIT Worker Thread! \n" );
}

////////////////////////////////////////
// Accessors

inline const size_t JobManager::GetThreadCount() const
{
    return WorkerThreads.size();
}