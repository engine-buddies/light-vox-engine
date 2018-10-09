#pragma once

#include "EntityComponents.h"

/// <summary>
/// Singleton for controlling the creation of game entities.
/// A structure of array's for each entity's components
/// </summary>
/// <author>Ben Hoffman</author>
class EntityManager
{
public:

    /// <summary>
    /// Getter for the InputManager
    /// </summary>
    /// <returns>Singleton instance of the input manager</returns>
    static EntityManager* GetInstance();

    /// <summary>
    /// Initialize the entity manager with default properties
    /// </summary>
    static void Init();

    /// <summary>
    /// Release the current singleton instance
    /// </summary>
    static void ReleaseInstance();

    // We don't want anything making copies of this class so delete these operators
    EntityManager( EntityManager const& ) = delete;
    void operator=( EntityManager const& ) = delete;

private:

    /// <summary>
    /// Private constructor for the entity manager
    /// </summary>
    EntityManager();

    /// <summary>
    /// Private destructor for the entity manager
    /// </summary>
    ~EntityManager();

    static EntityManager* Instance;

    ///////////////////////////////////////////////////
    // Entity components

    // TODO: Make these arrays the proper size
    EntityComponents::Transform Transforms[ 1024 ];

    EntityComponents::BodyProperties BodyProperties[ 1024 ];

    /*
    struct ContactImpulseData;
    struct ContactConstraintData;
    void Collide( ActiveBodies* active_bodies, ContactData* contacts, BodyData bodies, ColliderData colliders, BodyConnections body_connections );
    ContactImpulseData* Read_cached_impulses( ContactCache contact_cache, ContactData contacts );
    void Write_cached_impulses( ContactCache* contact_cache, ContactData contacts, ContactImpulseData* contact_impulses );
    ContactConstraintData* Setup_contact_constraints( ActiveBodies active_bodies, ContactData contacts, BodyData bodies, ContactImpulseData* contact_impulses );
    void Apply_Impulses( ContactConstraintData* data, BodyData bodies );
    void Update_Cached_impulses( ContactConstraintData* data, ContactImpulseData* contact_impulses );
    void UpdatePhysics( ActiveBodies active_bodies, BodyData bodies, float time_step );
    */

};
