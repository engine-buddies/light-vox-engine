#include "RigidBody.h"

namespace {

    inline bool BoxIntersectBox(glm::vec3& maxA, glm::vec3& minA, glm::vec3& maxB, glm::vec3& minB)
    {
       /* if (minA.x <= maxB.x && maxA.x >= minB.x)
            printf("test");

        if (minA.y <= maxB.y && maxA.y >= minB.y)
            printf("test");

        if (minA.z <= maxB.z && maxA.z >= minB.z)
            printf("test");*/

        //check for intersection
        return (minA.x <= maxB.x && maxA.x >= minB.x) &&
               (minA.y <= maxB.y && maxA.y >= minB.y) &&
               (minA.z <= maxB.z && maxA.z >= minB.z);
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

    glm::vec3& maxA = componentManager->boxCollider[entityA].maxVertex;
    glm::vec3& minA = componentManager->boxCollider[entityA].minVertex;

    glm::vec3& maxB = componentManager->boxCollider[entityB].maxVertex;
    glm::vec3& minB = componentManager->boxCollider[entityB].minVertex;

#ifdef _DEBUG

    if (BoxIntersectBox(maxA, minA, maxB, minB))
    {
        DEBUG_PRINT("Entity: %i hit Entity: %i \n", entityA, entityB);
        return true;
    }
    return false;
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
    if (best < 3)
    {
        //vertex of box two on a face of box one 
        //fillPointFaceBoxBox(two, one, toCentre*-1.0f, data, best - 3, pen);
        //data->addContacts(1);
        return 1;
    }
    else if (best < 6)
    {
        // We've got a vertex of box one on a face of box two
        //fillPointFaceBoxBox(two, one, toCentre*-1.0f, data, best - 3, pen);
        //data->addContacts(1);
        return 1;
    }
    else
    {
        //// We've got an edge-edge contact. Find out which axes
        //best -= 6;
        //unsigned oneAxisIndex = best / 3;
        //unsigned twoAxisIndex = best % 3;
        //Vector3 oneAxis = one.getAxis(oneAxisIndex);
        //Vector3 twoAxis = two.getAxis(twoAxisIndex);
        //Vector3 axis = oneAxis % twoAxis;
        //axis.normalise();

        //// The axis should point from box one to box two.
        //if (axis * toCentre > 0) axis = axis * -1.0f;

        //// We have the axes, but not the edges: each axis has 4 edges parallel
        //// to it, we need to find which of the 4 for each object. We do
        //// that by finding the point in the centre of the edge. We know
        //// its component in the direction of the box's collision axis is zero
        //// (its a mid-point) and we determine which of the extremes in each
        //// of the other axes is closest.
        //Vector3 ptOnOneEdge = one.halfSize;
        //Vector3 ptOnTwoEdge = two.halfSize;
        //for (unsigned i = 0; i < 3; i++)
        //{
        //    if (i == oneAxisIndex) ptOnOneEdge[i] = 0;
        //    else if (one.getAxis(i) * axis > 0) ptOnOneEdge[i] = -ptOnOneEdge[i];

        //    if (i == twoAxisIndex) ptOnTwoEdge[i] = 0;
        //    else if (two.getAxis(i) * axis < 0) ptOnTwoEdge[i] = -ptOnTwoEdge[i];
        //}

        //// Move them into world coordinates (they are already oriented
        //// correctly, since they have been derived from the axes).
        //ptOnOneEdge = one.transform * ptOnOneEdge;
        //ptOnTwoEdge = two.transform * ptOnTwoEdge;

        //// So we have a point and a direction for the colliding edges.
        //// We need to find out point of closest approach of the two
        //// line-segments.
        //Vector3 vertex = contactPoint(
        //    ptOnOneEdge, oneAxis, one.halfSize[oneAxisIndex],
        //    ptOnTwoEdge, twoAxis, two.halfSize[twoAxisIndex],
        //    bestSingleAxis > 2
        //);

        //// We can fill the contact.
        //Contact* contact = data->contacts;

        //contact->penetration = pen;
        //contact->contactNormal = axis;
        //contact->contactPoint = vertex;
        //contact->setBodyData(one.body, two.body,
        //    data->friction, data->restitution);
        //data->addContacts(1);
        //return 1;
    }



    return 0;
}
#undef CHECK_OVERLAP


void Physics::Rigidbody::CalcHalfSize(const UINT & index)
{
    glm::vec3* vertices = componentManager->boxCollider[index].vertices;
    glm::vec3& halfSize = componentManager->boxCollider[index].size;

    glm::mat4& collideTransform = componentManager->boxCollider[index].transformMatrix;
    glm::mat4& offset = componentManager->boxCollider[index].offset;
    glm::mat4& transform = componentManager->transform[index].transformMatrix;

    //calc. collide transform matrix 
    collideTransform = transform * offset;
    //set vertices into world space
    vertices[0] = glm::vec3(-halfSize.x, -halfSize.y, -halfSize.z);
    vertices[1] = glm::vec3(-halfSize.x, -halfSize.y, +halfSize.z);
    vertices[2] = glm::vec3(-halfSize.x, +halfSize.y, -halfSize.z);
    vertices[3] = glm::vec3(-halfSize.x, +halfSize.y, +halfSize.z);
    vertices[4] = glm::vec3(+halfSize.x, -halfSize.y, -halfSize.z);
    vertices[5] = glm::vec3(+halfSize.x, -halfSize.y, +halfSize.z);
    vertices[6] = glm::vec3(+halfSize.x, +halfSize.y, -halfSize.z);
    vertices[7] = glm::vec3(+halfSize.x, +halfSize.y, +halfSize.z);

    for (size_t i = 0; i < 8; ++i)
    {
        vertices[i] = collideTransform * glm::vec4(vertices[i], 1.0f);
    }

    //Set max, min global vertices 
    glm::vec3 maxG, minG;
    maxG = minG = vertices[0];

    for (size_t i = 1; i < 8; ++i)
    {
        maxG.x = glm::max(vertices[i].x, maxG.x);
        minG.x = glm::min(vertices[i].x, minG.x);

        maxG.y = glm::max(vertices[i].y, maxG.y);
        minG.y = glm::min(vertices[i].y, minG.y);

        maxG.z = glm::max(vertices[i].z, maxG.z);
        minG.z = glm::min(vertices[i].z, minG.z);
    }

    componentManager->boxCollider[index].maxVertex = maxG;
    componentManager->boxCollider[index].minVertex = minG;
}
