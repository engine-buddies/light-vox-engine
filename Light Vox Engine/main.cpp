#include "stdafx.h"
#include "engine.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	{
		//memory leak detections
#if defined(_DEBUG)
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
        _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
#endif
		Engine engine(hInstance);

		LV_PRINT_DEBUG("Initializing Systems");
		ThrowIfFailed(engine.InitSystems());	//init all 
		ThrowIfFailed(engine.Run());			//run that baby
	}

	//dump that info
#if defined(_DEBUG)
	_CrtDumpMemoryLeaks();
#endif
	return 0;
}