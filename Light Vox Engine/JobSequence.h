#pragma once

/// <summary>
/// Represents a subsystem for managing dependencies 
/// </summary>
/// <author>Ben Hoffman</author>
class JobSequence
{
public:

    JobSequence();

    ~JobSequence();


    /// <summary>
    /// Dispatch a job to a sequence. The execution order of added
    /// jobs is undefined. 
    /// </summary>
    /// <param name="aJob">pointer to a job function</param>
    /// <param name="aArgs">Arguments to pass to the job function</param>
    /// <param name="aJobCount">Number of jobs</param>
    void Dispatch( void* aJob, void* aArgs, int aJobCount );

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

    
};

