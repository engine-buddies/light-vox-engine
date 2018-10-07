#pragma once
class Physics
{
public:
    Physics();
    ~Physics();
    struct ContactImpulseData;
    struct ContactConstraintData;

    /*void Collide(ActiveBodies* active_bodies, ContactData* contacts, BodyData bodies, ColliderData colliders, BodyConnections body_connections);

    ContactImpulseData* Read_cached_impulses(ContactCache contact_cache, ContactData contacts);

    void Write_cached_impulses(ContactCache* contact_cache, ContactData contacts, ContactImpulseData* contact_impulses);

    ContactConstraintData* Setup_contact_constraints(ActiveBodies active_bodies, ContactData contacts, BodyData bodies, ContactImpulseData* contact_impulses);

    void Apply_Impulses(ContactConstraintData* data, BodyData bodies);

    void Update_Cached_impulses(ContactConstraintData* data, ContactImpulseData* contact_impulses);

    void UpdatePhysics(ActiveBodies active_bodies, BodyData bodies, float time_step);*/
};

