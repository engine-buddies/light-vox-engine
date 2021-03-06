#include "InputManager.h"

using namespace Input;

InputManager* InputManager::instance = nullptr;

InputManager * Input::InputManager::GetInstance()
{
    if ( instance == nullptr )
    {
        instance = new InputManager();
    }
    return instance;
}

void Input::InputManager::ReleaseInstance()
{
    if ( instance != nullptr )
    {
        delete instance;
        instance = nullptr;
    }
}

// Private constructor
InputManager::InputManager()
{
    // Register and OS callbacks here

}

// Private d'tor
InputManager::~InputManager()
{
    // Destroy any input listeners
    auto itr = actionListeners.begin();
    for ( ; itr != actionListeners.end(); ++itr )
    {
        auto in_itr = itr->second.begin();
        for ( ; in_itr != itr->second.end(); ++in_itr )
        {
            if ( *( in_itr ) != nullptr )
            {
                delete *( in_itr );
            }
        }
    }
    DEBUG_PRINT( "Input Listeners destroyed!" );
}

void Input::InputManager::SignalInput( InputType type )
{
    auto const map_itr = actionListeners.find( type );
    if ( map_itr == actionListeners.end() ) { return; }

    std::vector<IListener*>::iterator vec_itr = map_itr->second.begin();

    for ( ; vec_itr != map_itr->second.end(); ++vec_itr )    
        ( *( *vec_itr ) ) ( );
}

void Input::InputManager::BindAxis( InputType type, input_action_func inputListenerFunc )
{
    IListener* newListener = new ListenerFunc( inputListenerFunc );

    actionListeners[ type ].push_back( newListener );
}

#if defined(_WIN32) || defined(_WIN64)


void InputManager::OnMouseDown( WPARAM buttonState, int x, int y )
{
    // Test out the signaling of the system
    SignalInput( InputType::Fire );
    SignalInput( InputType::StartLook );
}

void InputManager::OnMouseUp( WPARAM buttonState, int x, int y )
{
    SignalInput( InputType::StopLook );
}

void InputManager::OnMouseMove( WPARAM buttonState, int x, int y )
{
    PrevMousePos = CurMousePos;

    CurMousePos.x = x;
    CurMousePos.y = y;

    SignalInput( InputType::Look );
}

#endif

bool Input::InputManager::IsKeyDown( int vKey )
{
    return GetAsyncKeyState( vKey ) & 0x80000;
}
