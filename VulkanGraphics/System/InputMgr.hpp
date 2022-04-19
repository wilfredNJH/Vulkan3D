/******************************************************************************/
/*!
\file   InputMgr.hpp
\author Wilfred Ng Jun Hwee
\par    DP email: junhweewilfred.ng[at]digipen.edu
\par    course: csd2150
\par    Final Mesh Viewer
\date   04/15/2022
\brief
	This is the declaration file of the input manager
*/
/******************************************************************************/
#ifndef __INPUTMGR_H__
#define __INPUTMGR_H__

#include <windows.h>
#include <glm/glm.hpp>

class MouseInput;
class KeyInput;
class InputMgr
{
	unsigned char data[256]; //current virtual key state
	unsigned char prevData[256]; // previous virtual key state

	 // mouse scrolling, going to negative means scroll down, going to positive means scroll up
	int zDelta = 0; //raw value from windows
	int mouse_changes = 0; //rate of change per WHEEL_DELTA
	bool last_mouse_scroll = 0; //temporary fix to not resetting to 0 after scroll

	void SaveKeyboardState();
	InputMgr();
	//Use this three fn to check for key state
	/*
	*	NOTE: USE MouseManager && KeyManager respectively instead
	*
	*
	*	Sample usage;
	*	isKeyTriggered(VK_ESCAPE);
	*	isKeyTriggered('A');                 -> Use capital letter if possible
	*	isKeyTriggered('a', true);
	*
	*	For list of inputs: https://docs.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
	*	Or find "WinUser.h" or simply peak on VK_ESCAPE.
	*/
	bool isKeyTriggered(char key, bool charKey = false, bool disableUntilUpdated = false);
	bool isKeyPressed(char key, bool charKey = false, bool disableUntilUpdated = false);
	bool isKeyReleased(char key, bool charKey = false, bool disableUntilUpdated = false);
public:
	~InputMgr();
	void update();

	static InputMgr& GetInstance();

	//Handle other message from window not handled in WinMgr WinProcedure, which include inputs
	LRESULT CALLBACK Inputs(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	//For getting toggle stat ->might delete away
	void CheckKeyState(char key, bool& PressedState, bool& ToggleState);

	//for access to isKeyTriggered, pressed and released
	friend class MouseInput;
	friend class KeyInput;
};

enum class MouseButton
{
	LEFT_BUTTON = VK_LBUTTON,
	RIGHT_BUTTON = VK_RBUTTON,
	MIDDLE_BUTTON = VK_MBUTTON,
	X_BUTTON_1 = VK_XBUTTON1,
	X_BUTTON_2 = VK_XBUTTON2,
	NONE
};

class MouseInput
{
	glm::vec2 mouse_position_TL;
	glm::vec2 mouse_position_BL;
	glm::vec2 mouse_position_relative;



	MouseInput() = default;

public:
	~MouseInput() = default;
	glm::vec2 getMousePosition_WindowSpace() const;//Client's window space (Origin: TOP LEFT)
	glm::vec2 getMousePosition_BL() const;//Normal space Origin: BOTTOM LEFT
	glm::vec2 getMousePosition_Relative() const;

	void updateMousePosition_windowSpace(float x, float y);
	void updateMousePosition_BL(float x, float y);
	void updateMousePosition_Relative(float x, float y);

	bool isButtonTriggered(MouseButton button, bool disableUntilUpdated = false);
	bool isButtonPressed(MouseButton button, bool disableUntilUpdated = false);
	bool isButtonReleased(MouseButton button, bool disableUntilUpdated = false);

	//mouse scroll 
	int getMouseScroll(); // 0 means no change, greater 0 means up, smaller 0 means down

	static MouseInput& GetInstance();
};

class KeyInput
{
	KeyInput() = default;
public:
	~KeyInput() = default;

	//Use this three fn to check for key state
	/*
	*	Sample usage;
	*	isKeyTriggered(VK_ESCAPE);
	*	isKeyTriggered('A');                 -> Use capital letter if possible
	*	isKeyTriggered('a', true);
	*
	*	For list of inputs: https://docs.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
	*	Or find "WinUser.h" or simply peak on VK_ESCAPE.
	*/
	bool isKeyTriggered(char key, bool charKey = false, bool disableUntilUpdated = false);
	bool isKeyPressed(char key, bool charKey = false, bool disableUntilUpdated = false);
	bool isKeyReleased(char key, bool charKey = false, bool disableUntilUpdated = false);

	static KeyInput& GetInstance();
};

#define InputManager ::InputMgr::GetInstance()
#define MouseManager ::MouseInput::GetInstance()
#define KeyManager ::KeyInput::GetInstance()

#endif
