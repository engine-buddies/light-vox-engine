#pragma once

#include <thread>           // std::thread
#include <queue>            // std::queue
#include <vector>           // std::vector
#include "JobSequence.h"    // A job 
#include "CpuJob.h"         // typedefs for jobs


/// <summary>
/// Manage and execute jobs with an aim to increase
/// efficiency in using a multicore CPU
/// </summary>
/// <author>Ben Hoffman</author>?
class JobManager
{
public:

    JobManager();

    ~JobManager();

    ////////////////////////////////////////
    // Accessors

    inline const bool GetIsDone() const;

    inline const size_t GetThreadCount() const;
    
private:

    /// <summary>
    /// A way for the worker threads to ask for work from the 
    /// job queue
    /// </summary>
    void AskForWork();

    /** Determines if the application is still running or not */
    bool IsDone;

    // Job sequences for managing jobs 
    CpuJob testJobBoi;
    
    // Ready queue for the jobs 
    std::queue<CpuJob> JobQueue;


    // Worker threads for executing jobs
    // A worker thread extracts a job from the job queue and executes it
    std::vector<std::thread> WorkerThreads;

    

};

