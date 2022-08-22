#ifndef IMGUI_HELPER
#define IMGUI_HELPER

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
#endif//OUR_IMGUI_HELPER_HEADER