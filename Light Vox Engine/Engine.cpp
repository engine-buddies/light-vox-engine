#include "engine.h"
#include "stdafx.h"
#include "Graphics/Camera.h"

#if defined(_WIN32) || defined(_WIN64)
#include <WindowsX.h>
#endif

// DEBUG CODE
//#if defined(_DEBUG)   //we'll uncomment when we have imgui working
#include <sstream>	//for creating a console
//#endif

Engine* Engine::engineInstance = nullptr;

/* LIFE CYCLE */
Engine::Engine( LV_INSTANCE hInstance )
{
    if ( hInstance != nullptr )
    {
        this->hInstance = hInstance;
        Engine::engineInstance = this;
        windowWidth = 1280;
        windowHeight = 720;
        windowTitle = "STRUGGLE BUS";
        hWindow = 0;
    }
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
    Input::InputManager::ReleaseInstance();
}

#if defined(_WIN32) || defined(_WIN64)
LV_RESULT Engine::InitWindow()
{
    CreateConsoleWindow( 500, 120, 32, 120 );

    //initialize a window
    DEBUG_PRINT( "Initializing a window..." );
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

#endif

LV_RESULT Engine::InitSystems()
{
    InitWindow();
    graphics = new Graphics::GraphicsCore( hWindow, static_cast<uint32_t>( windowWidth ), static_cast<uint32_t>( windowHeight ) );
    debugRenderer = Graphics::DebugRenderer::GetInstance();
    camera = new Graphics::Camera();

    physics = new Physics::Solver();
    rigidBody = new Physics::Rigidbody();
    time = GameTime::GetInstance();

    // Calling get instance will create the entity manager
    entityManager = ECS::EntityManager::GetInstance();
    componentManager = ECS::ComponentManager::GetInstance();

    jobManager = Jobs::JobManager::GetInstance();
    inputManager = Input::InputManager::GetInstance();

    // Bind an axis to the input man
    inputManager->BindAxis( Input::InputType::Fire, this, &Engine::UsingInputFunc );

    ThrowIfFailed( graphics->Init() );
    time->Init();
    entityManager->Init();

    for ( size_t i = 0; i < LV_MAX_INSTANCE_COUNT; ++i )
    {
        entityManager->Create_Entity();
    }

    //DEBUG:: INTIALIZE ENTITY POSSITIONS
    {
        static int count = static_cast<int>( sqrtf( static_cast<float>( LV_MAX_INSTANCE_COUNT ) ) );
        float x = static_cast <float>( -count );
        float y = static_cast <float>( -count );
        float z = 0;
        for ( int i = 0; i < count; ++i )
        {
            for ( int j = 0; j < count; ++j )
            {
                int index = i * count + j;
                size_t entityID = entityManager->Get_Entity( index ).index;
                rigidBody->Pos( glm::vec3( x, y, z ), entityID );
                x += 2;
            }

            y += 2;
            x = static_cast <float>( -count );
        }

        for ( size_t i = count * count; i < LV_MAX_INSTANCE_COUNT; ++i )
        {
            x += 2;
            size_t entityID = entityManager->Get_Entity( i ).index;
            rigidBody->Pos( glm::vec3( x, y, z ), entityID );
        }
    }

    return S_OK;
}

#if defined(_WIN32) || defined(_WIN64)
LV_RESULT Engine::Run()
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
            Update();
        }
    }
    return (HRESULT) msg.wParam;
}

void Engine::Quit()
{
    PostMessage( this->hWindow, WM_CLOSE, NULL, NULL );
}
#endif

/* EVENT PROCESSING */

#if defined(_WIN32) || defined(_WIN64)

LRESULT Engine::ProcessMessage( LV_HANDLE hWindow, uint32_t message, WPARAM wParam, LPARAM lParam )
{
    return Engine::engineInstance->HandleEvents( hWindow, message, wParam, lParam );
}

LRESULT Engine::HandleEvents( LV_HANDLE hWindow, uint32_t message, WPARAM wParam, LPARAM lParam )
{
    switch ( message )
    {
        //handle mouse down/up
        case WM_MOUSEMOVE:
            inputManager->OnMouseMove( wParam, GET_X_LPARAM( lParam ), GET_Y_LPARAM( lParam ) );
            return 0;
        case WM_LBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_RBUTTONDOWN:
            inputManager->OnMouseDown( wParam, GET_X_LPARAM( lParam ), GET_Y_LPARAM( lParam ) );
            return 0;
        case WM_LBUTTONUP:
        case WM_MBUTTONUP:
        case WM_RBUTTONUP:
            inputManager->OnMouseUp( wParam, GET_X_LPARAM( lParam ), GET_Y_LPARAM( lParam ) );
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

void Engine::OnResize( uint32_t width, uint32_t height )
{
    windowWidth = static_cast<float>( width );
    windowHeight = static_cast<float>( height );
}

#endif //windows

/* HELPERS */

void Engine::UsingInputFunc()
{
    DEBUG_PRINT( "FPS: %f \n", 1.0f / time->GetDeltaFloatTime() );
}

inline void Engine::Update()
{
    float dtfloat = time->GetDeltaFloatTime();
    const static float speed = 2.f;
    if ( inputManager->IsKeyDown( VK_LEFT ) ) // A
        camera->MoveSideways( dtfloat * speed );
    else if ( inputManager->IsKeyDown( VK_RIGHT ) ) // d
        camera->MoveSideways( -dtfloat * speed );

    if ( inputManager->IsKeyDown( VK_UP ) ) // W
        camera->MoveForward( dtfloat * speed );
    else if ( inputManager->IsKeyDown( VK_DOWN ) ) // S
        camera->MoveForward( -dtfloat * speed );

    //DEBUG collision code 
    float x = sinf( time->GetTotalFloatTime() ) / 100.0f;
    for ( size_t i = 0; i < LV_MAX_INSTANCE_COUNT; ++i )
    {
        componentManager->transform[ i ].pos.x += x;
    }

    //DEBUG CODE for debug wireframe renderer
    glm::mat4x4 transform = glm::translate( glm::mat4( 1.0f ), glm::vec3( 1.f, 1.f, 0.f ) );
    glm::vec3 scale = glm::float3( 1.f, 1.f, 1.f );
    glm::vec3 color = glm::float3( 1.f, 0.f, 0.f );

    debugRenderer->AddCube( transform, scale, color );
    transform = glm::identity<glm::mat4x4>();
    color = glm::float3( 0.f, 0.f, 1.f );
    debugRenderer->AddCube( transform, scale, color );

    //DEBUG CODE for basic camera update


    physics->Update( time->GetTotalFloatTime() );
    graphics->Update( reinterpret_cast<glm::mat4x4_packed *>( componentManager->transformMatrix ), camera );
    graphics->Render();
    time->UpdateTimer();
    debugRenderer->ClearCubes();
}

#if defined(_WIN32) || defined(_WIN64)
void Engine::CreateConsoleWindow( int bufferLines, int bufferColumns, int windowLines, int windowColumns )
{
    //#if defined(_DEBUG)  //we'll uncomment when we have imgui working
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
    //#endif
}
#endif
