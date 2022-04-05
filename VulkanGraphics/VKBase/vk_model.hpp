#pragma once

#include "vk_device.hpp"
#include "vk_buffer.hpp"
#include "../meshes/xprim_geom.h"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// std
#include <memory>
#include <vector>

namespace nekographics {
    class NKModel {
    public:
        //vertex 
        struct Vertex {
            glm::vec3 position{};
            glm::vec3 bitangent{};
            glm::vec3 tangent{};
            glm::vec3 normal{};
            glm::vec2 uv{};
            glm::vec3 color{};

            static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
            static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

            bool operator==(const Vertex& other) const {
                return position == other.position && color == other.color && normal == other.normal &&
                    uv == other.uv;
            }
        };

        // mesh
        class Mesh {
        public:
            std::vector<Vertex> vertices;
            std::vector<unsigned int> indices;
            Mesh(std::vector<Vertex> v, std::vector<unsigned int> i) {
                this->vertices = v;
                this->indices = i;
            }
        };

        //builder for the model
        struct Builder {
            std::vector<Vertex> vertices{};
            std::vector<uint32_t> indices{};

            void loadModel(const std::string& filepath);
            void loadMesh(xprim_geom::mesh pMesh);
            //void loadAssimpModel(const std::string& filepath);
        };

        struct AssimpBuilder {
            std::vector<Mesh> meshes{};

            Mesh processMesh(aiMesh* mesh, const aiScene* scene);
            void processNode(aiNode* node, const aiScene* scene);
            void loadAssimpModel(const std::string& filepath);;
        };

        NKModel(NKDevice& device, const NKModel::Builder& builders);
        NKModel(NKDevice& device, const NKModel::AssimpBuilder& builders);
        ~NKModel();

        NKModel(const NKModel&) = delete;
        NKModel& operator=(const NKModel&) = delete;

        static std::unique_ptr<NKModel> createModelFromFile(
            NKDevice& device, const std::string& filepath);

        static std::unique_ptr<NKModel> createAssimpModelFromFile(
            NKDevice& device, const std::string& filepath);

        static std::unique_ptr<NKModel> processMesh(NKDevice& device, xprim_geom::mesh pMesh);//processing the custom mesh 


        void bind(VkCommandBuffer commandBuffer);
        void draw(VkCommandBuffer commandBuffer);

    private:
        void createVertexBuffers(const std::vector<Vertex>& vertices);
        void createIndexBuffers(const std::vector<uint32_t>& indices);

        NKDevice& m_modelDevice;//reference to the device 

        //vertex buffer 
        std::unique_ptr<NKBuffer> vertexBuffer;
        uint32_t vertexCount;

        //index buffer 
        bool hasIndexBuffer = false;
        std::unique_ptr<NKBuffer> indexBuffer;
        uint32_t indexCount;
    };
}
