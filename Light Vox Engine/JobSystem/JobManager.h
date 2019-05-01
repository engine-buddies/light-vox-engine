#pragma once
#include <thread>
#include <vector>
#include <atomic>
#include <assert.h>

#include "Job.h"
#include "concurrentqueue.h"    // lock-less queue

/// <summary>
/// Job 
/// </summary>
/// <author>Ben Hoffman</author>
class JobManager
{
public:

    /// <summary>
    /// Creates thread pool
    /// </summary>
    void Startup();

    /// <summary>
    /// Blocking function, joins all worker threads and waits for work to be
    /// done
    /// </summary>
    void Shutdown();

    static JobManager Instance;

    /// <summary>
    /// Creates a job without a parent.
    /// DOES NOT run the job. To start it, you must call Run.
    /// </summary>
    /// <param name="aFunction">Function to call as a job</param>
    /// <param name="args">the arguments to pass into the job. Must fit inside
    /// the jobs padding buffer</param>
    /// <returns>Pointer to the job </returns>
    Job* CreateJob( JobFunction aFunction, void* args, size_t aSize );

    /// <summary>
    /// Creates a job as a child of another. 
    /// DOES NOT run the job. To start it, you must call Run.
    /// </summary>
    /// <param name="aParent"></param>
    /// <param name="aFunction"></param>
    /// <returns></returns>
    Job* CreateJobAsChild( Job* aParent, JobFunction aFunction, void* args, size_t aSize );

    /// <summary>
    /// Run the given job in the system, pushing it to the 
    /// work queue
    /// </summary>
    /// <param name="aJob">Job to run</param>
    void Run( Job* aJob );

    /// <summary>
    /// Wait for the given job to finish, return when it is done.
    /// Does other job work in the background if possible
    /// </summary>
    /// <param name="aJob">Job to wait on</param>
    void Wait( const Job* aJob );

    /// <summary>
    /// Fin out how many threads this system supports
    /// </summary>
    /// <returns>Amount of supported threads for this system</returns>
    static const unsigned int GetAmountOfSupportedThreads();

private:

    JobManager() {}     // any startup should be done in Startup
    ~JobManager() {}    // any cleanup should be done in shutdown

    /// <summary>
    /// Main work for each thread; Will do any work possible
    /// or try and steal work from other threads
    /// </summary>
    void WorkerThread();

    /// <summary>
    /// Yield the work of this thread by sleeping it
    /// </summary>
    void YieldWorker();

    /// <summary>
    /// Get a new job
    /// </summary>
    /// <returns>Pointer to a new job</returns>
    Job* AllocateJob();

    /// <summary>
    /// Grabs the next available job from this worker's queue,
    /// if one is not available then try to get one from another 
    /// thread. If that dones't work, return nullptr
    /// </summary>
    /// <returns></returns>
    Job* GetJob();

    /// <summary>
    /// Checks if the given job is complete or not
    /// </summary>
    /// <param name="aJob">The job to check</param>
    /// <returns>True if the job is compelte, false otherwise</returns>
    bool HasJobCompleted( const Job* aJob );

    /// <summary>
    /// Run the jobs function and mark it as finished when completed. 
    /// </summary>
    /// <param name="aJob">The job to run</param>
    void Execute( Job* aJob );

    /// <summary>
    /// Mark a job as finished and let its parent know the job is done if it has one
    /// </summary>
    /// <param name="aJob">Job to finish</param>
    void Finish( Job* aJob );

    /** Flag for if the system should keep running or not */
    std::atomic<bool> IsDone { false };

    /** A vector of all worker threads in the pool */
    static std::vector<std::thread> WorkerThreads;

    /** Lockless queue for keeping track of what jobs are ready to be done */
    static moodycamel::ConcurrentQueue<Job*> locklessReadyQueue;

    /** Job many possible jobs we can have in a pool */
    const static thread_local unsigned int MAX_JOB_COUNT = 4096;
    /** Memory block of possible jobs */
    static thread_local Job g_jobAllocator[ MAX_JOB_COUNT ];
    /** Thread-local variable used to keep track of which job we are on */
    static thread_local uint32_t g_allocatedJobs;

};