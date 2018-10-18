#include "JobSequence.h"

using namespace Jobs;

JobSequence::JobSequence( UINT aMaxJobsAllowed, UINT aMaxBarriers, UINT aMaxWaits )
    : maxJobs( aMaxJobsAllowed ), maxBarriers( aMaxBarriers ), maxWaits( aMaxWaits )
{

}


JobSequence::~JobSequence()
{
    // TODO: Any cleanup would go here, IE waiting for a sequence to complete
    // before ending
}

UINT JobSequence::Dispatch( void * aJob, void * aArgs, int aJobCount )
{
    // TODO: Add a job to this sequence and add it the job queue
    // Of the job manager
    if ( sequenceJobs.size() + 1 >= maxJobs ) return 1;
    if ( aJob == nullptr ) return 2;

    CpuJob TheJob { };
    // Cast to a correct type for now
    TheJob.func_ptr = static_cast<CpuJobFuncPtr> ( aJob );
    TheJob.args = aArgs;

    sequenceJobs.emplace_front( TheJob );

    return 0;
}

void JobSequence::GetNextJob_Unsafe( CpuJob& aOutJob )
{

    aOutJob = std::move( sequenceJobs.front() );

    sequenceJobs.pop_front();
}

void JobSequence::Barrier()
{
    // TODO : add a barrier Sync to this sequence
}

void JobSequence::Wait()
{
    // TODO: Create a barrier and wait for it's completion
}
