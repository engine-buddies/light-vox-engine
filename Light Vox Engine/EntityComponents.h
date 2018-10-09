#pragma once

/// <summary>
/// Structures of the different components that an entity can have
/// </summary>
/// <author>Arturo</author>
/// <author>Ben Hoffman</author>
namespace EntityComponents
{
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


}