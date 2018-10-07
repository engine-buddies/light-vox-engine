#pragma once


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


public:

    ///////////////////////////////////////////////////
    // Entity components

    struct Transform
    {
        float position[ 3 ];
        int body;
        float rotation[ 4 ];
    };

    struct BodyProperties
    {
        float inertia_inverse[ 3 ];
        float mass_inverse;
    };

    struct BodyMomentum
    {
        float velocity[ 3 ];
        float unused0;
        float angular_velocity[ 3 ];
        float unused1;
    };

    struct SphereCollider
    {
        float radius;
    };

    struct BoxCollider
    {
        float size[ 3 ];
        float unused;
    };

    struct Contact
    {
        float position[ 3 ];
        float penetration;
        float normal[ 3 ];
        float friction;
    };

    struct BodyPair
    {
        int a;
        int b;
    };

    struct ContactData
    {
        Contact* data;
        BodyPair* bodies;
        int* tags;
        int capacity;
        int count;

        int* sleeping_pairs;
        int sleeping_count;
    };

    struct ColliderData
    {
        struct
        {
            int* tags;
            BoxCollider* data;
            Transform* transforms;
            int count;
        } boxes;

        struct
        {
            int* tags;
            SphereCollider* data;
            Transform* transforms;
            int count;
        } spheres;
    };

    struct BodyData
    {
        Transform* transforms;
        BodyProperties* properties;
        BodyMomentum* momentum;
        int idle_counters;
        int count;
    };

    struct BodyConnections
    {
        BodyPair* data;
        int count;
    };

    struct CachedContactImpulse
    {
        float impulse[ 3 ];
        float unused;
    };

    struct ContactCache
    {
        int* tags;
        CachedContactImpulse* data;
        int capacity;
        int count;
    };

    struct ActiveBodies
    {
        int* indices;
        int capacity;
        int count;
    };
};
