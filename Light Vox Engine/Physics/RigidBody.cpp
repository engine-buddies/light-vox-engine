#include "RigidBody.h"

namespace {

    /// <summary>
    /// Helper method to check for the intersection of two bounding boxes given
    /// the min and max global values
    /// </summary>
    /// <param name="maxA"></param>
    /// <param name="minA"></param>
    /// <param name="maxB"></param>
    /// <param name="minB"></param>
    /// <returns></returns>
    inline bool BoxIntersectBox(glm::vec3& maxA, glm::vec3& minA, glm::vec3& maxB, glm::vec3& minB)
    {
        //check for intersection
        return (minA.x < maxB.x && maxA.x > minB.x) &&
            (minA.y < maxB.y && maxA.y > minB.y) &&
            (minA.z < maxB.z && maxA.z > minB.z);
    }

    //gets a basis vector from the transformation matrix
    inline glm::vec3 GetAxisVector(int i, const glm::mat4& mat)
    {
        return glm::vec3(mat[0][i], mat[1][i], mat[2][i]);
    }


    //helper method used for SAT
    //gets the scalar projection of bounding box size onto an axis
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

    //helper method used for SAT
    //finds the penetration depth on a axis 
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

    //helper method used for SAT
    //determines if there an overlap on an axis
    inline bool tryAxis(
        const EntityComponents::BoxCollider& one,
        const EntityComponents::BoxCollider& two,
        glm::vec3 axis,
        const glm::vec3 toCenter,
        uint32_t axisIndex,
        float& smallestPenetration,
        uint32_t& smallestCase)
    {
        if (glm::length2(axis) < FLT_EPSILON)
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

    //helper method to find contact information
    //for vertex vertex collision in SAT
    inline void FillPointFaceBoxBox(
        const EntityComponents::BoxCollider& one,
        const EntityComponents::BoxCollider& two,
        const glm::vec3& toCenter,
        EntityComponents::Contacts* contactData,
        unsigned best,
        float penetration
    )
    {
        //determine which direction the axis of collision is facing
        glm::vec3 normal = GetAxisVector(best, one.transformMatrix);
        if (glm::dot(normal, toCenter) > 0)
        {
            normal = normal * -1.0f;
        }

        //work out which vertex of box two we're colliding with
        glm::vec3 vertex = two.size;
        if (glm::dot(GetAxisVector(0, two.transformMatrix), normal) < 0)
            vertex.x = -vertex.x;

        if (glm::dot(GetAxisVector(1, two.transformMatrix), normal) < 0)
            vertex.y = -vertex.y;

        if (glm::dot(GetAxisVector(2, two.transformMatrix), normal) < 0)
            vertex.z = -vertex.z;

        //create the contact data 
        contactData[contactData->contactsFound].contactNormal = normal;
        contactData[contactData->contactsFound].penetration = penetration;
        contactData[contactData->contactsFound].contactPoint = two.transformMatrix * glm::vec4(vertex, 1.0f);
        contactData[contactData->contactsFound].bodyPair = { one.tag, two.tag };
    }

    //helper method for finding contact points for edge to edge
    //collision in SAT
    inline glm::vec3 FindContactPoint(
        const glm::vec3& pOne,
        const glm::vec3& dOne,
        float oneSize,
        const glm::vec3& pTwo,
        const glm::vec3& dTwo,
        float twoSize,
        bool useOne)
    {
        glm::vec3 toSt, cOne, cTwo;
        float dpStaOne, dpStaTwo, dpOneTwo, smOne, smTwo;
        float denom, mua, mub;

        smOne = glm::length2(dOne);
        smTwo = glm::length2(dTwo);
        dpOneTwo = glm::dot(dTwo, dOne);

        toSt = pOne - pTwo;
        dpStaOne = glm::dot(dOne, toSt);
        dpStaTwo = glm::dot(dTwo, toSt);

        denom = smOne * smTwo - dpOneTwo * dpOneTwo;

        // Zero denominator indicates parrallel lines
        if (glm::abs(denom) < FLT_EPSILON) {
            return useOne ? pOne : pTwo;
        }

        mua = (dpOneTwo * dpStaTwo - smTwo * dpStaOne) / denom;
        mub = (smOne * dpStaTwo - dpOneTwo * dpStaOne) / denom;

        // If either of the edges has the nearest point out
        // of bounds, then the edges aren't crossed, we have
        // an edge-face contact. Our point is on the edge, which
        // we know from the useOne parameter.
        if (mua > oneSize ||
            mua < -oneSize ||
            mub > twoSize ||
            mub < -twoSize)
        {
            return useOne ? pOne : pTwo;
        }
        else
        {
            cOne = pOne + dOne * mua;
            cTwo = pTwo + dTwo * mub;

            return cOne * 0.5f + cTwo * 0.5f;
        }
    }
}

Physics::Rigidbody::Rigidbody()
{
    componentManager = ECS::ComponentManager::GetInstance();
}

Physics::Rigidbody::~Rigidbody()
{
}

void Physics::Rigidbody::Pos(glm::vec3&& pos, const size_t& index)
{
    componentManager->transform[index].pos = pos;
}

void Physics::Rigidbody::RotateAxisAngle(glm::vec3&& rotationAxis, float angle, const size_t& index)
{
    componentManager->transform[index].orientation = glm::angleAxis(glm::degrees(angle), rotationAxis);
}

void Physics::Rigidbody::Velocity(glm::vec3&& vel, const size_t& index)
{
    componentManager->bodyProperties[index].velocity = vel;
}

void Physics::Rigidbody::Acceleration(glm::vec3&& accel, const size_t& index)
{
    componentManager->bodyProperties[index].acceleration = accel;
}

void Physics::Rigidbody::Force(glm::vec3&& force, const size_t& index)
{
    componentManager->bodyProperties[index].force = force;
}

void Physics::Rigidbody::Mass(float mass, const size_t& index)
{
    if (mass = 0.0f)
        mass = 1.0f;

    componentManager->bodyProperties[index].mass = mass;
    componentManager->bodyProperties[index].invMass = (1 / mass);
}

void Physics::Rigidbody::Scale(glm::vec3& scale, const size_t& index)
{
    assert("Hey you can't scale a voxel");
}

void Physics::Rigidbody::BoxColliderSize(glm::vec3& size, const size_t& index)
{
    componentManager->boxCollider[index].size = size;
}

void Physics::Rigidbody::InertiaTensor(glm::mat3 & inertiaTensor, const size_t& index)
{
    componentManager->bodyProperties[index].inertiaTensor = inertiaTensor;
}

void Physics::Rigidbody::SetAwake(const bool awake, const size_t& index)
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
    const size_t& index)
{
    glm::mat4& transformMatrix = componentManager->transformMatrix[index].transformMatrix;
    glm::vec3& pos = componentManager->transform[index].pos;
    glm::vec3& _force = componentManager->bodyProperties[index].force;
    glm::vec3& _torque = componentManager->bodyProperties[index].torque;

    glm::vec3 pt = transformMatrix * glm::vec4(point, 0.0f);
    pt -= pos;

    _force += force;
    _torque += glm::cross(pt, _torque);

    componentManager->bodyProperties[index].isAwake = true;
}


bool Physics::Rigidbody::IntersectBoxBox(const size_t& entityA, const size_t& entityB)
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
    return BoxIntersectBox(maxA, minA, maxB, minB);
#endif
}


// preprocessor definition is only used as a convenience
// in the boxAndBox contact generation method.
#define CHECK_OVERLAP(axis, index) \
    if (!tryAxis(one, two, (axis), toCenter, (index), pen, best)) return 0;

int Physics::Rigidbody::CollideBoxBox(const size_t& entityA, const size_t& entityB)
{
    glm::vec3& posA = componentManager->transform[entityA].pos;
    glm::vec3& posB = componentManager->transform[entityB].pos;

    EntityComponents::BoxCollider& one = componentManager->boxCollider[entityA];
    EntityComponents::BoxCollider& two = componentManager->boxCollider[entityB];

    EntityComponents::Contacts* contacts = componentManager->contacts;

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
        FillPointFaceBoxBox(one, two, toCenter, contacts, best, pen);
        ++contacts->contactsFound;
        return 1;
    }
    else if (best < 6)
    {
        // We've got a vertex of box one on a face of box two
        FillPointFaceBoxBox(two, one, toCenter*-1.0f, contacts, best - 3, pen);
        ++contacts->contactsFound;
        return 1;
    }
    else
    {
        //// We've got an edge-edge contact. Find out which axes
        best -= 6;
        unsigned oneAxisIndex = best / 3;
        unsigned twoAxisIndex = best % 3;
        glm::vec3 oneAxis = GetAxisVector(oneAxisIndex, one.transformMatrix);
        glm::vec3 twoAxis = GetAxisVector(twoAxisIndex, two.transformMatrix);
        glm::vec3 axis = glm::cross(oneAxis, twoAxis);
        axis = glm::normalize(axis);

        //if the axis point from box one to box two 
        if (glm::dot(axis, toCenter) > 0)
            axis = axis * -1.0f;

        //determine which extermes in each of the axes is closest 
        glm::vec3 ptOnOneEdge = one.size;
        glm::vec3 ptOnTwoEdge = two.size;

        for (unsigned int i = 0; i < 3; ++i)
        {
            if (i == oneAxisIndex)
                ptOnOneEdge[i] = 0.0f;
            else if (glm::dot(GetAxisVector(i, one.transformMatrix), axis) > 0)
                ptOnOneEdge[i] = -ptOnOneEdge[i];

            if (i == twoAxisIndex)
                ptOnTwoEdge[i] = 0.0f;
            else if (glm::dot(GetAxisVector(i, two.transformMatrix), axis) > 0)
                ptOnTwoEdge[i] = -ptOnTwoEdge[i];
        }

        //move them into world coords.
        ptOnOneEdge = one.transformMatrix * glm::vec4(ptOnOneEdge, 1.0f);
        ptOnTwoEdge = two.transformMatrix * glm::vec4(ptOnTwoEdge, 1.0f);

        //find point closest to the two line-segments 
        glm::vec3 vertex = FindContactPoint(
            ptOnOneEdge,
            oneAxis,
            one.size[oneAxisIndex],
            ptOnTwoEdge,
            twoAxis,
            two.size[twoAxisIndex],
            bestSingleAxis > 2
        );

        contacts[contacts->contactsFound].penetration = pen;
        contacts[contacts->contactsFound].contactNormal = axis;
        contacts[contacts->contactsFound].contactPoint = vertex;
        contacts[contacts->contactsFound].bodyPair = { one.tag, two.tag };
        ++contacts->contactsFound;
        return 1;
    }
    return 0;
}
#undef CHECK_OVERLAP


void Physics::Rigidbody::CalcHalfSize(const size_t & index)
{
    glm::vec3* vertices = componentManager->boxCollider[index].vertices;
    glm::vec3& halfSize = componentManager->boxCollider[index].size;

    glm::mat4& collideTransform = componentManager->boxCollider[index].transformMatrix;
    glm::mat4& offset = componentManager->boxCollider[index].offset;
    glm::mat4& transform = componentManager->transformMatrix[index].transformMatrix;

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