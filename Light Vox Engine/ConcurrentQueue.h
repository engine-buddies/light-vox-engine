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
    
public:
    
    /// <summary>
    /// Emplace data at the front of the deque
    /// -- Uses lock on mutex --
    /// </summary>
    /// <param name="aData">Data to emplace</param>
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
    void emplace_back( T const& aData )
    {
        boost::mutex::scoped_lock( My_Mutex );
        // Add the data safely
        TheDequeue.emplace_back( aData );
        // Notify any threads that may be waiting on a condition for new data
        DataAvailableCondition.notify_one();
    }
    
    /// <summary>
    /// Pop's the front item off of the deque 
    /// and puts it's values to the item passed in
    /// -- Uses mutex -- 
    /// </summary>
    /// <param name="aItem">Out variable</param>
    void pop_front(T& aItem)
    {
        boost::mutex::scoped_lock waitLock( My_Mutex );

        while ( TheDequeue.empty() )
        {
            DataAvailableCondition.wait( waitLock );
        }
        aItem = std::move( TheDequeue.front() );
        printf( "Hello front boi: %p \n", &aItem );
        TheDequeue.pop_front();
    }
    
    
    /// <summary>
    /// returns the first element of the mutable sequence
    /// -- Uses lock on mutex --
    /// </summary>
    /// <returns>first element of the mutable sequence</returns>
    T& front()
    {
        boost::mutex::scoped_lock( My_Mutex );        

        return TheDequeue.front();
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

    size_t size() const
    {
        boost::mutex::scoped_lock( My_Mutex );
        return TheDequeue.size();
    }

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


};