/******************************************************************************/
/*!
\file   controller.cpp
\author Wilfred Ng Jun Hwee
\par    DP email: junhweewilfred.ng[at]digipen.edu
\par    course: csd2150
\par    Homework 2
\date   03/07/2022
\brief
    This is the implementation file of the camera
*/
/******************************************************************************/
#include "controller.hpp"

// std
#include <limits>
#include <iostream>

namespace nekographics {

    void KeyboardMovementController::moveInPlaneXZ(
        VkWindow* window, float dt, NkGameObject& gameObject) {

        //check if wants to switch
        if (KeyManager.isKeyTriggered('M')) {
            firstPerson = false;//set back to first person 
            return;
        }

        UNREFERENCED_PARAMETER(window);

        glm::vec3 rotate{ 0 };

        //rotate
        if (KeyManager.isKeyPressed(VK_RIGHT)) {
            rotate.y += 1.f;
        }if (KeyManager.isKeyPressed(VK_LEFT)) {
            rotate.y -= 1.f;
        }if (KeyManager.isKeyPressed(VK_UP)) {
            rotate.x += 1.f;
        }if (KeyManager.isKeyPressed(VK_DOWN)) {
            rotate.x -= 1.f;
        }

        //calculating the rotation 
        if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon()) {
            gameObject.transform.rotation += lookSpeed * dt * glm::normalize(rotate);
        }

        // limit pitch values between about +/- 85ish degrees
        gameObject.transform.rotation.x = glm::clamp(gameObject.transform.rotation.x, -1.5f, 1.5f);
        gameObject.transform.rotation.y = glm::mod(gameObject.transform.rotation.y, glm::two_pi<float>());

        float yaw = gameObject.transform.rotation.y;
        const glm::vec3 forwardDir{ sin(yaw), 0.f, cos(yaw) };
        const glm::vec3 rightDir{ forwardDir.z, 0.f, -forwardDir.x };
        const glm::vec3 upDir{ 0.f, -1.f, 0.f };
        glm::vec3 moveDir{ 0.f };

        moveSpeed = 3.f;//default move speed 

        //zoom in & out 
        if (KeyManager.isKeyPressed('W')) moveDir += forwardDir;
        if (MouseManager.getMouseScroll() > 0) moveDir += forwardDir, moveSpeed = 500.f;
        if (KeyManager.isKeyPressed('S')) moveDir -= forwardDir;
        if (MouseManager.getMouseScroll() < 0) moveDir -= forwardDir, moveSpeed = 500.f;

        //looking at
        if (KeyManager.isKeyPressed('d', true)) moveDir += rightDir;
        if (KeyManager.isKeyPressed('a', true)) moveDir -= rightDir;
        if (KeyManager.isKeyPressed('e', true)) moveDir += upDir;
        if (KeyManager.isKeyPressed('q', true)) moveDir -= upDir;

        //calculating the translation
        if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon()) {
            gameObject.transform.translation += moveSpeed * dt * glm::normalize(moveDir);
        }
    }

    float angleBetween(
        glm::vec3 a,
        glm::vec3 b,
        glm::vec3 origin
    ) {
        glm::vec3 da = glm::normalize(a - origin);
        glm::vec3 db = glm::normalize(b - origin);
        return glm::acos(glm::dot(da, db));
    }

    void KeyboardMovementController::moveThirdPerson(float dt, NkGameObject& gameObject, NkGameObject& camera) { 

        //check if wants to switch
        if (KeyManager.isKeyTriggered('M')) {
            firstPerson = true;//set back to first person 
            return;
        }


        glm::vec3 resultingVector = gameObject.transform.translation - camera.transform.translation;//vector to game object


        const glm::vec3 forwardDir{ resultingVector };
        glm::vec3 moveDir{ 0.f };

        moveSpeed = 3.f;//default move speed 

        //zoom in & out 
        if (MouseManager.getMouseScroll() > 0) {
            const float clampDistance = 0.3f;
            //clamp if too near to the box 
            if (glm::distance(gameObject.transform.translation, camera.transform.translation) <= 1.f) {
                moveDir = { 0.f,0.f,0.f };
            }
            else {
                moveDir += forwardDir, moveSpeed = 500.f;
            }
        }
        if (MouseManager.getMouseScroll() < 0) {
            moveDir -= forwardDir, moveSpeed = 500.f;
        }

        //rotation
        float degree = 0.2f;
        if (KeyManager.isKeyPressed('D')) {
            glm::vec3 rotationAxis = { 0.f,-1.f,0.f };
            glm::mat4 rot_mat = glm::rotate(glm::mat4(1.f), glm::radians(degree), rotationAxis);
            camera.transform.translation = glm::vec3(glm::vec4(camera.transform.translation, 1.0f) * rot_mat);
        }
        float degreeOpp = -0.2f;
        if (KeyManager.isKeyPressed('A')) {
            glm::vec3 rotationAxis = { 0.f,-1.f,0.f };
            glm::mat4 rot_mat = glm::rotate(glm::mat4(1.f), glm::radians(degreeOpp), rotationAxis);
            camera.transform.translation = glm::vec3(glm::vec4(camera.transform.translation, 1.0f) * rot_mat);
        }

        //calculating the translation
        if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon()) {
            camera.transform.translation += moveSpeed * dt * glm::normalize(moveDir);
        }
    }
}  
