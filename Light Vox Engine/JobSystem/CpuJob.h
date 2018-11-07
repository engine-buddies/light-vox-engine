#pragma once

#include <functional>
#include <future>
#include <memory>

namespace Jobs
{

    //typedef std::function<void( void*, int )> job_t;


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



    // https://stackoverflow.com/questions/12988320/store-function-pointers-to-any-member-function

    /// <summary>
    /// Base functor for jobs to use
    /// </summary>
    struct IJob
    {
        virtual ~IJob() {}
        virtual bool invoke( void* args, int aIndex ) = 0;
    };

    // Use this for non-member functions
    struct JobFunc : IJob
    {
        JobFunc( void( *aFunc_ptr )( void*, int ) )
            : func_ptr( aFunc_ptr )
        {

        }

        virtual bool invoke( void* args, int aIndex ) override
        {
            func_ptr( args, aIndex );
            return true;
        }

        void( *func_ptr )( void*, int );
    };

    // Use this for member functions
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

            (parentObj->*func_ptr)( args, aIndex );
            return true;
        }

        T* parentObj;
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

};      // namespace jobs