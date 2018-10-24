#pragma once



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

        //BindAxis("MoveForward", this, &ASampleCharacter::MoveForward);
        // Where MoveForward is a function like this :
            // ASampleCharacter::MoveForward(float inputValue)

        template<class UserClass>
        void BindAxis(
            const char* aAxisName,
            UserClass* aObject,
            axisCallbackFunction_t aCallbalFunc
        )
        {
            DEBUG_PRINT( "Add axis name %s to the boi", axisName );
        }

        //void AddInputEvent( InputAction aAction );

        //void RemoveInputEvent(InputAction aAction);

        //float AxisValue( InputAxis aAxis );


    };  // class InputManager

}   // namespace Input