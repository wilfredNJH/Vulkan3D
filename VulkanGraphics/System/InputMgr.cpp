/******************************************************************************/
/*!
\file   InputMgr.cpp
\author Wilfred Ng Jun Hwee
\par    DP email: junhweewilfred.ng[at]digipen.edu
\par    course: csd2150
\par    Homework 2
\date   03/07/2022
\brief
	This is the implementation file of the input manager
*/
/******************************************************************************/
#include "InputMgr.hpp"

//bit 31 of lParam in window message determine if a key is pressed repeatedly
#define REPEATED_BIT 0b01000000000000000000000000000000
//#define REPEATED_BIT 0x40000000

//#define HI_BIT_GETKEYSTATE 0b10000000
#define HI_BIT_GETKEYSTATE 0x80
#define LOW_BIT_GETKEYSTATE 0b00000001

//In a Win32 context lParam is 32 bits.  
//"Word" is 16 bits, so two words can fit into a 32 bit location at the same time.
//The "low" word is the least significant 16 bits.The "high" word is the most significant 16 bits.
#define LOW_WORD_PARAM 0xffff
//#define HIGH_WORD_PARAM 0b11111111'11111111'00000000'00000000

////////////////////////////////////////////


//Uncomment cout every key pressed
#define INPUT_DEBUG

#ifdef INPUT_DEBUG
#include <iostream>
using std::cout;
using std::endl;
#endif

//static InputMgr* instance;
void InputMgr::SaveKeyboardState()
{
	//prevData = data;
	memcpy(prevData, data, sizeof(data));
	if (GetKeyboardState(data) == false)
	{
		std::cout << "GetKeyboardState failed" << std::endl;//error, function failed
	}
}

//temp
InputMgr::InputMgr()
{
	SaveKeyboardState();
}

InputMgr::~InputMgr()
{
}

void InputMgr::update()
{
	SaveKeyboardState();
	//This is a temp fix, it fixes, but havent thought of a better way atm, TODO
	if (last_mouse_scroll)
		last_mouse_scroll = false;
	else
	{
		zDelta = 0;
		mouse_changes = 0;
	}
}

InputMgr& InputMgr::GetInstance()
{
	static InputMgr instance;
	return instance;
}

//might remove if not needed in the future
//You typically do not need the information in lParam. One flag that might be useful is bit 30, the "previous key state" flag, which is set to 1 for repeated key-down messages.
LRESULT InputMgr::Inputs(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_MOUSEMOVE:
	{
		//IF THIS IS CAUSING ISSUE, JUST INCLUDE WINMGR TO GET THE HWND, try not to use foreground window coz think its refering to any active(even if not this window)
		RECT client_dimension{};
		GetClientRect(GetActiveWindow(), &client_dimension);

		int x = lParam & LOW_WORD_PARAM;
		int y = ((lParam >> 16) & LOW_WORD_PARAM);

		if (x < 0)
			x = 0;
		if (x > client_dimension.right)
			x = client_dimension.right;

		if (y < 0)
			y = 0;
		if (y > client_dimension.bottom)
			y = client_dimension.bottom;

		MouseManager.updateMousePosition_windowSpace(static_cast<float>(x), static_cast<float>(y));

		//change to normal 2d axis space
		y = client_dimension.bottom - y;
		MouseManager.updateMousePosition_BL(static_cast<float>(x), static_cast<float>(y));
	}
	break;
	case WM_MOUSEWHEEL:
	{
		mouse_changes = GET_WHEEL_DELTA_WPARAM(wParam);
		zDelta += mouse_changes;
		mouse_changes /= WHEEL_DELTA;

		last_mouse_scroll = true;
	}
	break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

void InputMgr::CheckKeyState(char key, bool& PressedState, bool& ToggleState)
{
	//https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getkeystate
	//short result = GetKeyState(key);
	short result = data[key];
	if ((result & HI_BIT_GETKEYSTATE) == HI_BIT_GETKEYSTATE)
		//Key is pressed
		PressedState = true;
	else
		PressedState = false;

	if ((result & LOW_BIT_GETKEYSTATE) == LOW_BIT_GETKEYSTATE)
		//toggle is on
		ToggleState = true;
	else
		ToggleState = false;
}

bool InputMgr::isKeyTriggered(char key, bool charKey, bool disableUntilUpdated)
{
	if (charKey)
	{
		if (key >= 'a' && key < 'z')
			key -= 32;
	}
	//current on, and prev off
	bool result = (isKeyPressed(key) && (prevData[key] & HI_BIT_GETKEYSTATE) != HI_BIT_GETKEYSTATE);
	if (result && disableUntilUpdated)
		prevData[key] = false;
	return result;
}

bool InputMgr::isKeyPressed(char key, bool charKey, bool disableUntilUpdated)
{
	if (charKey)
	{
		if (key >= 'a' && key < 'z')
			key -= 32;
	}
	//current on
	bool result = (data[key] & HI_BIT_GETKEYSTATE) == HI_BIT_GETKEYSTATE;
	if (result && disableUntilUpdated)
		data[key] = false;
	return result;
}

bool InputMgr::isKeyReleased(char key, bool charKey, bool disableUntilUpdated)
{
	if (charKey)
	{
		if (key >= 'a' && key < 'z') {
			key -= 32;

		}


	}
	//current off, and prev on
	bool result = (isKeyPressed(key) == false) && (prevData[key] & HI_BIT_GETKEYSTATE) == HI_BIT_GETKEYSTATE;
	if (result && disableUntilUpdated)
		prevData[key] = false;
	return result;
}

glm::vec2 MouseInput::getMousePosition_WindowSpace() const
{

	return mouse_position_TL;
}

glm::vec2 MouseInput::getMousePosition_BL() const
{
	return mouse_position_BL;
}


void MouseInput::updateMousePosition_windowSpace(float x, float y)
{
	mouse_position_TL = glm::vec2{ x, y };
}

void MouseInput::updateMousePosition_BL(float x, float y)
{
	mouse_position_BL = glm::vec2{ x, y };
}

bool MouseInput::isButtonTriggered(MouseButton button, bool disableUntilUpdated)
{
	return InputManager.isKeyTriggered(static_cast<char>(button), false, disableUntilUpdated);
}

bool MouseInput::isButtonPressed(MouseButton button, bool disableUntilUpdated)
{
	return InputManager.isKeyPressed(static_cast<char>(button), false, disableUntilUpdated);
}

bool MouseInput::isButtonReleased(MouseButton button, bool disableUntilUpdated)
{
	return InputManager.isKeyReleased(static_cast<char>(button), false, disableUntilUpdated);
}

int MouseInput::getMouseScroll()
{
	return InputManager.mouse_changes;
}

MouseInput& MouseInput::GetInstance()
{
	static MouseInput instance;
	return instance;
}

bool KeyInput::isKeyTriggered(char key, bool charKey, bool disableUntilUpdated)
{
	return InputManager.isKeyTriggered(key, charKey, disableUntilUpdated);
}

bool KeyInput::isKeyPressed(char key, bool charKey, bool disableUntilUpdated)
{
	return InputManager.isKeyPressed(key, charKey, disableUntilUpdated);
}

bool KeyInput::isKeyReleased(char key, bool charKey, bool disableUntilUpdated)
{
	return InputManager.isKeyReleased(key, charKey, disableUntilUpdated);
}

KeyInput& KeyInput::GetInstance()
{
	static KeyInput instance;
	return instance;
}
