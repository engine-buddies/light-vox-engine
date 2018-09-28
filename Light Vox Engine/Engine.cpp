#include "engine.h"
#include "stdafx.h"

#include <WindowsX.h>
#if defined(_DEBUG)
#include <sstream>	//for creating a console
#endif

Engine* Engine::engineInstance = nullptr;

/* LIFE CYCLE */

Engine::Engine(HINSTANCE hInstance)
{
	this->hInstance = hInstance;
	Engine::engineInstance = this;
	windowWidth = 800;
	windowHeight = 500;
	windowTitle = "STRUGGLE BUS";
	hWindow = 0;
}

Engine::~Engine()
{
    time->ReleaseInstance();
	delete graphics;
}

HRESULT Engine::InitWindow()
{
	LV_PRINT_DEBUG("Initializing a window...");
#if defined(_DEBUG)
	CreateConsoleWindow(500, 120, 32, 120);
#endif

	//initialize a window
	WNDCLASSEX windowClass = { };
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = Engine::ProcessMessage;
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.cbClsExtra = 0;
	windowClass.cbWndExtra = 0;
	windowClass.hInstance = hInstance;
	windowClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	windowClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	windowClass.lpszMenuName = NULL;
	windowClass.lpszClassName = "Direct3DWindowClass";

	//register and print out
	if (!RegisterClassEx(&windowClass))
	{
		DWORD error = GetLastError();
		if (error != ERROR_CLASS_ALREADY_EXISTS)
			return HRESULT_FROM_WIN32(error);
	}

	//used for adjusting window correctly
	RECT clientRect;
	RECT desktopRect;

	SetRect(&clientRect, 0, 0, static_cast<LONG>(windowWidth), static_cast<LONG>(windowHeight));
	AdjustWindowRect(&clientRect, WS_OVERLAPPEDWINDOW, false);

	GetClientRect(GetDesktopWindow(), &desktopRect);

	hWindow = CreateWindow(
		windowClass.lpszClassName,
		windowTitle,
		WS_OVERLAPPEDWINDOW,
		(desktopRect.right / 2) - (clientRect.right / 2),
		(desktopRect.bottom / 2) - (clientRect.bottom / 2),
		clientRect.right - clientRect.left,
		clientRect.bottom - clientRect.top,
		0,
		0,
		hInstance,
		0
	);

	if (hWindow == NULL)
		return HRESULT_FROM_WIN32(GetLastError());

	ShowWindow(hWindow, SW_SHOW);
	return S_OK;
}

HRESULT Engine::InitSystems()
{
	InitWindow();
	graphics = new GraphicsCore(hWindow, windowWidth, windowHeight);
    time = Time::GetInstance();

	ThrowIfFailed(graphics->Init());
    time->Init();

	return S_OK;
}

HRESULT Engine::Run()
{
	MSG msg = { };
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			graphics->Update();
			graphics->Render();
            time->UpdateTimer();
		}
	}

	return (HRESULT)msg.wParam;
}

void Engine::Quit()
{
	PostMessage(this->hWindow, WM_CLOSE, NULL, NULL);
}

/* EVENT PROCESSING */

LRESULT Engine::ProcessMessage(HWND hWindow, UINT message, WPARAM wParam, LPARAM lParam)
{
	return Engine::engineInstance->HandleEvents(hWindow, message, wParam, lParam);
}


LRESULT Engine::HandleEvents(HWND hWindow, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		//handle mouse down/up
	case WM_MOUSEMOVE:
		OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;

		//handle resizing
	case WM_SIZE:
		if (wParam == SIZE_MINIMIZED)
			return 0;
		OnResize(LOWORD(lParam), HIWORD(lParam));
		return 0;

		//the min window size
	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
		return 0;

		//prevent beeping when 'alt-enter' to fullscreen
	case WM_MENUCHAR:
		return MAKELRESULT(0, MNC_CLOSE);

		//quit out
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	//default windows message processing
	return DefWindowProc(hWindow, message, wParam, lParam);
}

void Engine::OnResize(UINT width, UINT height)
{
	windowWidth = static_cast<float>(width);
	windowHeight = static_cast<float>(height);
}

void Engine::OnMouseDown(WPARAM buttonState, int x, int y)
{
}

void Engine::OnMouseUp(WPARAM buttonState, int x, int y)
{
}

void Engine::OnMouseMove(WPARAM buttonState, int x, int y)
{
}

/* HELPERS */

#if defined(_DEBUG)
void Engine::CreateConsoleWindow(int bufferLines, int bufferColumns, int windowLines, int windowColumns)
{
	// Our temp console info struct
	CONSOLE_SCREEN_BUFFER_INFO coninfo = { };

	// Get the console info and set the number of lines
	AllocConsole();
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &coninfo);
	coninfo.dwSize.Y = bufferLines;
	coninfo.dwSize.X = bufferColumns;
	SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), coninfo.dwSize);

	SMALL_RECT rect;
	rect.Left = 0;
	rect.Top = 0;
	rect.Right = windowColumns;
	rect.Bottom = windowLines;
	SetConsoleWindowInfo(GetStdHandle(STD_OUTPUT_HANDLE), TRUE, &rect);

	FILE *stream;
	freopen_s(&stream, "CONIN$", "r", stdin);
	freopen_s(&stream, "CONOUT$", "w", stdout);
	freopen_s(&stream, "CONOUT$", "w", stderr);

	// Prevent accidental console window close
	HWND consoleHandle = GetConsoleWindow();
	HMENU hmenu = GetSystemMenu(consoleHandle, FALSE);
	EnableMenuItem(hmenu, SC_CLOSE, MF_GRAYED);
}
#endif

