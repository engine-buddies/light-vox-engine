#pragma once 
#include "stdafx.h"

class Entity
{
public:
    uint32_t index;
    uint32_t counter; //for keeping track of avalible entities

    /// <summary>
    /// Creates an entity 
    /// </summary>
    Entity() : index( 0 ), counter( 0 ) {}
    /// <summary>
    /// Creates en Entity with given index and counter
    /// </summary>
    /// <param name="a_index"></param>
    /// <param name="a_counter"></param>
    Entity( unsigned int a_index, unsigned int a_counter ) :
        index( a_index ), counter( a_counter )
    {
    }

    /// <summary>
    /// Compares if two entities are the same
    /// </summary>
    /// <param name="rhs"></param>
    /// <returns></returns>
    inline bool operator==( const Entity& rhs ) const
    {
        return index == rhs.index && counter == rhs.counter;
    }

    /// <summary>
    /// Compares if two entities are not the same
    /// </summary>
    /// <param name="rhs"></param>
    /// <returns></returns>
    inline bool operator!=( const Entity& rhs ) const
    {
        return !( *this == rhs );
    }
    /// <summary>
    /// Checks for a valid entity
    /// </summary>
    /// <returns></returns>
    bool Is_Valid() const { return !( index == 0 && counter == 0 ); }
    /// <summary>
    /// Checks for a valid entity
    /// </summary>
    /// <returns></returns>
    operator bool() const { return Is_Valid(); }

};

