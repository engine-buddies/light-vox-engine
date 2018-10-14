#pragma once

#include <thread>
#include <condition_variable>
#include <vector>           // std::vector
#include <atomic>           // std::atomic

#include "JobSequence.h"    // A job 
#include "CpuJob.h"         // typedefs for jobs
#include "ConcurrentQueue.h"

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
    void AddJob( CpuJob aJob );

    // TODO: Sequence factory interface

    ////////////////////////////////////////
    // Accessors

    inline const size_t GetThreadCount() const;
    
private:

    /// <summary>
    /// Worker thread will wait for any jobs
    /// to be available in the Ready Queue and execute
    /// them accordingly
    /// </summary>
    void WorkerThread();
    

    /// <summary>
    /// A mutex determining if the queue is ready
    /// </summary>
    std::mutex ReadyQueueMutex;

    // Ready queue for the jobs
    ConcurrentQueue<CpuJob> ReadyQueue;

    /// <summary>
    /// Conditional variable for if a job is available
    /// </summary>
    std::condition_variable JobAvailable;

    // Worker threads for executing jobs
    // A worker thread extracts a job from the job queue and executes it
    std::vector<std::thread> WorkerThreads;
    
    /// <summary>
    /// Atomic bool determining if we are done
    /// </summary>
    std::atomic<bool> IsDone;

    
};

