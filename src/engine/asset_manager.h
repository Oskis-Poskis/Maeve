#pragma once

#include <vector>
#include <memory>
#include <unordered_map>

#include "../../lib/glm/glm.hpp"
#include "../common/shader.h"

namespace AssetManager
{
    struct VtxData
    {
        glm::vec3 Position;
        glm::vec3 Normal;

        bool operator == (const VtxData& other) { return this->Position == other.Position && this->Normal == other.Normal; }
    };

    struct MeshData
    {
        std::vector<VtxData> VertexData;
        std::vector<uint> Indices;
    };

    struct Mesh
    {
        uint VAO;
        uint VBO;
        uint EBO;
        int  TriangleCount;
        bool UseElements;

        Mesh(std::vector<VtxData> VertexData);
        Mesh(std::vector<VtxData> VertexData, std::vector<uint> Faces);
    };

    void Initialize();
    void AddMeshByData(std::vector<VtxData> VertexData, std::vector<uint> Faces, std::string Name);
    
    inline std::unordered_map<std::string, Mesh> Meshes;
    inline std::unique_ptr<Shader> S_GBuffers;
    inline std::unique_ptr<Shader> S_Lambert;
    inline int UniqueMeshTriCount;

    inline glm::mat4 OrthoProjMat4;

    namespace ObjLoader
    {
        void Load(std::string Path, std::string MeshName);
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
        inline std::vector<uint> PlaneIndices
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
        inline std::vector<uint> CubeIndices
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