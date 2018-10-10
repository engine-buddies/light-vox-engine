#pragma once


/// <summary>
/// A job that will be run in parallel with others on the CPU
/// A function pointer for the worker threads to use
/// </summary>
/// <author>Ben Hoffman</author>
struct CpuJob
{
    int priority;
    void( *func_ptr )( void* arg, int index );
};

// This is the definition of a CPU job that we have to roll with
// typedef void( *CpuJob ) ( void * arg, int index );
