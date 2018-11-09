#pragma once

#include <future>

namespace Jobs
{
    // This is an explanation of how this logic works if you are curious 
    // https://stackoverflow.com/questions/12988320/store-function-pointers-to-any-member-function

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

            (parentObj->*func_ptr)( args, aIndex );
            return true;
        }

        /** the object to invoke the function pointer on */
        T* parentObj;
        /** The function pointer to call when we invoke this function */
        void ( T::*func_ptr )( void*, int );
    };

   /* template <class T>
    struct TrackedJob : IJob
    {
        TrackedJob( T* aParent, void ( T::*f )( void*, int, std::promise<void> ) )
        {
            parentObj = aParent;
            func_ptr = f;
        }

        virtual bool invoke( void* args, int aIndex, std::promise<void> aBarrier ) override
        {
            if ( !parentObj ) { aBarrier.set_value(); return false; }

            ( parentObj->*func_ptr )( args, aIndex );
            aBarrier.set_value();
            return true;
        }

        T* parentObj;
        void ( T::*func_ptr )( void*, int, std::promise<void> );
    };*/


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