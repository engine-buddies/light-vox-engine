#include "Physics.h"

void Physics::Init()
{
    transform = add_component<Transform>();
}

void Physics::Update(double dt)
{
    transform->pos.x = dt * bodyProperties->velocity.x;
}

bool Physics::Collide(std::vector<Entity> es)
{
    for (int i = 0; i < es.size(); ++i) 
    {
        for (int j = 0; j < es.size(); ++j)
        {
            if (j == i)
                continue;
            DirectX::XMFLOAT3& aPos = es[i].get_component<Transform>()->pos;
            DirectX::XMFLOAT3& bPos = es[i].get_component<Transform>()->pos;
            float aRadius = es[i].get_component<SphereCollider>()->radius;
            float bRadius = es[i].get_component<SphereCollider>()->radius;

            float distanceSqr = (aPos.x - bPos.x) * (aPos.x - bPos.x) +
                (aPos.y - bPos.y) * (aPos.y - bPos.y) +
                (aPos.z - bPos.z) * (aPos.z - bPos.z);

            float radiusSqr = (aRadius + bRadius) * (aRadius + bRadius);

            return distanceSqr < radiusSqr;

        }
    }

}
