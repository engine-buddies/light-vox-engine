#pragma once
#include <DirectXMath.h>
#include "../Entity.h"
#include "../ComponentManager.h"

class Physics
{
public:
    /// <summary>
    /// Intializes physics system
    /// -sets gravity 
    /// -intializies transform matrix
    /// </summary>
    Physics();
    ~Physics();
    /// <summary>
    /// updates physics calls by solving collision, solving forces,
    /// integrating position, and calc. model to world matrix
    /// </summary>
    /// <param name="dt"></param>
    void Update( float dt );
    /// <summary>
    /// set a position for an entity
    /// </summary>
    /// <param name="pos"></param>
    /// <param name="e"></param>
    void Move( glm::vec3 pos, UINT index );
    /// <summary>
    /// Set quaternion roation for entity
    /// </summary>
    /// <param name="rotationAxis"></param>
    /// <param name="angle"></param>
    /// <param name="e"></param>
    void RotateAxisAngle( glm::vec3 rotationAxis,
        float angle,
        UINT index );

private:

    /// <summary>
    /// Solves Collision (aabb to aabb) 
    /// </summary>
    void Collide();
    /// <summary>
    /// Semi Implicit euler intergration for position
    /// </summary>
    /// <param name="dt"></param>
    void Integrate( float dt );
    /// <summary>
    /// Calc. total force
    /// </summary>
    void AccumlateForces();
    /// <summary>
    /// Calc model to world matrix
    /// </summary>
    void ModelToWorld();

    //helper function to check box to box intersect
    inline bool BoxIntersect( glm::vec3 posA,
        glm::vec3 posB,
        glm::vec3 sizeA,
        glm::vec3 sizeB
    );


    glm::vec3 gravity;
    ComponentManager* componentManager;
};