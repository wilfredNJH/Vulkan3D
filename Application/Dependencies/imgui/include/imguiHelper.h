/*!*****************************************************************************
 * @file	imguiHelper.h
 * @author	Owen Huang Wensong  [390008220]  (w.huang@digipen.edu)
 * @date	02 OCT 2021
 * @brief	This file makes it simple for others to include imgui.
 *
 * Copyright (C) 2021 DigiPen Institute of Technology. All rights reserved.
*******************************************************************************/

#ifndef OUR_IMGUI_HELPER_HEADER
#define OUR_IMGUI_HELPER_HEADER

#include "imgui.h"
//#include "imgui_stdlib.h" // for inputText w/ strings
//#include "imgui_impl_glfw.h"
//#include "../include/imgui_impl_opengl3.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_vulkan.h"

namespace imguiHelper
{
  void initIMGUI(ImGui_ImplVulkan_InitInfo InitInfo, VkRenderPass RenderPass, void* inHWND);
  void createFont(VkCommandBuffer SingleUseCommandBuffer);
  void Update();
  void Draw(VkCommandBuffer FCB);
  void ShutDown();
}

#define IMGUI_WIN32_WNDPROCHANDLER_FORWARD_DECLARATION extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)

#endif//OUR_IMGUI_HELPER_HEADER

