#include "vk_model.hpp"
#include "NK_utils.hpp"


//libs
#define TINYOBJLOADER_IMPLEMENTATION
#include "tinyobjloader/tiny_obj_loader.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>


// std
#include <iostream>
#include <cassert>
#include <cstring>
#include <unordered_map>

namespace std {
    template <>
    struct hash<nekographics::NKModel::Vertex> {
        size_t operator()(nekographics::NKModel::Vertex const& vertex) const {
            size_t seed = 0;
            nekographics::hashCombine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv);
            return seed;
        }
    };
}  // namespace std

namespace nekographics {

    NKModel::NKModel(NKDevice& device, const NKModel::Builder& builder) : m_modelDevice{ device } {
        createVertexBuffers(builder.vertices);
        createIndexBuffers(builder.indices);
    }

    NKModel::NKModel(NKDevice& device, const NKModel::AssimpBuilder& builder) : m_modelDevice{ device } {
        //createVertexBuffers(builder.meshes.data()->vertices);
        //createIndexBuffers(builder.meshes.data()->indices);

        if (builder.meshes.size() > 1) {
            hasChildModels = true;
            for (uint32_t i = 0; i < builder.meshes.size(); ++i) {
                Builder builderTmp{};
                //copying the builder 
                builderTmp.vertices = builder.meshes[i].vertices;
                builderTmp.indices = builder.meshes[i].indices;
                childModels.emplace_back(std::make_unique<NKModel>(device, builderTmp)); //emplace back the child models using the original nk model builder 
            }
        }
        else {
            createVertexBuffers(builder.meshes.data()->vertices);
            createIndexBuffers(builder.meshes.data()->indices);
        }

    }

    NKModel::~NKModel() {

    }

    std::unique_ptr<NKModel> NKModel::createModelFromFile(
        NKDevice& device, const std::string& filepath) {
        Builder builder{};
        builder.loadModel(filepath);
        return std::make_unique<NKModel>(device, builder);
    }

    std::unique_ptr<NKModel> NKModel::processMesh(NKDevice& device, xprim_geom::mesh pMesh){
        Builder builder{};
        builder.loadMesh(pMesh);
        return std::make_unique<NKModel>(device, builder);
    }

    std::unique_ptr<NKModel> NKModel::createAssimpModelFromFile(
        NKDevice& device, const std::string& filepath) {
        AssimpBuilder builder{};
        builder.loadAssimpModel(filepath);
        return std::make_unique<NKModel>(device, builder);
    }

    void NKModel::createVertexBuffers(const std::vector<Vertex>& vertices) {
        vertexCount = static_cast<uint32_t>(vertices.size());//getting the vertex count 
        assert(vertexCount >= 3 && "Vertex count must be at least 3");
        VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;//getting the buffer size 
        uint32_t vertexSize = sizeof(vertices[0]);//getting the vertex size 

        //creating the staging buffer 
        NKBuffer stagingBuffer{
            m_modelDevice,
            vertexSize,
            vertexCount,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        };

        stagingBuffer.map();//mapping the memory
        stagingBuffer.writeToBuffer((void*)vertices.data());//sending the memory cpy to the gpu 

        vertexBuffer = std::make_unique<NKBuffer>(
            m_modelDevice,
            vertexSize,
            vertexCount,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        m_modelDevice.copyBuffer(stagingBuffer.getBuffer(), vertexBuffer->getBuffer(), bufferSize);
    }



    void NKModel::createIndexBuffers(const std::vector<uint32_t>& indices) {
        indexCount = static_cast<uint32_t>(indices.size());
        hasIndexBuffer = indexCount > 0;

        if (!hasIndexBuffer) {
            return;
        }

        VkDeviceSize bufferSize = sizeof(indices[0]) * indexCount;
        uint32_t indexSize = sizeof(indices[0]);

        //setting up the stagging buffer 
        NKBuffer stagingBuffer{
            m_modelDevice,
            indexSize,
            indexCount,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        };

        stagingBuffer.map();//map the buffer on the cpu to the gpu 
        stagingBuffer.writeToBuffer((void*)indices.data());//copy the information into the gpu

        //creating the actual index buffer 
        indexBuffer = std::make_unique<NKBuffer>(
            m_modelDevice,
            indexSize,
            indexCount,
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        m_modelDevice.copyBuffer(stagingBuffer.getBuffer(), indexBuffer->getBuffer(), bufferSize);//copy buffer data to the device, optimal memory 
    }

    void NKModel::draw(VkCommandBuffer commandBuffer) {
        if (hasIndexBuffer) {
            vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
        }
        else {
            vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);// the last 2 zeros because we aren't using any offsets in our data 
        }
    }

    void NKModel::bind(VkCommandBuffer commandBuffer) {
        //binding the buffers 
        VkBuffer buffers[] = { vertexBuffer->getBuffer() };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

        if (hasIndexBuffer) {
            vkCmdBindIndexBuffer(commandBuffer, indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
        }
    }

    std::vector<VkVertexInputBindingDescription> NKModel::Vertex::getBindingDescriptions() {
        std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
        //setting the binding discripption
        bindingDescriptions[0].binding = 0;//the first binding at index 0
        bindingDescriptions[0].stride = sizeof(Vertex);//setting the strides
        bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescriptions;
    }

    std::vector<VkVertexInputAttributeDescription> NKModel::Vertex::getAttributeDescriptions() {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

        //telling the vertex descriptions, 
        attributeDescriptions.push_back({ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position) });  //structure - binding , location , format , offset
        attributeDescriptions.push_back({ 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, bitangent) }); //structure - binding , location , format , offset
        attributeDescriptions.push_back({ 2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, tangent) });   //structure - binding , location , format , offset
        attributeDescriptions.push_back({ 3, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal) });    //structure - binding , location , format , offset
        attributeDescriptions.push_back({ 4, 0, VK_FORMAT_R32G32_SFLOAT,    offsetof(Vertex, uv) });           //structure - binding , location , format , offset
        attributeDescriptions.push_back({ 5, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color) });     //structure - binding , location , format , offset

        return attributeDescriptions;
    }



    void NKModel::Builder::loadModel(const std::string& filepath) {
        tinyobj::attrib_t attrib;//the tiny obj attribute 
        std::vector<tinyobj::shape_t> shapes;//vector for storing the shapes
        std::vector<tinyobj::material_t> materials;//vector for storing the materials
        std::string warn, err;

        //load in the object 
        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.c_str())) {
            throw std::runtime_error(warn + err);
        }

        //clearing the vectices & the indices 
        vertices.clear();
        indices.clear();

        std::unordered_map<Vertex, uint32_t> uniqueVertices{};//setting a map of unqiue vertices 
        //loop through the shapes 
        for (const auto& shape : shapes) {
            for (const auto& index : shape.mesh.indices) {
                Vertex vertex{};

                if (index.vertex_index >= 0) {
                    vertex.position = {
                        attrib.vertices[3 * index.vertex_index + 0],
                        attrib.vertices[3 * index.vertex_index + 1],
                        attrib.vertices[3 * index.vertex_index + 2],
                    };

                    vertex.color = {
                        attrib.colors[3 * index.vertex_index + 0],
                        attrib.colors[3 * index.vertex_index + 1],
                        attrib.colors[3 * index.vertex_index + 2],
                    };
                }

                if (index.normal_index >= 0) {
                    vertex.normal = {
                        attrib.normals[3 * index.normal_index + 0],
                        attrib.normals[3 * index.normal_index + 1],
                        attrib.normals[3 * index.normal_index + 2],
                    };
                }

                if (index.texcoord_index >= 0) {
                    vertex.uv = {
                        attrib.texcoords[2 * index.texcoord_index + 0],
                        attrib.texcoords[2 * index.texcoord_index + 1],
                    };
                }

                if (uniqueVertices.count(vertex) == 0) {
                    uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                    vertices.push_back(vertex);//push back the vertices 
                }
                indices.push_back(uniqueVertices[vertex]);//push back the indices 
            }
        }
    }

    void NKModel::Builder::loadMesh(xprim_geom::mesh pMesh) {
       
        //clearing the vectices & the indices 
        vertices.clear();
        indices.clear();

       //
       // Recompute the tangent vectors
       //
        if (false) for (int index = 0, n = (int)pMesh.m_Indices.size(); index < n; index += 3)
        {
            std::array<glm::vec3, 3> pos;
            std::array<glm::vec2, 3> uv;

            // copy data for the 3 vertices
            for (int i = 0; i < 3; ++i)
            {
                const auto& Vert = pMesh.m_Vertices[pMesh.m_Indices[index + i]];
                pos[i] = { Vert.m_Position.m_X, Vert.m_Position.m_Y, Vert.m_Position.m_Z };
                uv[i] = { Vert.m_Texcoord.m_X, Vert.m_Texcoord.m_Y };
            }

            //
            // Compute the Tangent and Binormal Vectors for the first vertex
            //
            const auto edge1 = pos[1] - pos[0];
            const auto edge2 = pos[2] - pos[0];
            const auto deltaUV1 = uv[1] - uv[0];
            const auto deltaUV2 = uv[2] - uv[0];

            const float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

            const glm::vec3 Tangent = glm::vec3
            { f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x)
            , f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y)
            , f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z)
            };

            glm::vec3 normTangent = glm::normalize(Tangent);

            const glm::vec3 Bitangent = glm::vec3
            { f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x)
            , f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y)
            , f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z)
            };

            glm::vec3 normBitangent = glm::normalize(Bitangent);

            //
            // Make sure everything is correct
            //
            for (int i = 0; i < 3; ++i)
            {
                auto& Vert = pMesh.m_Vertices[pMesh.m_Indices[index + i]];
                    
                Vert.m_Tangent.m_X = normTangent.x;
                Vert.m_Tangent.m_Y = normTangent.y;
                Vert.m_Tangent.m_Z = normTangent.z;

                const glm::vec3 ONormal{ Vert.m_Normal.m_X,   Vert.m_Normal.m_Y, Vert.m_Normal.m_Z };
                const glm::vec3 OTangent{ Vert.m_Tangent.m_X, Vert.m_Tangent.m_Y, Vert.m_Tangent.m_Z };
                glm::vec3 normNormal = glm::normalize(ONormal);
                const glm::vec3 OBitangent = glm::cross(OTangent, normNormal);//getting the bitangent 
            }
        }

        indices = pMesh.m_Indices;//storing the indices 

        vertices.resize(pMesh.m_Vertices.size());//resize the vertices to store 

        for (int i = 0; i < vertices.size(); ++i) {
            //calculate the tangent 
            vertices[i].tangent = { pMesh.m_Vertices[i].m_Tangent.m_X,pMesh.m_Vertices[i].m_Tangent.m_Y ,pMesh.m_Vertices[i].m_Tangent.m_Z };
            glm::vec3 normNormal = glm::normalize(glm::vec3{ pMesh.m_Vertices[i].m_Normal.m_X, pMesh.m_Vertices[i].m_Normal.m_Y ,pMesh.m_Vertices[i].m_Normal.m_Z });
            glm::vec3 crossTangentNorm = glm::cross(glm::vec3(pMesh.m_Vertices[i].m_Tangent.m_X, pMesh.m_Vertices[i].m_Tangent.m_Y, pMesh.m_Vertices[i].m_Tangent.m_Z), normNormal);
            vertices[i].bitangent = { crossTangentNorm.x,crossTangentNorm.y,crossTangentNorm.z };

            //assigning the variables 
            vertices[i].normal = { pMesh.m_Vertices[i].m_Normal.m_X,pMesh.m_Vertices[i].m_Normal.m_Y ,pMesh.m_Vertices[i].m_Normal.m_Z };
            vertices[i].position = { pMesh.m_Vertices[i].m_Position.m_X,pMesh.m_Vertices[i].m_Position.m_Y,pMesh.m_Vertices[i].m_Position.m_Z };
            vertices[i].uv = { pMesh.m_Vertices[i].m_Texcoord.m_X,pMesh.m_Vertices[i].m_Texcoord.m_Y};
        }
    }

    //helper function for assimpmodel

    NKModel::Mesh  NKModel::AssimpBuilder::processMesh(aiMesh* mesh, const aiScene* scene) {

        UNREFERENCED_PARAMETER(scene);

        // Data to fill
        std::vector<Vertex>			Vertices;
        std::vector<std::uint32_t>	Indices;

        // Walk through each of the mesh's vertices
        for (auto i = 0u; i < mesh->mNumVertices; ++i)
        {
            Vertex Vertextmp;

            Vertextmp.position = glm::vec3
            (static_cast<float>(mesh->mVertices[i].x)
                , static_cast<float>(-mesh->mVertices[i].y)
                , static_cast<float>(mesh->mVertices[i].z)
            );

            if (mesh->mTextureCoords[0])
            {
                Vertextmp.uv = glm::vec2
                (static_cast<float>(mesh->mTextureCoords[0][i].x)
                    , static_cast<float>(mesh->mTextureCoords[0][i].y)
                );
            }

            if (mesh->mColors[0])
            {
                Vertextmp.color = glm::vec4
                (static_cast<float>(mesh->mColors[0][i].r)
                    , static_cast<float>(mesh->mColors[0][i].g)
                    , static_cast<float>(mesh->mColors[0][i].b)
                    , static_cast<float>(mesh->mColors[0][i].a)
                );
            }

            if (mesh->HasNormals()) {
                Vertextmp.normal = glm::vec3
                (static_cast<float>(mesh->mNormals[i].x)
                    , static_cast<float>(mesh->mNormals[i].y)
                    , static_cast<float>(mesh->mNormals[i].z)
                );
            }

            //setting tangent & bitangent 
            if (mesh->HasTangentsAndBitangents()) {
                //setting tangent 
                Vertextmp.tangent.x = mesh->mTangents[i].x;
                Vertextmp.tangent.y = mesh->mTangents[i].y;
                Vertextmp.tangent.z = mesh->mTangents[i].z;
                //setting bitangent
                Vertextmp.bitangent.x = mesh->mBitangents[i].x;
                Vertextmp.bitangent.y = mesh->mBitangents[i].y;
                Vertextmp.bitangent.z = mesh->mBitangents[i].z;
            }


            Vertices.push_back(Vertextmp);
        }

        // Walk thourgh the faces
        for (auto i = 0u; i < mesh->mNumFaces; ++i)
        {
            const auto& Face = mesh->mFaces[i];

            for (auto j = 0u; j < Face.mNumIndices; ++j)
                Indices.push_back(Face.mIndices[j]);
        }
        return Mesh(Vertices, Indices);
    }

    void NKModel::AssimpBuilder::processNode(aiNode* node, const aiScene* scene) {
        for (unsigned int i = 0; i < node->mNumMeshes; i++) {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene));
        }
        for (unsigned int i = 0; i < node->mNumChildren; i++) {
            processNode(node->mChildren[i], scene);
        }
    }

    void NKModel::AssimpBuilder::loadAssimpModel(const std::string& filepath) {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(filepath.c_str(), 
            aiProcess_Triangulate                  // Make sure we get triangles rather than nvert polygons
            | aiProcess_LimitBoneWeights           // 4 weights for skin model max
            | aiProcess_GenUVCoords                // Convert any type of mapping to uv mapping
            | aiProcess_TransformUVCoords          // preprocess UV transformations (scaling, translation ...)
            | aiProcess_FindInstances              // search for instanced meshes and remove them by references to one master
            | aiProcess_CalcTangentSpace           // calculate tangents and bitangents if possible
            | aiProcess_JoinIdenticalVertices      // join identical vertices/ optimize indexing
            | aiProcess_RemoveRedundantMaterials   // remove redundant materials
            | aiProcess_FindInvalidData            // detect invalid model data, such as invalid normal vectors
            | aiProcess_PreTransformVertices       // pre-transform all vertices
            | aiProcess_FlipUVs                    // flip the V to match the Vulkans way of doing UVs
        );
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
            return;
        }
        processNode(scene->mRootNode, scene);
    }


}  