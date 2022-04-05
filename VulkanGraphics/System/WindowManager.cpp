/******************************************************************************/
/*!
\file   WindowManager.cpp
\author Wilfred Ng Jun Hwee
\par    DP email: junhweewilfred.ng[at]digipen.edu
\par    course: csd2150
\par    Homework 2
\date   03/07/2022
\brief
    This is the implementation file of the window manager
*/
/******************************************************************************/
#include "WindowManager.h"
#include "InputMgr.hpp"
#include <iostream>

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_SIZE:
    case WM_EXITSIZEMOVE:
        PostMessage(hWnd, WM_USER + 1, wParam, lParam);
        break;
    case WM_CLOSE:
        PostMessage(hWnd, WM_USER + 2, wParam, lParam);
        break;
    case WM_KEYDOWN:
        switch (wParam)
        {
        case VK_ESCAPE:
            PostMessage(hWnd, WM_USER + 2, wParam, lParam);//exit 
            break;
        }
        break;
    default:
        return InputManager.Inputs(hWnd, message, wParam, lParam);
    }
    return 0;
}


VkWindow::VkWindow(int width,int height) : mSizeChanged(false), mCanRender(true) , mCloseWindow(false){
    wInstance = GetModuleHandle(nullptr);

    // Register window class
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = wInstance;
    wcex.hIcon = NULL;
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = TEXT("CSD2150");
    wcex.hIconSm = NULL;

    //assigning the width and height 
    m_width = width;
    m_height = height;

    if (!RegisterClassEx(&wcex))
    {
        std::cout << "failed to creat window" << std::endl;
    }
    // Create window
    wHandle = CreateWindow(wcex.lpszClassName, TEXT("Homework Cube"), WS_OVERLAPPEDWINDOW, 20, 20, width, height, nullptr, nullptr, wInstance, nullptr);
    if (!wHandle)
    {
        std::cout << "failed to creat window" << std::endl;
    }
}


void VkWindow::CreateWindowSurface(VkInstance instance, VkSurfaceKHR* surface) {
    VkWin32SurfaceCreateInfoKHR surfaceCreateInfo{};
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surfaceCreateInfo.hinstance = wInstance;
    surfaceCreateInfo.hwnd = wHandle;

    if (vkCreateWin32SurfaceKHR(instance, &surfaceCreateInfo, nullptr, surface) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create win32 surface!");
    }
}

bool VkWindow::Update() {
    MSG message;// Main message loop

    if (PeekMessage(&message, NULL, 0, 0, PM_REMOVE)) {
        // Process events
        switch (message.message) {
        case WM_USER + 1:
            break;
        case WM_USER + 2:
            mCloseWindow = true;// Close
            break;
        }
        TranslateMessage(&message);
        DispatchMessage(&message);
        return true;
    }
    else {
        return false;
    }
}

void VkWindow::showWindow() {
    //init showing the window 
    ShowWindow(wHandle, 1);
    UpdateWindow(wHandle);
}