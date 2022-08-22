/*!*****************************************************************************
 * @file	imguiHelper.cpp
 * @author	Owen Huang Wensong  [390008220]  (w.huang@digipen.edu)
 * @date	02 OCT 2021
 * @brief	This file contains the implementations of helper functions
 *
 * Copyright (C) 2021 DigiPen Institute of Technology. All rights reserved.
*******************************************************************************/

#include "../include/imguiHelper.h"
#include <iostream>
void imguiHelper::initIMGUI(ImGui_ImplVulkan_InitInfo InitInfo, VkRenderPass RenderPass, void* inHWND)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io{ ImGui::GetIO() };
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	ImGui::StyleColorsDark();
	ImGui_ImplVulkan_Init(&InitInfo, RenderPass);
	ImGui_ImplWin32_Init(inHWND);
}

void imguiHelper::createFont(VkCommandBuffer SingleUseCommandBuffer)
{
	ImGui_ImplVulkan_CreateFontsTexture(SingleUseCommandBuffer);
}

void imguiHelper::Update()
{
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	//const ImGuiViewport* viewport{ ImGui::GetMainViewport() };
	//ImGui::DockSpaceOverViewport(viewport, ImGuiDockNodeFlags_PassthruCentralNode);
}

void imguiHelper::Draw(VkCommandBuffer FCB)
{
	//GLFWwindow* winptr{ OKP::WindowHandler::getInstance().getWindowPtr() };
	ImGuiIO& io{ ImGui::GetIO() };
	//int w, h;
	//glfwGetFramebufferSize(winptr, &w, &h);
	//io.DisplaySize = ImVec2{ static_cast<float>(w), static_cast<float>(h) };
	if (const ImGuiViewport* pViewPort{ ImGui::GetMainViewport() }; pViewPort != nullptr)
	{
		io.DisplaySize = pViewPort->Size;
	}

	//glViewport(0, 0, w, h);
	ImGui::Render();
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), FCB);
	//ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	//if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	//{
	//	ImGui::UpdatePlatformWindows();
	//	ImGui::RenderPlatformWindowsDefault();
	//	glfwMakeContextCurrent(winptr);
	//}
}

void imguiHelper::ShutDown()
{
	ImGui_ImplWin32_Shutdown();
	ImGui_ImplVulkan_Shutdown();
	ImGui::DestroyContext();
}
