#pragma once

#include <deque>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>

template<typename T>
/// <summary>
/// Wrapper around std::deque to make it concurrent
/// and preserve the integrity of the data inside the object.
/// WARNING: Contains mutexes and locks 
/// </summary>
/// <author>Ben Hoffman</author>
class ConcurrentQueue
{
    
private:

    /// <summary>
    /// Dequeue will be used here because it does not allocate memory 
    /// on every push and pop and is constant-time
    /// </summary>
    std::deque<T> TheDequeue;

    /// <summary>
    /// The mutex to be used by this concurrent queue
    /// </summary>
    boost::mutex My_Mutex;

    /// <summary>
    /// Conditional variable stating that there is data in the deuque
    /// that can be used
    /// </summary>
    boost::condition_variable DataAvailableCondition;

public:
    
    /// <summary>
    /// Emplace data at the front of the deque
    /// -- Uses lock on mutex --
    /// </summary>
    /// <param name="aData">Data to emplace</param>
    template<typename T>
    void emplace_front( T const & aData )
    {
        boost::mutex::scoped_lock( My_Mutex );
        // Add the data safely
        TheDequeue.emplace_front( aData );
        // Notify any threads that may be waiting on a condition for new data
        DataAvailableCondition.notify_one();
    }

    /// <summary>
    /// Emplace data at the back of the deque
    /// -- Uses lock on mutex --
    /// </summary>
    /// <param name="aData">Data to emplace</param>
    template<typename T>
    void emplace_back( T const& aData )
    {
        boost::mutex::scoped_lock( My_Mutex );
        // Add the data safely
        TheDequeue.emplace_back( aData );
        // Notify any threads that may be waiting on a condition for new data
        DataAvailableCondition.notify_one();
    }
    

    /// <summary>
    /// -- Waits for the dequeue to not be empty -- 
    /// Guarantees the dequeue is not empty before returning data. 
    /// 
    /// </summary>
    /// <returns>The front element of the dequeue</returns>
    template<typename T>
    T wait_pop_front()
    {
        boost::mutex::scoped_lock( My_Mutex );
        // Wait to be notified of new data
        while ( TheDequeue.empty() )
        {
            DataAvailableCondition.wait( lock );
        }

        auto elem = std::move( TheDequeue.front() );  // Move the front element
        TheDequeue.pop_front();                       // Remove it from the deque

        return elem;
    }

    /// <summary>
    /// test if sequence is empty
    /// -- Uses lock on mutex --
    /// </summary>
    /// <returns>True if empty</returns>
    bool empty() const
    {
        boost::mutex::scoped_lock( My_Mutex );
        return TheDequeue.empty();
    }


};