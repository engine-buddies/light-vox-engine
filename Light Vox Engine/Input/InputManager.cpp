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
    auto itr = listeners.begin();
    for ( ; itr != listeners.end(); ++itr )
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
    auto const map_itr = listeners.find( type );
    if ( map_itr == listeners.end() ) { return; }
    
    std::vector<IListener*>::iterator vec_itr = map_itr->second.begin();

    // Why doesnt this heckin work
    /*for ( ; vec_itr != map_itr->second.end(); ++vec_itr )
    {
        *( vec_itr )->Invoke();        
    }*/

    // Call the functors that are listening
    for ( size_t i = 0; i < map_itr->second.size(); ++i )
        (*map_itr->second[ i ])();
    

    //auto itr_l = map_itr->second->cbegin();
    
}

void Input::InputManager::BindAxis( InputType type, input_action_func inputListenerFunc )
{
    IListener* newListener = new ListenerFunc( inputListenerFunc );

    listeners[ type ].push_back( newListener );
}

void InputManager::OnMouseDown( WPARAM buttonState, int x, int y )
{
    // Test out the signalling of the system
    SignalInput( InputType::Use );
}

void InputManager::OnMouseUp( WPARAM buttonState, int x, int y )
{

}

void InputManager::OnMouseMove( WPARAM buttonState, int x, int y )
{

}

bool Input::InputManager::IsActionDown( int vKey )
{


    return GetAsyncKeyState( vKey ) & 0x80000;
}
