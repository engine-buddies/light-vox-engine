#include "RigidBody.h"

namespace {

    inline bool BoxIntersectBox(glm::vec3& posA, glm::vec3& posB, glm::vec3& sizeA, glm::vec3& sizeB)
    {
        //Bounding box min and max for A
        float aMaxX = posA.x + sizeA.x;
        float aMinX = posA.x - sizeA.x;
        float aMaxY = posA.y + sizeA.y;
        float aMinY = posA.y - sizeA.y;
        float aMaxZ = posA.z + sizeA.z;
        float aMinZ = posA.z - sizeA.z;

        //Bounding box min and max for B
        float bMaxX = posB.x + sizeB.x;
        float bMinX = posB.x - sizeB.x;
        float bMaxY = posB.y + sizeB.y;
        float bMinY = posB.y - sizeB.y;
        float bMaxZ = posB.z + sizeB.z;
        float bMinZ = posB.z - sizeB.z;

        //check for intersection
        return (aMinX <= bMaxX && aMaxX >= bMinX) &&
            (aMinY <= bMaxY && aMaxY >= bMinY) &&
            (aMinZ <= bMaxZ && aMaxZ >= bMinZ);
    }

    inline bool BoxIntersectHalfSpace(
        const EntityComponents::BoxCollider& box,
        const EntityComponents::PlaneCollider& plane)
    {

    }

    /// <summary>
    /// Gets a vector representing one axis in the matrix 
    /// </summary>
    /// <param name="i"></param>
    /// <param name="mat"></param>
    /// <returns></returns>
    inline glm::vec3 GetAxisVector(int i, const glm::mat4& mat)
    {
        return glm::vec3(mat[i][0], mat[i][1], mat[i][2]);
    }

    inline float transfromToAxis(
        const EntityComponents::BoxCollider& box,
        const glm::vec3& axis
    )
    {
        glm::mat4 transform = box.transformMatrix;
        glm::vec3 size = box.size;
        //project axis onto basis axis of box 
        return
            size.x * glm::abs(glm::dot(axis, GetAxisVector(0, transform))) +
            size.y * glm::abs(glm::dot(axis, GetAxisVector(1, transform))) +
            size.z * glm::abs(glm::dot(axis, GetAxisVector(2, transform)));

    }

    inline float penetrationOnAxis(
        const EntityComponents::BoxCollider& one,
        const EntityComponents::BoxCollider& two,
        const glm::vec3& axis,
        const glm::vec3& toCenter
    )
    {
        //project the half-size of one onto axis
        float oneProject = transfromToAxis(one, axis);
        float twoProject = transfromToAxis(two, axis);

        //project this onto the axis 
        float distance = glm::abs(glm::dot(toCenter, axis));

        //return the overlap 
        return oneProject + twoProject - distance;
    }

    inline bool tryAxis(
        const EntityComponents::BoxCollider& one,
        const EntityComponents::BoxCollider& two,
        glm::vec3 axis,
        const glm::vec3 toCenter,
        UINT axisIndex,
        float& smallestPenetration,
        UINT& smallestCase)
    {
        if (glm::length2(axis) < 0.0001)
            return true;

        axis = glm::normalize(axis);

        float penetration = penetrationOnAxis(one, two, axis, toCenter);

        if (penetration < 0)
            return false;

        if (penetration < smallestPenetration)
        {
            smallestPenetration = penetration;
            smallestCase = axisIndex;
        }

        return true;
    }
}

Physics::Rigidbody::Rigidbody()
{
    componentManager = ECS::ComponentManager::GetInstance();
}

Physics::Rigidbody::~Rigidbody()
{
}

void Physics::Rigidbody::Pos(glm::vec3& pos, const UINT& index)
{
    componentManager->transform[index].pos = pos;
}

void Physics::Rigidbody::RotateAxisAngle(glm::vec3& rotationAxis, float angle, const UINT& index)
{
    componentManager->transform[index].orientation = glm::angleAxis(glm::degrees(angle), rotationAxis);
}

void Physics::Rigidbody::Velocity(glm::vec3& vel, const UINT& index)
{
    componentManager->bodyProperties[index].velocity = vel;
}

void Physics::Rigidbody::Acceleration(glm::vec3& accel, const UINT& index)
{
    componentManager->bodyProperties[index].acceleration = accel;
}

void Physics::Rigidbody::Force(glm::vec3& force, const UINT& index)
{
    componentManager->bodyProperties[index].force = force;
}

void Physics::Rigidbody::Mass(float mass, const UINT& index)
{
    if (mass = 0.0f)
        mass = 1.0f;

    componentManager->bodyProperties[index].mass = mass;
    componentManager->bodyProperties[index].invMass = (1 / mass);
}

void Physics::Rigidbody::Scale(glm::vec3& scale, const UINT& index)
{
    componentManager->transform[index].scale = scale;
}

void Physics::Rigidbody::BoxColliderSize(glm::vec3& size, const UINT& index)
{
    componentManager->boxCollider[index].size = size;
}

void Physics::Rigidbody::InertiaTensor(glm::mat3 & inertiaTensor, const UINT& index)
{
    componentManager->bodyProperties[index].inertiaTensor = inertiaTensor;
}

void Physics::Rigidbody::SetAwake(const bool awake, const UINT& index)
{
    bool& bodyAwake = componentManager->bodyProperties[index].isAwake;
    if (awake)
    {
        bodyAwake = true;
    }
    else
    {
        bodyAwake = false;

        componentManager->bodyProperties[index].velocity = glm::vec3(.0f);
        componentManager->transform[index].rot = glm::vec3(.0f);
    }
}

void Physics::Rigidbody::AddForceAtPoint(
    const glm::vec3 & force,
    const glm::vec3 & point,
    const UINT& index)
{
    glm::mat4& transformMatrix = componentManager->transform[index].transformMatrix;
    glm::vec3& pos = componentManager->transform[index].pos;
    glm::vec3& _force = componentManager->bodyProperties[index].force;
    glm::vec3& _torque = componentManager->bodyProperties[index].torque;

    glm::vec3 pt = transformMatrix * glm::vec4(point, 0.0f);
    pt -= pos;

    _force += force;
    _torque += glm::cross(pt, _torque);

    componentManager->bodyProperties[index].isAwake = true;
}


bool Physics::Rigidbody::IntersectBoxBox(const UINT& entityA, const UINT& entityB)
{
    if (entityA == entityB)
        return false;

    glm::vec3& posA = componentManager->transform[entityA].pos;
    glm::vec3& posB = componentManager->transform[entityB].pos;

    glm::vec3& sizeA = componentManager->boxCollider[entityA].size;
    glm::vec3& sizeB = componentManager->boxCollider[entityB].size;

#ifdef _DEBUG

    if (BoxIntersectBox(posA, posB, sizeA, sizeB))
    {
        DEBUG_PRINT("Entity: %i hit Entity: %i \n", entityA, entityB);
    }
#else
    return BoxIntersectBox(posA, posB, sizeA, sizeB);
#endif
}


// This preprocessor definition is only used as a convenience
// in the boxAndBox contact generation method.
#define CHECK_OVERLAP(axis, index) \
    if (!tryAxis(one, two, (axis), toCenter, (index), pen, best)) return 0;

int Physics::Rigidbody::CollideBoxBox(const UINT& entityA, const UINT& entityB)
{
    glm::vec3& posA = componentManager->transform[entityA].pos;
    glm::vec3& posB = componentManager->transform[entityB].pos;

    EntityComponents::BoxCollider& one = componentManager->boxCollider[entityA];
    EntityComponents::BoxCollider& two = componentManager->boxCollider[entityB];

    //Find the vector b/w two centers
    glm::vec3 toCenter = posA - posB;

    //start by assuming there is no contact 
    float pen = std::numeric_limits<float>::max();
    uint32_t best = 0xffffff;

    //Now check each axes, returning if it gives us 
    //a separating axis, and keeping track of the axes 
    //the smallest penetration
    CHECK_OVERLAP(GetAxisVector(0, one.transformMatrix), 0);
    CHECK_OVERLAP(GetAxisVector(0, one.transformMatrix), 1);
    CHECK_OVERLAP(GetAxisVector(0, one.transformMatrix), 2);

    CHECK_OVERLAP(GetAxisVector(0, two.transformMatrix), 3);
    CHECK_OVERLAP(GetAxisVector(0, two.transformMatrix), 4);
    CHECK_OVERLAP(GetAxisVector(0, two.transformMatrix), 5);

    //Store the best axis major 
    uint32_t bestSingleAxis = best;

    CHECK_OVERLAP(glm::cross(GetAxisVector(0, one.transformMatrix), GetAxisVector(0, two.transformMatrix)), 6);
    CHECK_OVERLAP(glm::cross(GetAxisVector(0, one.transformMatrix), GetAxisVector(1, two.transformMatrix)), 7);
    CHECK_OVERLAP(glm::cross(GetAxisVector(0, one.transformMatrix), GetAxisVector(2, two.transformMatrix)), 8);
    CHECK_OVERLAP(glm::cross(GetAxisVector(1, one.transformMatrix), GetAxisVector(0, two.transformMatrix)), 9);
    CHECK_OVERLAP(glm::cross(GetAxisVector(1, one.transformMatrix), GetAxisVector(1, two.transformMatrix)), 10);
    CHECK_OVERLAP(glm::cross(GetAxisVector(1, one.transformMatrix), GetAxisVector(2, two.transformMatrix)), 11);
    CHECK_OVERLAP(glm::cross(GetAxisVector(2, one.transformMatrix), GetAxisVector(0, two.transformMatrix)), 12);
    CHECK_OVERLAP(glm::cross(GetAxisVector(2, one.transformMatrix), GetAxisVector(1, two.transformMatrix)), 13);
    CHECK_OVERLAP(glm::cross(GetAxisVector(2, one.transformMatrix), GetAxisVector(2, two.transformMatrix)), 14);

    //make sure we got the right results
    assert(best != 0xffffff);

    //we know there is collision 




    return 0;
}
