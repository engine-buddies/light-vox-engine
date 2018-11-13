#pragma once
#include <Windows.h>
#include <vector>
#include "Graphics/GraphicsCore.h"
#include "Graphics/DebugRenderer.h"
#include "GameTime.h"
#include "ECS/EntityManager.h"
#include "ECS/Entity.h"
#include "Physics/Solver.h"
#include "Physics/RigidBody.h"
#include "JobSystem/JobManager.h"
#include "JobSystem/ConcurrentQueue.h"
#include "Input/InputManager.h"

/*
It's basically our entire engine (handles life cycle and windows related stuff)
*/
class Engine
{
public:
#if defined(_WIN32) || defined(_WIN64)
    Engine( HINSTANCE hInstance );
#else
    Engine();
#endif

    ~Engine();

    LV_RESULT InitSystems();
    LV_RESULT Run();
    void Quit();

    //note: would prefer to not do our #defines this way, but not sure if every implementation will share same/abstractible interfaces
#if defined(_WIN32) || defined(_WIN64)

    //OS-level message handling
    static LRESULT CALLBACK ProcessMessage( LV_HANDLE hWindow, uint32_t message, WPARAM wParam, LPARAM lParam );

    LRESULT HandleEvents( LV_HANDLE hWindow, uint32_t message, WPARAM wParam, LPARAM lParam );

    //responding to events
    void OnResize( uint32_t width, uint32_t height );

#endif

    
private:
#if defined(_WIN32) || defined(_WIN64)
    HINSTANCE hInstance;    //instance for windows type stuff
#else
    //hInstance for other class?
#endif
    static Engine* engineInstance;
    LV_HANDLE hWindow;
    char* windowTitle;
    float windowWidth;
    float windowHeight;

    // Input
    Input::InputManager* inputManager = nullptr;

    //ECS
    ECS::EntityManager* entityManager = nullptr;
    ECS::ComponentManager* componentManager = nullptr;

    //Systems
    Graphics::GraphicsCore* graphics = nullptr;
    Graphics::Camera* camera = nullptr;
    GameTime* time = nullptr;
    Jobs::JobManager* jobManager = nullptr;

    Graphics::DebugRenderer* debugRenderer = nullptr;

    //Physics 
    Physics::Solver* physics = nullptr;
    Physics::Rigidbody* rigidBody = nullptr;

    LV_RESULT InitWindow();

    void UsingInputFunc();
    inline void Update();

    /*Debug function to create a console window*/
    void CreateConsoleWindow( int bufferLines, int bufferColumns, int windowLines, int windowColumns );
};
