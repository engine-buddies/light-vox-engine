#pragma once

#include <thread>           // std::thread
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

    
    /// <summary>
    /// Get the current job manager
    /// </summary>
    /// <returns>Static instance of the job manager</returns>
    static JobManager* GetInstance();
    
    /// <summary>
    /// Destroy the current job manager instance
    /// </summary>
    static void ReleaseInstance();

    // We don't want anything making copies of this class so delete these operators
    JobManager( JobManager const& )     = delete;
    void operator=( JobManager const& ) = delete;

private:

    JobManager();

    ~JobManager();

    /** Static instance of the job manager  */
    static JobManager* Instance;

    // Job sequences for managing jobs 

    
    // Ready queue for the jobs 


    // Worker threads for executing jobs
    // A worker thread extracts a job from the job queue and executes it


};

