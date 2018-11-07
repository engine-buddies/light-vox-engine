#pragma once

#include <thread>               // std::thread
#include <condition_variable>   // std::condition_variable
#include <vector>               // std::vector
#include <atomic>               // std::atomic
#include <future>
#include <memory>

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

        void AddTask( void( *func_ptr )( void*, int ), void* args, int Index );

        template <class T>
        void AddTask( T* aParent, 
            void( T::*func_ptr )( void*, int ),
            void* args,
            int Index )
        {
            CpuJob aJob = {};
            aJob.args = args;
            aJob.index = Index;

            IJob* jobPtr = new JobMemberFunc<T>( aParent, func_ptr );
            aJob.TaskPtr = jobPtr;
            
            readyQueue.emplace_back( aJob );
            jobAvailableCondition.notify_one();
        }

        void TestMemberFunc( void* aArgs, int index )
        {
            printf( "Hey this is a MEMBER %s\n", ( char* ) aArgs );
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