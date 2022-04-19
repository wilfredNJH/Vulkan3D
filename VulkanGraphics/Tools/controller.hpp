/******************************************************************************/
/*!
\file   controller.hpp
\author Wilfred Ng Jun Hwee
\par    DP email: junhweewilfred.ng[at]digipen.edu
\par    course: csd2150
\par    Final Mesh Viewer
\date   04/15/2022
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
        void moveThirdPerson(float dt, NkGameObject::Map& gameObject, NkGameObject& camera);//moving in third person perspective 

        bool firstPerson = false;//tracks first person or third person perspective 

        bool modelNumber = true;//toggle between the first model and the second model 

        //controls speed 
        float moveSpeed{ 3.f };
        float lookSpeed{ 3.f };
    };
}  