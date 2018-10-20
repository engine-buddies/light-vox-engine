#pragma once

#include <thread>               // std::thread
#include <condition_variable>   // std::condition_variable
#include <vector>               // std::vector
#include <atomic>               // std::atomic

#include "JobSequence.h"        // A job 
#include "CpuJob.h"             // typedefs for jobs
#include "ConcurrentQueue.h"    // Concurrent queue that is NOT LOCKLESS

namespace Jobs
{

    /// <summary>
    /// Manage and execute jobs with an aim to increase
    /// efficiency in using a multicore CPU
    /// </summary>
    /// <author>Ben Hoffman</author>
    class JobManager
    {
    public:

        /// <summary>
        /// Get the Job manager instance
        /// </summary>
        /// <returns>Current instance of the job manager</returns>
        static JobManager* GetInstance();

        /// <summary>
        /// Release the current Job Manager instance
        /// </summary>
        static void ReleaseInstance();

        template <typename F>
        /// <summary>
        /// Add a job to the job queue
        /// </summary>
        /// <param name="function"></param>
        void AddJob( F&& function, void* jobArgs = nullptr, int aIndex = 0 )
        {
            CpuJob tempJob { };

            tempJob.job_func = std::forward<F>( function );

            tempJob.args = jobArgs;
            tempJob.index = aIndex;
            tempJob.priority = Jobs::JobPriority::NORMAL;

            readyQueue.emplace_back( tempJob );

            jobAvailableCondition.notify_one();
        }

        // We don't want anything making copies of this class so delete these operators
        JobManager( JobManager const& ) = delete;
        void operator=( JobManager const& ) = delete;

        ////////////////////////////////////////
        // Accessors

        inline const size_t GetThreadCount() const;

    private:

        JobManager();

        ~JobManager();

        static JobManager* instance;

        /// <summary>
        /// Worker thread will wait for any jobs
        /// to be available in the Ready Queue and execute
        /// them accordingly
        /// </summary>
        void WorkerThread();

        /// <summary>
        /// A mutex determining if the queue is ready
        /// </summary>
        std::mutex readyQueueMutex;

        // Ready queue for the jobs
        ConcurrentQueue<CpuJob> readyQueue;

        /// <summary>
        /// Conditional variable for if a job is available
        /// </summary>
        std::condition_variable jobAvailableCondition;

        /// <summary>
        /// Worker threads for executing jobs
        /// A worker thread extracts a job from the job queue and executes it
        /// </summary>
        std::vector<std::thread> workerThreads;

        /// <summary>
        /// Atomic bool determining if we are done used for closing all threads
        /// </summary>
        std::atomic<bool> isDone;

    };

};      // namespace jobs