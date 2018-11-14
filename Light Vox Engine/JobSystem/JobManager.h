#pragma once

#include <thread>               // std::thread
#include <condition_variable>   // std::condition_variable
#include <vector>               // std::vector
#include <atomic>               // std::atomic
#include <future>

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

        /// <summary>
        /// Add a non-member function to the job queue
        /// </summary>
        /// <param name="func_ptr">Function to add to the job queue</param>
        /// <param name="args">Arguments for the job function</param>
        /// <param name="Index">Index of this job</param>
        void AddJob( void( *func_ptr )( void*, int ), void* args, int Index );

        /// <summary>
        /// Add a member function to the job queue
        /// </summary>
        /// <param name="aParent">That calling object</param>
        /// <param name="func_ptr">Functoin to jobified</param>
        /// <param name="args">Arguments to pass to that function</param>
        /// <param name="Index">Index of this job</param>
        template <class T>
        void AddJob( T* aParent,
            void( T::*func_ptr )( void*, int ),
            void* args,
            int Index )
        {
            CpuJob aJob = {};
            aJob.args = args;
            aJob.index = Index;

            // #TODO
            // make this a pooled resource
            IJob* jobPtr = new JobMemberFunc<T>( aParent, func_ptr );
            aJob.TaskPtr = jobPtr;

            readyQueue.emplace_back( aJob );
            jobAvailableCondition.notify_one();
        }

        void TestTrackedFunc( void* aArgs, int index )
        {
            std::promise<void> *aPromise = ( std::promise<void>* )( aArgs );

            printf( "Inside the test tracked func \n\n" );
            std::this_thread::sleep_for( std::chrono::seconds::duration( 1 ) );
            printf( "Done inside the tracked function\n\n" );
            
            aPromise->set_value();
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

        ////////////////////////////////////////
        // Job Definitions

        /// <summary>
        /// Base functor for jobs to use
        /// </summary>
        struct IJob
        {
            virtual ~IJob() {}
            virtual bool invoke( void* args, int aIndex ) = 0;
        };

        /// <summary>
        /// A job function that is not a member of a class
        /// </summary>
        struct JobFunc : IJob
        {
            JobFunc( void( *aFunc_ptr )( void*, int ) )
                : func_ptr( aFunc_ptr )
            {

            }

            /// <summary>
            /// invoke this job with the given arguments
            /// </summary>
            /// <param name="args">Arguments to pass to this job function</param>
            /// <param name="aIndex">Index of this jobect</param>
            /// <returns>True if job was successful</returns>
            virtual bool invoke( void* args, int aIndex ) override
            {
                func_ptr( args, aIndex );
                return true;
            }

            /** The function pointer for this job to invoke */
            void( *func_ptr )( void*, int );
        };

        /// <summary>
        /// A job member function that is a member of a class
        /// </summary>
        template <class T>
        struct JobMemberFunc : IJob
        {
            JobMemberFunc( T* aParent, void ( T::*f )( void*, int ) )
            {
                parentObj = aParent;
                func_ptr = f;
            }

            virtual bool invoke( void* args, int aIndex ) override
            {
                if ( !parentObj ) { return false; }

                ( parentObj->*func_ptr )( args, aIndex );
                return true;
            }

            /** the object to invoke the function pointer on */
            T* parentObj;

            /** The function pointer to call when we invoke this function */
            void ( T::*func_ptr )( void*, int );
        };


        /// <summary>
        /// A job that will be run in parallel with others on the CPU
        /// A function pointer for the worker threads to use
        /// </summary>
        /// <author>Ben Hoffman</author>
        struct CpuJob
        {
            IJob* TaskPtr = nullptr;
            void* args = nullptr;
            int index = 0;
        };

        // Ready queue for the jobs
        ConcurrentQueue<CpuJob> readyQueue;

    };

};      // namespace jobs