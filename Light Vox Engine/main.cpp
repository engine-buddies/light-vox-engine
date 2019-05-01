#include "stdafx.h"
#include "engine.h"

#if defined(_WIN32) || defined(_WIN64)

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
    {
        //memory leak detections
#if defined(_DEBUG)
        _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
        _CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_DEBUG );
#endif
        Engine engine( hInstance );

        ThrowIfFailed( engine.InitSystems() );	//init all 
        ThrowIfFailed( engine.Run() );			//run that baby
    }

    return 0;
}

#else   //defined windows

int main() 
{
    assert( "Unimplemented main" );
    return 0;
}

#endif
