#pragma once

// The guide that I based this job system off of is here: 
// https://blog.molecular-matters.com/2015/08/24/job-system-2-0-lock-free-work-stealing-part-1-basics/

#include <atomic>

#define JOB_DATA_PADDING_SIZE   44

/** A job function is a function that can be worked on by the job manager */
typedef void( *JobFunction )( struct Job*, const void* );

/// <summary>
/// A job is something that represents work to be done by the thread pool.
/// Contains a pointer to it's work function and a counter to how many jobs
/// still need to be completed.
/// </summary>
struct Job
{
	JobFunction Function;                       // 8 byte
	Job* Parent;                                // 8 byte
	std::atomic<int32_t> UnfinishedJobs;        // 4 byte
	char Padding[ JOB_DATA_PADDING_SIZE ];      // 44 bytes, so that this object fills
												// a whole x64 cache line
};
