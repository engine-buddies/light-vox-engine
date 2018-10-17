#include "EntityManager.h"


// Singleton requirement
EntityManager* EntityManager::Instance = nullptr;

EntityManager* EntityManager::GetInstance()
{
    if ( Instance == nullptr )
        Instance = new EntityManager();

    return Instance;
}

void EntityManager::Init()
{
    firstFree = NONE;
    lastFree = NONE;
}

void EntityManager::ReleaseInstance()
{
    if ( Instance != nullptr )
    {
        delete Instance;
        Instance = nullptr;
    }
}

Entity EntityManager::Create_Entity()
{
    unsigned int i = Get_Free_Entry();
    entries[ i ].counter++;
    assert( entries[ i ].counter != 0 && "Out of handles" );
    entries[ i ].next_free = USED;
    return Entity( i, entries[ i ].counter );
}

void EntityManager::Free_Entity( Entity e )
{
    unsigned int index = e.index;
    entries[ index ].next_free = NONE;
    if ( lastFree == NONE )
    {
        firstFree = lastFree = index;
        return;
    }
    entries[ lastFree ].next_free = index;
    lastFree = index;
}

bool EntityManager::Is_Valid( Entity e ) const
{
    if ( !e.Is_Valid() )
        return false;
    if ( e.index >= entries.size() )
        return false;
    const index_entry& entry = entries[ e.index ];
    return entry.next_free == USED && entry.counter == e.counter;
}

void EntityManager::Clear()
{
    firstFree = NONE;
    lastFree = NONE;
    entries.clear();
}

Entity EntityManager::Get_Entity( int index ) const
{
    if ( index >= 0 && index < entries.size() )
        return Entity( index, entries[ index ].counter );
    return {};
}

EntityManager::EntityManager()
{

}

EntityManager::~EntityManager()
{

}

unsigned int EntityManager::Get_Free_Entry()
{
    if ( firstFree != NONE )
    {
        int result = firstFree;
        firstFree = entries[ result ].next_free;
        entries[ result ].next_free = USED;
        if ( firstFree == NONE )
            lastFree = NONE;
        return result;
    }
    entries.emplace_back();
    return unsigned int( entries.size() - 1 );
}
