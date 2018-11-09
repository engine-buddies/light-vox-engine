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
    Engine( HINSTANCE hInstance );
    ~Engine();

    HRESULT InitSystems();
    HRESULT Run();
    void Quit();

    //OS-level message handling
    static LRESULT CALLBACK ProcessMessage( HWND hWindow, UINT message, WPARAM wParam, LPARAM lParam );

    //responding to events
    LRESULT HandleEvents( HWND hWindow, UINT message, WPARAM wParam, LPARAM lParam );
    void OnResize( UINT width, UINT height );
    
private:
    static Engine* engineInstance;
    HINSTANCE hInstance;
    HWND hWindow;
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

    HRESULT InitWindow();


    void UsingInputFunc();

#if defined(_DEBUG)
    /*Debug function to create a console window*/
    void CreateConsoleWindow( int bufferLines, int bufferColumns, int windowLines, int windowColumns );
#endif

};
