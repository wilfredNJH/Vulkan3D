#pragma once

#include "vk_model.hpp"

//libs
#include <glm/gtc/matrix_transform.hpp>

// std
#include <memory>
#include <unordered_map>

namespace nekographics {

    //the transform component of the game object
    struct TransformComponent {
        glm::vec3 translation{};
        glm::vec3 scale{ 1.f, 1.f, 1.f };
        glm::vec3 rotation{};

        // Matrix corrsponds to Translate * Ry * Rx * Rz * Scale
        // Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
        // https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
        glm::mat4 mat4();

        glm::mat3 normalMatrix();
    };

    struct PointLightComponent {
        float lightIntensity = 1.0f;
    };



    class NkGameObject {
    public:
        using id_t = unsigned int;
        using Map = std::unordered_map<id_t, NkGameObject>;//unordered map of the game objects


        static NkGameObject createGameObject() {
            static id_t currentId = 0;
            return NkGameObject{ currentId++ };
        }

        static NkGameObject makePointLight(
            float intensity = 10.f, float radius = 0.1f, glm::vec3 color = glm::vec3(1.f));

        NkGameObject(const NkGameObject&) = delete;
        NkGameObject& operator=(const NkGameObject&) = delete;
        NkGameObject(NkGameObject&&) = default;
        NkGameObject& operator=(NkGameObject&&) = default;

        id_t getId() { return id; }

        glm::vec3 color{};//the color component of the model 
        TransformComponent transform{};//transform component of the model which is definitely needed

        //optional pointer components
        std::shared_ptr<NKModel> model{};
        std::unique_ptr<PointLightComponent> pointLight = nullptr;

        NkGameObject(id_t objId = 1) : id{ objId } {}
    private:

        id_t id;
    };
}  // namespace lve
