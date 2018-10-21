#pragma once

#include <functional>

namespace Jobs
{

    typedef std::function<void( void*, int )> job_t;

    /// <summary>
    /// Priority of a CPU job. Will determine how the job 
    /// is sorted into the job queue
    /// </summary>
    /// <author>Ben Hoffman</endif>
    enum JobPriority
    {
        CRITICAL,
        HIGH,
        NORMAL,
        LOW
    };

    /// <summary>
    /// A job that will be run in parallel with others on the CPU
    /// A function pointer for the worker threads to use
    /// </summary>
    /// <author>Ben Hoffman</author>
    struct CpuJob
    {
        job_t job_func;     // This is 64 bytes!!!  Ahhhh

        void* args = nullptr;

        int index = 0;

        JobPriority priority = JobPriority::NORMAL;
    };

};      // namespace jobs
