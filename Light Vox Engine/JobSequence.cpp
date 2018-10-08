#include "JobSequence.h"



JobSequence::JobSequence()
{

}


JobSequence::~JobSequence()
{

}

void JobSequence::Dispatch( void * aJob, void * aArgs, int aJobCount )
{
    // TODO: Add a job to this sequence and add it the job queue
    // Of the job manager
}

void JobSequence::Barrier()
{
    // TODO : add a barrier Sync to this sequence
}

void JobSequence::Wait()
{
    // TODO: Create a barrier and wait for it's completion
}
