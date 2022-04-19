/******************************************************************************/
/*!
\file   camera.hpp
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

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace nekographics {

    class NKCamera {
    public:
        void setOrthographicProjection(
            float left, float right, float top, float bottom, float near, float far);
        void setPerspectiveProjection(float fovy, float aspect, float near, float far);

        void setViewDirection(glm::vec3 position, glm::vec3 direction, glm::vec3 up = glm::vec3{ 0.f, -1.f, 0.f });//setting the view direction of the camera 
        void setViewTarget(glm::vec3 position, glm::vec3 target, glm::vec3 up = glm::vec3{ 0.f, -1.f, 0.f }); //centering of the view
        void setViewYXZ(glm::vec3 position, glm::vec3 rotation);//using euler angles to specify orientation of the camera 

        const glm::mat4& getProjection() const { return projectionMatrix; }
        const glm::mat4& getView() const { return viewMatrix; }

    private:
        glm::mat4 projectionMatrix{ 1.f };
        glm::mat4 viewMatrix{ 1.f };
    };
    
}  // namespace lve