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
    void OnMouseDown( WPARAM buttonState, int x, int y );
    void OnMouseUp( WPARAM buttonState, int x, int y );
    void OnMouseMove( WPARAM buttonState, int x, int y );

private:
    static Engine* engineInstance;
    HINSTANCE hInstance;
    HWND hWindow;
    char* windowTitle;
    float windowWidth;
    float windowHeight;

    //ECS
    ECS::EntityManager* entityManager;
    ECS::ComponentManager* componentManager;

    //Systems
    GraphicsCore* graphics;
    Camera* camera;
    GameTime* time;

    DebugRenderer* debugRenderer;

    //Physics 
    Physics::Solver* physics;
    Physics::Rigidbody* rigidBody;

    HRESULT InitWindow();

#if defined(_DEBUG)
    /*Debug function to create a console window*/
    void CreateConsoleWindow( int bufferLines, int bufferColumns, int windowLines, int windowColumns );
#endif

};
