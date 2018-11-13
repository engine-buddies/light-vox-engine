/// <summary>
/// By Arturo Kuang
/// </summary>
#pragma once
#include "../ECS/Entity.h"
#include "../ECS/ComponentManager.h"
#include "../JobSystem/JobManager.h"

namespace Physics
{
    class Solver
    {
    public:
        /// <summary>
        /// Intializes physics system
        /// -sets gravity 
        /// -intializies transform matrix
        /// </summary>
        Solver();
        ~Solver();
        /// <summary>
        /// updates physics calls by solving collision, solving forces,
        /// integrating position, and calc. model to world matrix
        /// </summary>
        /// <param name="dt"></param>
        void Update( float dt );

    private:

        /// <summary>
        /// Solves Collision (aabb to aabb) 
        /// </summary>
        void Collide();
        /// <summary>
        /// Semi Implicit euler intergration for position
        /// </summary>
        /// <param name="dt"></param>
        void Integrate( void* args, int index );
        /// <summary>
        /// Calc. total force
        /// </summary>
        void AccumlateForces( void* args, int index );
        /// <summary>
        /// Calc model to world matrix
        /// </summary>
        void ModelToWorld( void* args, int index );

        //helper function to check box to box intersect
        inline bool BoxIntersect( glm::vec3 posA,
            glm::vec3 posB,
            glm::vec3 sizeA,
            glm::vec3 sizeB
        );


        glm::vec3 gravity;
        ECS::ComponentManager* componentManager;
        Jobs::JobManager* jobManager = nullptr;


        struct PhysicsArguments
        {
            float DeltaTime;

            int StartElem;
            int EndElm;

            std::promise<void>* jobPromise;
        };

        PhysicsArguments* a_argument = nullptr;
        PhysicsArguments* b_argument = nullptr;

    };
};