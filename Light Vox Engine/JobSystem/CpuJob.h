#pragma once

#include <functional>


namespace Jobs
{
    /// <summary>
    /// Priority of a CPU job. Will determine how the job 
    /// is sorted into the job queue
    /// </summary>
    enum JobPriority
    {
        CRITICAL,
        HIGH,
        NORMAL,
        LOW
    };

    typedef void( *CpuJobFuncPtr ) ( void * args, int index );
    
    /// <summary>
    /// A job that will be run in parallel with others on the CPU
    /// A function pointer for the worker threads to use
    /// </summary>
    /// <author>Ben Hoffman</author>
    struct CpuJob
    {
        JobPriority priority = JobPriority::NORMAL;

        CpuJobFuncPtr func_ptr = nullptr;

        void* args = nullptr;

        int index = 0;
    };

    
};      // namespace jobs

// This is the definition of a CPU job that we have to roll with
