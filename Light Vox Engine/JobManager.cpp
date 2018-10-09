#include "JobManager.h"

void TestJob( void* myTestArgs, int myTestIndex )
{
    printf( "Test job here bub: %s \n", (char*) myTestArgs );
}

void PleaseWork( int index )
{
    ///printf( "Please work here bub: %d \n", index );
    int x = 10;
}

JobManager::JobManager()
{
    unsigned int supportedThreads = std::thread::hardware_concurrency();
    printf( "The number of threads supported on this system is: %d\n", supportedThreads );

    IsDone = false;

    // Create the Job Queue here if we need to
    
    // test a function here and see if my work threads are up
    testJobBoi = &TestJob;
    JobQueue.push( testJobBoi );

    // Create worker threads
    for ( unsigned int i = 0; i < supportedThreads; ++i )
    {
        WorkerThreads.push_back( std::thread( &JobManager::AskForWork, this ) );
        //WorkerThreads.push_back( move( t ) );
    }

    printf( "Created %d worker threads!\n", (int)WorkerThreads.size() );

}

// Private destructor
JobManager::~JobManager()
{
    // TODO: Any cleanup of worker threads here
    printf( "\tJob Manager dtor!\n" );
    
    // Ensure that all jobs are done, and then free up 
    // any resources we may have allocated here
    for ( auto &item : WorkerThreads )
    {
        item.detach();
    }

    IsDone = true;
}

void JobManager::AskForWork()
{
    //printf( "Ask me for work dude" );
    int i = 0;
   /* printf("This is asking for works\n");
    while ( !IsDone )
    {
        // Ask for work
        if ( !JobQueue.empty() )
        {
            CpuJob myjob = JobQueue.back();
            JobQueue.pop();
            // I would get the args from the job sequence!
            // I think I will need a data structure to define 
            // the args and stuff and use that in the queue instead of 
            // just the little boi up there
            myjob( "Test in Worker!", 1 );
        }
    }*/
}


////////////////////////////////////////
// Accessors

inline const bool JobManager::GetIsDone() const
{
    return IsDone;
}

inline const size_t JobManager::GetThreadCount() const
{
    return WorkerThreads.size();
}