/******************************************************************************/
/*!
\file   controller.hpp
\author Wilfred Ng Jun Hwee
\par    DP email: junhweewilfred.ng[at]digipen.edu
\par    course: csd2150
\par    Homework 2
\date   03/07/2022
\brief
    This is the declaration file of the camera
*/
/******************************************************************************/
#pragma once

#include "WindowManager.h"
#include "vk_gameobject.hpp"
#include "InputMgr.hpp"


namespace nekographics {
    class KeyboardMovementController {
    public:

        void moveInPlaneXZ(VkWindow* window, float dt, NkGameObject& gameObject);//moving the plane xz 
        void moveThirdPerson(float dt, NkGameObject& gameObject, NkGameObject& camera);//moving in third person perspective 

        bool firstPerson = false;//tracks first person or third person perspective 

        //controls speed 
        float moveSpeed{ 3.f };
        float lookSpeed{ 3.f };
    };
}  