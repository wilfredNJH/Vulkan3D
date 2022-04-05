/******************************************************************************/
/*!
\file   WindowManager.h
\author Wilfred Ng Jun Hwee
\par    DP email: junhweewilfred.ng[at]digipen.edu
\par    course: csd2150
\par    Homework 2
\date   03/07/2022
\brief
    This is the declaration file of the window manager
*/
/******************************************************************************/
#pragma once
#include <Windows.h>
#include <thread>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>

#pragma comment(lib,"user32.lib") 

struct VkWindow
{
    HINSTANCE           wInstance;
    HWND                wHandle;

    VkWindow(int width,int height);
    
    int m_width{ 500 };
    int m_height{ 500 };

    bool mCanRender;
    bool mSizeChanged = false;
    bool mCloseWindow = false;

    bool wasWindowResize() { return mSizeChanged;  }
    void resetWindowResizedFlag() { mSizeChanged = false; }
    bool closeWindow() { return mCloseWindow; }//check if to close window 
    bool isMinimised() {return IsIconic(wHandle);}//checks if it's minimise  
    bool Update();//update window
    void showWindow();//showing the window


    VkExtent2D getExtent() { return { static_cast<uint32_t>(m_width),static_cast<uint32_t>(m_height) }; }


    void CreateWindowSurface(VkInstance instance, VkSurfaceKHR* surface);//creating the vulkan surface 
};