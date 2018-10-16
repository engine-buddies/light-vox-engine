#pragma once
#include <Windows.h>
#include <vector>
#include "GraphicsCore.h"
#include "GameTime.h"
#include "EntityManager.h"
#include "Entity.h"
#include "Physics.h"
#include "JobManager.h"
#include "ComponentManager.h"

/*
It's basically our entire engine (handles life cycle and windows related stuff)
*/
class Engine
{
public:
	Engine(HINSTANCE hInstance);
	~Engine();

	HRESULT InitSystems();
	HRESULT Run();
	void Quit();

	//OS-level message handling
	static LRESULT CALLBACK ProcessMessage(HWND hWindow, UINT message, WPARAM wParam, LPARAM lParam);

	//responding to events
	LRESULT HandleEvents(HWND hWindow, UINT message, WPARAM wParam, LPARAM lParam);
	void OnResize(UINT width, UINT height);
	void OnMouseDown(WPARAM buttonState, int x, int y);
	void OnMouseUp(WPARAM buttonState, int x, int y);
	void OnMouseMove(WPARAM buttonState, int x, int y);

private:
	static Engine* engineInstance;
	HINSTANCE hInstance;
	HWND hWindow;
	char* windowTitle;
	float windowWidth;
	float windowHeight;

    //ECS
    EntityManager* entityManager;
    std::vector<Entity> entityList;
    ComponentManager* componentManager;

    //Systems
	GraphicsCore* graphics;
    Camera* camera;
    GameTime* time;
    Physics* physics;
   
	HRESULT InitWindow();

#if defined(_DEBUG)
	/*Debug function to create a console window*/
	void CreateConsoleWindow(int bufferLines, int bufferColumns, int windowLines, int windowColumns);
#endif

};
