#include "engine.h"
#include "stdafx.h"
#include "Graphics/Camera.h"

#include <WindowsX.h>
#if defined(_DEBUG)
#include <sstream>	//for creating a console
#endif

// DEBUG CODE

Engine* Engine::engineInstance = nullptr;


/* LIFE CYCLE */

Engine::Engine( HINSTANCE hInstance )
{
    this->hInstance = hInstance;
    Engine::engineInstance = this;
    windowWidth = 1280;
    windowHeight = 720;
    windowTitle = "STRUGGLE BUS";
    hWindow = 0;

    debugRenderer = Graphics::DebugRenderer::GetInstance();
    camera = new Graphics::Camera();
}

Engine::~Engine()
{
    time->ReleaseInstance();
    delete graphics;
    delete camera;
    delete physics;
    delete rigidBody;

    // Releases the instance of the entity manager
    entityManager->ReleaseInstance();
    componentManager->ReleaseInstance();
    Jobs::JobManager::ReleaseInstance();
    Graphics::DebugRenderer::ReleaseInstance();
}

HRESULT Engine::InitWindow()
{
    LV_PRINT_DEBUG( "Initializing a window..." );
#if defined(_DEBUG)
    CreateConsoleWindow( 500, 120, 32, 120 );
#endif

    //initialize a window
    WNDCLASSEX windowClass = { };
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = Engine::ProcessMessage;
    windowClass.cbSize = sizeof( WNDCLASSEX );
    windowClass.cbClsExtra = 0;
    windowClass.cbWndExtra = 0;
    windowClass.hInstance = hInstance;
    windowClass.hIcon = LoadIcon( NULL, IDI_APPLICATION );
    windowClass.hCursor = LoadCursor( NULL, IDC_ARROW );
    windowClass.hbrBackground = (HBRUSH) GetStockObject( BLACK_BRUSH );
    windowClass.lpszMenuName = NULL;
    windowClass.lpszClassName = "Direct3DWindowClass";

    //register and print out
    if ( !RegisterClassEx( &windowClass ) )
    {
        DWORD error = GetLastError();
        if ( error != ERROR_CLASS_ALREADY_EXISTS )
            return HRESULT_FROM_WIN32( error );
    }

    //used for adjusting window correctly
    RECT clientRect;
    RECT desktopRect;

    SetRect( &clientRect, 0, 0, static_cast<LONG>( windowWidth ), static_cast<LONG>( windowHeight ) );
    AdjustWindowRect( &clientRect, WS_OVERLAPPEDWINDOW, false );

    GetClientRect( GetDesktopWindow(), &desktopRect );

    hWindow = CreateWindow(
        windowClass.lpszClassName,
        windowTitle,
        WS_OVERLAPPEDWINDOW,
        ( desktopRect.right / 2 ) - ( clientRect.right / 2 ),
        ( desktopRect.bottom / 2 ) - ( clientRect.bottom / 2 ),
        clientRect.right - clientRect.left,
        clientRect.bottom - clientRect.top,
        0,
        0,
        hInstance,
        0
    );

    if ( hWindow == NULL )
        return HRESULT_FROM_WIN32( GetLastError() );

    ShowWindow( hWindow, SW_SHOW );
    return S_OK;
}

HRESULT Engine::InitSystems()
{
    InitWindow();
    graphics = new Graphics::GraphicsCore( hWindow, static_cast<UINT>( windowWidth ), static_cast<UINT>( windowHeight ) );
    //Physics
    physics = new Physics::Solver();
    rigidBody = new Physics::Rigidbody();
    //Time
    time = GameTime::GetInstance();

    // Calling get instance will create the entity manager
    entityManager = ECS::EntityManager::GetInstance();
    componentManager = ECS::ComponentManager::GetInstance();

    Jobs::JobManager* man = Jobs::JobManager::GetInstance();
    // Add any jobs you need here, like this: 
    //man->AddJobA( &FunctionName, void* args );

    man = nullptr;

    ThrowIfFailed( graphics->Init() );
    time->Init();
    entityManager->Init();

    for ( size_t i = 0; i < LV_MAX_INSTANCE_COUNT; ++i )
    {
        entityManager->Create_Entity();
    }


    //DEBUG:: INTIALIZE ENTITY POSSITIONS
    static bool init = false;
    static int count = static_cast<int>( sqrtf( static_cast<float>( LV_MAX_INSTANCE_COUNT ) ) );
    static float rotation = 0.001f;
    {

        float x = static_cast <float>( -count );
        float y = static_cast <float>( -count );
        float z = 0;
        for ( int i = 0; i < count; ++i )
        {
            for ( int j = 0; j < count; ++j )
            {
                int index = i * count + j;
                UINT entityID = entityManager->Get_Entity( index ).index;
                rigidBody->Pos( glm::vec3( x + rotation, y, z ), entityID );
                rigidBody->RotateAxisAngle( glm::vec3( .0f, 1.0f, .0f ), rotation, entityID );

                x += 2;
            }

            y += 2;
            x = static_cast <float>( -count );
        }
    }

    Input::InputManager::BindAxis(
        "TestAxis",
        this,
        &Engine::UsingInputFunc
    );

    return S_OK;
}

HRESULT Engine::Run()
{
    MSG msg = { };

    while ( msg.message != WM_QUIT )
    {
        if ( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
        {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
        else
        {
            //DEBUG collision code 
            float x = sinf( time->GetTotalFloatTime() ) / 100.0f;
            for ( size_t i = 0; i < LV_MAX_INSTANCE_COUNT; ++i )
            {
                componentManager->transform[ i ].pos.x += x;
            }

            //DEBUG CODE for debug wireframe renderer
            glm::mat4x4 transform = glm::translate( glm::vec3( 1, 1, 0 ) );
            glm::vec3 scale = glm::float3( 1, 1, 1 );
            glm::vec3 color = glm::float3( 1, 0, 0 );

            debugRenderer->AddCube( transform, scale, color );
            transform = glm::identity<glm::mat4x4>();
            color = glm::float3( 0, 0, 1 );
            debugRenderer->AddCube( transform, scale, color );

            //DEBUG CODE for basic camera update
            glm::vec3 pos = glm::vec3( 0.f, 0.f, -5.f );
            glm::vec3 forward = glm::vec3( 0.f, 0.f, 1.f );
            glm::vec3 up = glm::vec3( 0.f, 1.f, 0.f );
            camera->SetTransform( pos, forward, up );

            physics->Update( time->GetTotalFloatTime() );
            //memcpy( transforms, componentManager->transform, LV_MAX_INSTANCE_COUNT * sizeof( glm::mat4x4 ) );
            //for ( size_t i = 0; i < LV_MAX_INSTANCE_COUNT; ++i )
            //{
            //    transforms[ i ] = glm::transpose( transforms[ i ] );
            //}

            graphics->Update( reinterpret_cast<glm::mat4x4_packed *>( componentManager->transformMatrix), camera );
            graphics->Render();
            time->UpdateTimer();
            debugRenderer->ClearCubes();
        }
    }
    return (HRESULT) msg.wParam;
}

void Engine::Quit()
{
    PostMessage( this->hWindow, WM_CLOSE, NULL, NULL );
}

/* EVENT PROCESSING */

LRESULT Engine::ProcessMessage( HWND hWindow, UINT message, WPARAM wParam, LPARAM lParam )
{
    return Engine::engineInstance->HandleEvents( hWindow, message, wParam, lParam );
}

LRESULT Engine::HandleEvents( HWND hWindow, UINT message, WPARAM wParam, LPARAM lParam )
{
    switch ( message )
    {
        //handle mouse down/up
        case WM_MOUSEMOVE:
            //OnMouseMove( wParam, GET_X_LPARAM( lParam ), GET_Y_LPARAM( lParam ) );
            return 0;
        case WM_LBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_RBUTTONDOWN:
            //OnMouseDown( wParam, GET_X_LPARAM( lParam ), GET_Y_LPARAM( lParam ) );
            return 0;
        case WM_LBUTTONUP:
        case WM_MBUTTONUP:
        case WM_RBUTTONUP:
            //OnMouseUp( wParam, GET_X_LPARAM( lParam ), GET_Y_LPARAM( lParam ) );
            return 0;

            //handle resizing
        case WM_SIZE:
            if ( wParam == SIZE_MINIMIZED )
                return 0;
            OnResize( LOWORD( lParam ), HIWORD( lParam ) );
            return 0;

            //the min window size
        case WM_GETMINMAXINFO:
            ( (MINMAXINFO*) lParam )->ptMinTrackSize.x = 200;
            ( (MINMAXINFO*) lParam )->ptMinTrackSize.y = 200;
            return 0;

            //prevent beeping when 'alt-enter' to fullscreen
        case WM_MENUCHAR:
            return MAKELRESULT( 0, MNC_CLOSE );

            //quit out
        case WM_DESTROY:
            PostQuitMessage( 0 );
            return 0;
    }

    //default windows message processing
    return DefWindowProc( hWindow, message, wParam, lParam );
}

void Engine::OnResize( UINT width, UINT height )
{
    windowWidth = static_cast<float>( width );
    windowHeight = static_cast<float>( height );
}

/* HELPERS */

void Engine::UsingInputFunc( float axis )
{
    DEBUG_PRINT( "Inside of the function that uses it: %f\n", axis );

}

#if defined(_DEBUG)
void Engine::CreateConsoleWindow( int bufferLines, int bufferColumns, int windowLines, int windowColumns )
{
    // Our temp console info struct
    CONSOLE_SCREEN_BUFFER_INFO coninfo = { };

    // Get the console info and set the number of lines
    AllocConsole();
    GetConsoleScreenBufferInfo( GetStdHandle( STD_OUTPUT_HANDLE ), &coninfo );
    coninfo.dwSize.Y = bufferLines;
    coninfo.dwSize.X = bufferColumns;
    SetConsoleScreenBufferSize( GetStdHandle( STD_OUTPUT_HANDLE ), coninfo.dwSize );

    SMALL_RECT rect;
    rect.Left = 0;
    rect.Top = 0;
    rect.Right = windowColumns;
    rect.Bottom = windowLines;
    SetConsoleWindowInfo( GetStdHandle( STD_OUTPUT_HANDLE ), TRUE, &rect );

    FILE *stream;
    freopen_s( &stream, "CONIN$", "r", stdin );
    freopen_s( &stream, "CONOUT$", "w", stdout );
    freopen_s( &stream, "CONOUT$", "w", stderr );

    // Prevent accidental console window close
    HWND consoleHandle = GetConsoleWindow();
    HMENU hmenu = GetSystemMenu( consoleHandle, FALSE );
    EnableMenuItem( hmenu, SC_CLOSE, MF_GRAYED );
}
#endif

