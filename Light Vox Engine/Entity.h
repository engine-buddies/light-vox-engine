#pragma once 
#include "stdafx.h"

class Entity
{
public:
    uint32_t index;
    uint32_t counter; //for keeping track of avalible entities
    
    Entity() : index(0), counter(0) {}
    Entity(unsigned int a_index, unsigned int a_counter) :
        index(a_index), counter(a_counter) {}

    inline bool operator==(const Entity& rhs) const
    {
        return index == rhs.index && counter == rhs.counter;
    }

    inline bool operator!=(const Entity& rhs) const
    {
        return !(*this == rhs);
    }

    bool Is_Valid() const { return !(index == 0 && counter == 0); }
    operator bool() const { return Is_Valid(); }

};

