#pragma once

#include <thread>           // std::thread
#include <deque>            // std::dequeue for the job queue
                            // because it does not allocate memory 
                            // on every push and pop and is constant-time
#include <vector>           // std::vector
#include <mutex>            // std::mutex
#include <atomic>           // std::atomic
#include "JobSequence.h"    // A job 
#include "CpuJob.h"         // typedefs for jobs


/// <summary>
/// Manage and execute jobs with an aim to increase
/// efficiency in using a multicore CPU
/// </summary>
/// <author>Ben Hoffman</author>
class JobManager
{
public:

    JobManager();

    ~JobManager();

    /// <summary>
    /// Adds a job to the ready queue
    /// </summary>
    void AddJob( int aIndex );

    ////////////////////////////////////////
    // Accessors

    inline const size_t GetThreadCount() const;
    
private:

    /// <summary>
    /// A way for the worker threads to ask for work from the 
    /// job queue. Blocking function
    /// </summary>
    void AskForWork();

    /// <summary>
    /// Notifies all threads that our work has been done
    /// </summary>
    void SignalDone();

    // Define some mutex things for easier reference later
    typedef std::mutex                mutex_t;
    typedef std::unique_lock<mutex_t> lock_t;

    
    mutex_t ReadyQueueMutex;

    // Ready queue for the jobs 
    std::deque<CpuJob> ReadyQueue;

    // Worker threads for executing jobs
    // A worker thread extracts a job from the job queue and executes it
    std::vector<std::thread> WorkerThreads;

    /// <summary>
    /// Atomic bool determining if we are done
    /// </summary>
    std::atomic<bool> IsDone;

    /// <summary>
    /// Condition to notify other threads that a job has been added.
    /// Puts an unused thread ot sleep until it is woken up by another
    /// thread
    /// </summary>
    std::condition_variable Condition_m;

};

