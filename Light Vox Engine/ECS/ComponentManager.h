#pragma once
#include "EntityComponents.h"

namespace ECS {
    class ComponentManager
    {
    public:
        //SOA components
        EntityComponents::TransformMatrix* transformMatrix;
        EntityComponents::Transform* transform;
        EntityComponents::BoxCollider* boxCollider;
        EntityComponents::BodyProperties* bodyProperties;
        EntityComponents::Contacts* contacts;

        static ComponentManager* GetInstance();
        void Init();
        void ReleaseInstance();

        ComponentManager( ComponentManager const& ) = delete;
        void operator=( ComponentManager const& ) = delete;

        void AddContactsFound();
        void ClearContactsFound();
        size_t GetContactsFound();
    private:
        size_t contactsFound = 0;

        ComponentManager();
        ~ComponentManager();

        static ComponentManager* Instance;
    };

};