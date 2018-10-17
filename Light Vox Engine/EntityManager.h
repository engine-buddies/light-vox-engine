#pragma once
#include <vector>
#include "stdafx.h"
#include "Entity.h"
#include "EntityComponents.h"

/// <summary>
/// Singleton for controlling the creation of game entities.
/// A structure of array's for each entity's components
/// </summary>
/// <author>Arturo Kuang</author>

class EntityManager
{
    static const int NONE = -1;
    static const int USED = -2;
    static const uint32_t MAX_GAME_OBJECTS = LV_MAX_INSTANCE_COUNT;
public:

    /// <summary>
    /// Getter for the InputManager
    /// </summary>
    /// <returns>Singleton instance of the input manager</returns>
    static EntityManager* GetInstance();

    /// <summary>
    /// Initialize the entity manager with default properties
    /// </summary>
    void Init();

    /// <summary>
    /// Release the current singleton instance
    /// </summary>
    void ReleaseInstance();

    /// <summary>
    /// Creates and Entity by finding next available index
    /// </summary>
    Entity Create_Entity();

    /// <summary>
    /// Removes Entity from index and makes it available
    /// </summary>
    void Free_Entity( Entity e );

    /// <summary>
    /// Checks if Entity exist 
    /// </summary>
    bool Is_Valid( Entity e ) const;

    /// <summary>
    /// Removes all entities
    /// </summary>
    void Clear();

    /// <summary>
    /// Returns Entity at index
    /// </summary>
    Entity Get_Entity( int index ) const;

    // We don't want anything making copies of this class so delete these operators
    EntityManager( EntityManager const& ) = delete;
    void operator=( EntityManager const& ) = delete;

private:
    EntityManager();
    ~EntityManager();

    static EntityManager* Instance;
    struct index_entry
    {
        unsigned int counter;
        int next_free;

        index_entry() : counter( 0 ), next_free( NONE ) {}
    };

    int firstFree;
    int lastFree;
    std::vector<index_entry> entries;

    unsigned int Get_Free_Entry();
};
