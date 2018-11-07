#pragma once

#if defined(_WIN32) || defined(_WIN64)

#include <Windows.h>

#endif // Windows

#include <functional>



namespace Input
{

    enum InputAction
    {
        Fire,
        Use
    };

    /// <summary>
    /// Axis mappings for input. An axis is 
    /// </summary>
    enum InputAxis
    {
        Horizontal,
        Vertical
    };

    typedef std::function<void( float )> axisCallbackFunction_t;

    /// <summary>
    /// Input Manager that will be in charge of handling input events and 
    /// action mappings
    /// </summary>
    /// <author>Ben Hoffman</author>
    class InputManager
    {
    public:
        //BindAxis("MoveForward", this, &ASampleCharacter::MoveForward);
        // Where MoveForward is a function like this :
            // ASampleCharacter::MoveForward(float inputValue)

        template<class UserClass>
        static void BindAxis(
            const char* aAxisName,
            UserClass* aObject,
            void ( UserClass::*func_ptr )( float )
        )
        {
            DEBUG_PRINT( "Add axis name %s to the boi", aAxisName );
            // Wowza
            std::invoke( func_ptr, aObject, 1.f );
        }

        void OnMouseDown( WPARAM buttonState, int x, int y );
        void OnMouseUp( WPARAM buttonState, int x, int y );
        void OnMouseMove( WPARAM buttonState, int x, int y );

    private:




    };  // class InputManager

}   // namespace Input