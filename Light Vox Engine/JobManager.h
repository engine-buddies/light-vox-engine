#pragma once

#include <thread>           // std::thread
#include <deque>            // std::dequeue for the job queue
                            // because it does not allocate memory 
                            // on every push and pop and is constant-time
#include <vector>           // std::vector
#include <mutex>
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

    ////////////////////////////////////////
    // Accessors

    inline const size_t GetThreadCount() const;
    
private:

    /// <summary>
    /// A way for the worker threads to ask for work from the 
    /// job queue. Blocking function
    /// </summary>
    void AskForWork();

    // Define some mutex things for easier reference later
    typedef std::mutex                mutex_t;
    typedef std::unique_lock<mutex_t> lock_t;

    // Ready queue for the jobs 
    std::deque<CpuJob> ReadyQueue;

    // Worker threads for executing jobs
    // A worker thread extracts a job from the job queue and executes it
    std::vector<std::thread> WorkerThreads;

    

};

