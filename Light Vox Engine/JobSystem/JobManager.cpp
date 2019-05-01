#include "../stdafx.h"
#include "JobManager.h"

thread_local Job JobManager::g_jobAllocator[ MAX_JOB_COUNT ];
thread_local uint32_t JobManager::g_allocatedJobs = 0u;

JobManager JobManager::Instance = {};
moodycamel::ConcurrentQueue<Job*> JobManager::locklessReadyQueue;
std::vector<std::thread> JobManager::WorkerThreads;

void JobManager::Startup()
{
    IsDone = false;

    // Create a thread pool to the worker thread
    const unsigned int threadCount = GetAmountOfSupportedThreads();

    for ( size_t i = 0; i < threadCount; ++i )
    {
        WorkerThreads.push_back( std::thread( &JobManager::WorkerThread, this ) );
    }

}

void JobManager::Shutdown()
{
    IsDone = true;

    for ( auto &item : WorkerThreads )
    {
        item.join();
    }
}

Job * JobManager::CreateJob( JobFunction aFunction, void* args, size_t aSize )
{
    assert( aSize >= 0 && aSize < JOB_DATA_PADDING_SIZE );

    Job* job = AllocateJob();
    job->Function = aFunction;
    job->Parent = nullptr;
    job->UnfinishedJobs.store( 1 );

    // Memcpy the args to the jobs padding
    if ( args != nullptr )
    {
        memcpy( job->Padding, args, aSize );
    }

    return job;
}

Job * JobManager::CreateJobAsChild( Job * aParent, JobFunction aFunction, void* args, size_t aSize )
{
    assert( aSize >= 0 && aSize < JOB_DATA_PADDING_SIZE );

    // Keep track of the number of jobs on the parent
    aParent->UnfinishedJobs.fetch_add( 1 );

    // Create a new job
    Job* job = AllocateJob();
    job->Function = aFunction;
    job->Parent = aParent;
    job->UnfinishedJobs.store( 1 );

    // Memcpy the args to the jobs padding
    if ( args != nullptr )
    {
        memcpy( job->Padding, args, aSize );
    }

    return job;
}

void JobManager::Run( Job * aJob )
{
    assert( aJob != nullptr );
    locklessReadyQueue.enqueue( aJob );
}

void JobManager::Wait( const Job * aJob )
{
    // Wait until this job has completed
    // in the meantime, work on another job
    while ( !HasJobCompleted( aJob ) )
    {
        Job* nextJob = GetJob();
        if ( nextJob )
        {
            Execute( nextJob );
        }
    }
}

const unsigned int JobManager::GetAmountOfSupportedThreads()
{
    return std::thread::hardware_concurrency();
}

void JobManager::WorkerThread()
{
    while ( !IsDone )
    {
        Job* job = GetJob();
        if ( job )
        {
            Execute( job );
        }
    }
}

void JobManager::YieldWorker()
{
    // For now, simply sleep this thread to yield its time
    std::this_thread::sleep_for( std::chrono::milliseconds( 3 ) );
}

Job * JobManager::GetJob()
{
    Job* CurJob = nullptr;
    bool found = locklessReadyQueue.try_dequeue( CurJob );
    if ( found )
    {
        return CurJob;
    }
    else
    {
        // TODO: Try and steal a job

        // We couldn't find out job, 
        YieldWorker();
        return nullptr;
    }

    return nullptr;
}

bool JobManager::HasJobCompleted( const Job * aJob )
{
    // A job is done if there is no more unfinished work
    return ( aJob->UnfinishedJobs <= 0 );
}

void JobManager::Execute( Job * aJob )
{
    assert( aJob != nullptr );
    // Call the jobs function
    ( aJob->Function )( aJob, aJob->Padding );
    // Finish the job when it is done
    Finish( aJob );
}

Job * JobManager::AllocateJob()
{
    const uint32_t index = g_allocatedJobs++;
    return &g_jobAllocator[ index & ( MAX_JOB_COUNT - 1u ) ];
    //return &g_jobAllocator[ ( index - 1u ) & ( MAX_JOB_COUNT - 1u ) ];
}

void JobManager::Finish( Job * aJob )
{
    // Decrement the jobs unfinished job count
    const int32_t unfinishedJobs = --aJob->UnfinishedJobs;

    // If this job is done, then let the jobs parent know
    if ( ( unfinishedJobs == 0 ) && ( aJob->Parent ) )
    {
        Finish( aJob->Parent );
    }
}