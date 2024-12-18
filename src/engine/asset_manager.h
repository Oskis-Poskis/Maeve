#pragma once

#include <vector>
#include <memory>
#include <unordered_map>

#include <glm/glm.hpp>
#include "../common/shader.h"
#include "../common/camera.h"

namespace AssetManager
{
    struct VtxData
    {
        glm::vec3 Position;
        glm::vec3 Normal;
    };

    struct MeshData
    {
        std::vector<VtxData> VertexData;
        std::vector<unsigned int> Indices;
    };

    struct Mesh
    {
        unsigned int VAO;
        unsigned int VBO;
        unsigned int EBO;
        int  TriangleCount;
        bool UseElements;

        Mesh(std::vector<VtxData> VertexData);
        Mesh(std::vector<VtxData> VertexData, std::vector<unsigned int> Faces);
    };

    void Initialize();
    void AddMeshByData(std::vector<VtxData> VertexData, std::string Name);
    void AddMeshByData(std::vector<VtxData> VertexData, std::vector<unsigned int> Faces, std::string Name);
    
    inline std::unordered_map<std::string, Mesh> Meshes;
    inline std::vector<std::string> MeshNames;
    inline std::unique_ptr<Shader> S_GBuffers;
    inline std::unique_ptr<Shader> S_SingleColor;
    inline std::unique_ptr<Shader> S_Lambert;
    inline int UniqueMeshTriCount;

    inline Camera EditorCam;

    inline glm::mat4 ProjMat4;
    inline glm::mat4 ViewMat4;
    inline glm::mat4 OrthoProjMat4;

    namespace IO
    {
        void LoadObj(std::string Path, std::string MeshName);
    };

    namespace Presets
    {
        inline std::vector<VtxData> PlaneVtxData =
        {
            VtxData(glm::vec3(-1.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
            VtxData(glm::vec3(-1.0f, 0.0f,  1.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
            VtxData(glm::vec3( 1.0f, 0.0f,  1.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
            VtxData(glm::vec3( 1.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f))
        };
        inline std::vector<unsigned int> PlaneIndices
        {
            0, 1, 2, 0, 2, 3
        };

        inline std::vector<VtxData> CubeVtxData =
        {
            // Top Face
            VtxData(glm::vec3(-1.0f, 1.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
            VtxData(glm::vec3(-1.0f, 1.0f,  1.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
            VtxData(glm::vec3( 1.0f, 1.0f,  1.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
            VtxData(glm::vec3( 1.0f, 1.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f)),

            // Bottom Face
            VtxData(glm::vec3( 1.0f, -1.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
            VtxData(glm::vec3( 1.0f, -1.0f,  1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
            VtxData(glm::vec3(-1.0f, -1.0f,  1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
            VtxData(glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),

            // Front  Face
            VtxData(glm::vec3(-1.0f, -1.0f,  1.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
            VtxData(glm::vec3(-1.0f,  1.0f,  1.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
            VtxData(glm::vec3( 1.0f,  1.0f,  1.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
            VtxData(glm::vec3( 1.0f, -1.0f,  1.0f), glm::vec3(0.0f, 0.0f, 1.0f)),

            // Right  Face
            VtxData(glm::vec3( 1.0f, -1.0f,  1.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
            VtxData(glm::vec3( 1.0f,  1.0f,  1.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
            VtxData(glm::vec3( 1.0f,  1.0f, -1.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
            VtxData(glm::vec3( 1.0f, -1.0f, -1.0f), glm::vec3(1.0f, 0.0f, 0.0f)),

            // Left Face
            VtxData(glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(-1.0f, 0.0f, 0.0f)),
            VtxData(glm::vec3(-1.0f,  1.0f, -1.0f), glm::vec3(-1.0f, 0.0f, 0.0f)),
            VtxData(glm::vec3(-1.0f,  1.0f,  1.0f), glm::vec3(-1.0f, 0.0f, 0.0f)),
            VtxData(glm::vec3(-1.0f, -1.0f,  1.0f), glm::vec3(-1.0f, 0.0f, 0.0f)),

            // Back Face
            VtxData(glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
            VtxData(glm::vec3(-1.0f,  1.0f, -1.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
            VtxData(glm::vec3( 1.0f,  1.0f, -1.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
            VtxData(glm::vec3( 1.0f, -1.0f, -1.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
        };
        inline std::vector<unsigned int> CubeIndices
        {
            0, 1, 2,    0, 2, 3,    // Top Face
            4, 5, 6,    4, 6, 7,    // Bottom Face
            10, 9, 8,   11, 10, 8,  // Front Face 
            14, 13, 12, 15, 14, 12, // Right Face
            18, 17, 16, 19, 18, 16, // Left Face
            20, 21, 22, 20, 22, 23  // Back Face
        };
    }
}