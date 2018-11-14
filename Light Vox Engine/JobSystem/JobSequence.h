#pragma once

//#include "stdafx.h"
#include "../stdafx.h"

#include <deque>        // std::deque


namespace Jobs
{

    /// <summary>
    /// Represents a subsystem for managing dependencies 
    /// </summary>
    /// <author>Ben Hoffman</author>
    class JobSequence
    {
    public:


        JobSequence( UINT aMaxJobsAllowed, UINT aMaxBarriers, UINT aMaxWaits );

        ~JobSequence();

        /// <summary>
        /// Dispatch a job to a sequence. The execution order of added
        /// jobs is undefined. 
        /// </summary>
        /// <param name="aJob">pointer to a job function</param>
        /// <param name="aArgs">Arguments to pass to the job function</param>
        /// <param name="aJobCount">Number of jobs</param>
        /// <returns>Success if 0, error code if not</returns>
        UINT Dispatch( void* aJob, void* aArgs, int aJobCount );

        /// <summary>
        /// Dispatch a barrier synchronization to a sequence.
        /// Jobs added by dispatch after the call of barrier are guaranteed
        /// that all the jobs added by dispatch before the call have competed
        /// and have been deleted
        /// </summary>
        void Barrier();

        /// <summary>
        /// - Blocking function - 
        /// Dispatch a barrier synchronization to a sequence 
        /// and wait for its completion
        /// </summary>
        void Wait();


    private:

        // The current set of jobs in this sequence
        //std::deque<CpuJob> sequenceJobs;

        ///////////////////////////////////////////
        // Max settings for this sequence

        UINT maxJobs;

        UINT maxBarriers;

        UINT maxWaits;

    };

};