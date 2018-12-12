#pragma once

#include "../stdafx.h"
#include <unordered_map>
#include <vector>

#if defined(_WIN32) || defined(_WIN64)

#include <Windows.h>

#endif // Windows


namespace Input
{

	struct Point
	{
		int x;
		int y;
	};

	enum InputType
	{
		Horizontal,
		Vertical,
		Look,
		Fire,
		StartLook,
		StopLook
	};

	/// <summary>
	/// Input Manager that will be in charge of handling input events and 
	/// action mappings
	/// </summary>
	/// <author>Ben Hoffman</author>
	class InputManager
	{

		typedef void(*input_action_func)();

	public:

		/// <summary>
		/// Gets a staticinstance of the input manager, if it is null 
		/// then it will be created. 
		/// </summary>
		/// <returns>Pointer to the current input manager instance</returns>
		static InputManager* GetInstance();

		/// <summary>
		/// Destroy the input manager instance
		/// </summary>
		static void ReleaseInstance();

		void BindAxis(InputType type, input_action_func inputListenerFunc);

		template<class T>
		void BindAxis(InputType type, T* parentObj, void (T::*inputListenerFunc)())
		{
			IListener* newListener = new ListenerMember<T>(parentObj, inputListenerFunc);

			actionListeners[type].push_back(newListener);
		}

		bool IsKeyDown(int vKey);

		// Windows specific input callbacks
#if defined(_WIN32) || defined(_WIN64)

		void OnMouseDown(WPARAM buttonState, int x, int y);
		void OnMouseUp(WPARAM buttonState, int x, int y);
		void OnMouseMove(WPARAM buttonState, int x, int y);

		const Point GetCurrentMousePos() { return CurMousePos; }
		const Point GetPrevMousePos() { return PrevMousePos; }
#endif

	private:

		InputManager();

		~InputManager();

		/** The instance of the input manager */
		static InputManager* instance;

		void SignalInput(InputType type);

		Point CurMousePos;

		Point PrevMousePos;

		///////////////////////////////////////////////////////
		// Listener definitions 

		struct IListener
		{
			virtual ~IListener() {}
			virtual void operator () () = 0;
		};

		struct ListenerFunc : IListener
		{
			ListenerFunc(input_action_func aFunc_ptr)
				: func_ptr(aFunc_ptr)
			{
			}

			virtual void operator () () override
			{
				return (func_ptr());
			}

			/** The function pointer for this input action to invoke */
			input_action_func func_ptr;
		};

		template <class T>
		struct ListenerMember : IListener
		{
			ListenerMember(T* aParent, void (T::*f)())
				: parentObj(aParent), func_ptr(f)
			{
			}

			virtual void operator () () override
			{
				assert(parentObj != nullptr);

				return ((parentObj->*func_ptr)());
			}

			/** the object to invoke the function pointer on */
			T* parentObj;

			/** The function pointer to call when we invoke this function */
			void (T::*func_ptr)();
		};

		/** A map of active listeners */
		std::unordered_map<InputType, std::vector<IListener*>> actionListeners;

	};  // class InputManager

}   // namespace Input