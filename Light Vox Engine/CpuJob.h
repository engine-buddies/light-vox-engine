#pragma once


/// <summary>
/// A job that will be run in parallel with others on the CPU
/// A function pointer for the worker threads to use
/// </summary>
/// <author>Ben Hoffman</author>
typedef void( *CpuJob ) ( void * arg, int index );
