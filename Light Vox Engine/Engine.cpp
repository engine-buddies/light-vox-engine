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
    Graphics::LightingManager::ReleaseInstance();
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
    lightingManager = Graphics::LightingManager::GetInstance();

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
        uint32_t entityID = entityManager->Get_Entity( i ).index;

        //calc. moment of inertia 
        float& mass = componentManager->bodyProperties[ entityID ].mass;
        float inertia = ( ( mass * 0.4f ) * 0.4f ) / 6.0f;
        glm::mat3& inertiaTensor = componentManager->bodyProperties[ entityID ].inertiaTensor;
        inertiaTensor[ 0 ][ 0 ] = inertia;
        inertiaTensor[ 1 ][ 1 ] = inertia;
        inertiaTensor[ 2 ][ 2 ] = inertia;
        componentManager->boxCollider[ entityID ].tag = entityID;
    }

    //DEBUG:: INTIALIZE ENTITY POSSITIONS
    {
        FastNoiseSIMD* noiseLib = FastNoiseSIMD::NewFastNoiseSIMD();
        noiseLib->SetFrequency( 0.17f );
        noiseLib->SetNoiseType( FastNoiseSIMD::NoiseType::Simplex );

        float noiseThreshold = 0.18f;
        float increment = 2.0f;
        int size = LV_MAX_WORLD_SIZE;
        float* noiseSet = noiseLib->GetPerlinSet( 0, 0, 0, size, size, size );

        int start = LV_MAX_WORLD_SIZE * static_cast<int>( increment ) / -2;
        glm::vec3 position = glm::vec3( static_cast<float>( start ) );
        int noiseIndex = 0;
        int entityIndex = 0;

        for ( int x = 0; x < size && entityIndex < LV_MAX_INSTANCE_COUNT; ++x )
        {
            position.y = static_cast<float>( start );
            for ( int y = 0; y < size && entityIndex < LV_MAX_INSTANCE_COUNT; ++y )
            {
                position.z = static_cast<float>( start );
                for ( int z = 0; z < size && entityIndex < LV_MAX_INSTANCE_COUNT; ++z )
                {
                    float val = noiseSet[ noiseIndex ];
                    ++noiseIndex;

                    if ( glm::abs( val ) > noiseThreshold )
                    {
                        uint32_t entityID = entityManager->Get_Entity( entityIndex ).index;
                        rigidBody->Pos( position, entityID );
                        rigidBody->Velocity( glm::linearRand( glm::vec3( -0.2f ), glm::vec3( 0.2f ) ), entityID );
                        rigidBody->RotateAxisAngle( glm::sphericalRand( 1.0f ), glm::linearRand( 0.f, glm::two_pi<float>() ), entityID );

                        ++entityIndex;
                    }

                    position.z += increment;
                }

                position.y += increment;
            }

            position.x += increment;
        }

        if ( entityIndex < LV_MAX_INSTANCE_COUNT )
        {
            DEBUG_PRINT( "%d entities did not initialize -- increase world size or adjust noise settings", LV_MAX_INSTANCE_COUNT - entityIndex );
        }
        else
        {
            DEBUG_PRINT( "%d noise indices left", ( size * size * size ) - noiseIndex );

        }

        FastNoiseSIMD::FreeNoiseSet( noiseSet );
        delete noiseLib;
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
    printf( "FPS: %f \n", 1.0f / time->GetDeltaFloatTime() );
}

inline void Engine::Update()
{
    if ( inputManager->IsKeyDown( VK_ESCAPE ) ) // A
        Quit();

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
    //for ( size_t i = 0; i < LV_MAX_INSTANCE_COUNT; ++i )
    //{
    //    //componentManager->transform[i].pos.x += x;
    //    if ( componentManager->transform[ i ].pos.x > 5.0f )
    //        componentManager->bodyProperties[ i ].velocity.x = -10.0f;

    //    else if ( componentManager->transform[ i ].pos.x < -5.0f )
    //        componentManager->bodyProperties[ i ].velocity.x = 10.0f;
    //}

    //DEBUG CODE for debug wireframe renderer
    glm::mat4x4 transform = glm::translate( glm::mat4( 1.0f ), glm::vec3( 1.f, 1.f, 0.f ) );
    glm::vec3 scale = glm::float3( 1.f, 1.f, 1.f );
    glm::vec3 color = glm::float3( 1.f, 0.f, 0.f );

    debugRenderer->AddCube( transform, scale, color );
    transform = glm::identity<glm::mat4x4>();
    color = glm::float3( 0.f, 0.f, 1.f );
    debugRenderer->AddCube( transform, scale, color );

    //DEBUG CODE for basic camera update
    physics->Update( time->GetDeltaFloatTime() );
    graphics->Update( reinterpret_cast<glm::mat4x4_packed **>( &componentManager->transformMatrix ), camera );
    lightingManager->Update( time->GetDeltaFloatTime() );
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
