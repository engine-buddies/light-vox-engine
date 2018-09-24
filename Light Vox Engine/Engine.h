#pragma once
#include <Windows.h>
#include <WindowsX.h>
#include "GraphicsCore.h"

#if defined(_DEBUG)
#include <sstream>	//for creating a window
#endif


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

	GraphicsCore* graphics;

	HRESULT InitWindow();

#if defined(_DEBUG)
	/*Debug function to create a console window*/
	void CreateConsoleWindow(int bufferLines, int bufferColumns, int windowLines, int windowColumns);
#endif

};

