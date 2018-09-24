#include "stdafx.h"

#include "engine.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	{
#if defined(_DEBUG)
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
		Engine engine(hInstance);

		LV_PRINT_DEBUG("Initializing Systems");
		ThrowIfFailed(engine.InitSystems());
		ThrowIfFailed(engine.Run());
	}

	_CrtDumpMemoryLeaks();
	return 0;
}