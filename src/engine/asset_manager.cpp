#include <iostream>
#include <iterator>
#include <fstream>

#include <glad/glad.h>
#include "asset_manager.h"
#include "render_engine.h"
#include "../ui/text_renderer.h"
#include "../ui/ui.h"
#include "../common/stat_counter.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace AssetManager
{
    void Resize(int width, int height);

    void Initialize()
    {
        Engine::RegisterResizeCallback(Resize);
        Engine::RegisterEditorFunction([&]() { EditorCam.Update(); });
        Resize(Engine::GetWindowSize().x, Engine::GetWindowSize().y);

        AddMeshByData(AssetManager::Presets::PlaneVtxData, AssetManager::Presets::PlaneIndices, "plane");
        AddMeshByData(AssetManager::Presets::CubeVtxData, AssetManager::Presets::CubeIndices, "cube");
        AddMeshByData(std::vector<VtxData> {}, "empty");
        
        S_GBuffers = std::make_unique<Shader>("/res/shaders/deferred/draw_gbuffers");
        S_SingleColor = std::make_unique<Shader>("/res/shaders/debug/singlecolor");

        EditorCam = Camera(glm::vec3(0.0f, 0.0f, 3.0f), 70, -90.0f, 0.0f);
    }

    Mesh::Mesh(std::vector<VtxData> VertexData, std::vector<unsigned int> Indices)
    {
        UseElements = true;
        TriangleCount = Indices.size() / 3;

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * VertexData.size(), &VertexData[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,  sizeof(unsigned int) * Indices.size(), &Indices[0], GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)offsetof(VtxData, Normal));
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        UniqueMeshTriCount += Indices.size() / 3;
    }

    Mesh::Mesh(std::vector<VtxData> VertexData)
    {
        UseElements = false;
        TriangleCount = VertexData.size() / 3;

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * VertexData.size(), &VertexData[0], GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)offsetof(VtxData, Normal));
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        UniqueMeshTriCount += TriangleCount;    
    }

    void AddMeshByData(std::vector<VtxData> VertexData, std::string Name)
    {
        Meshes.insert( {Name, Mesh(VertexData)} );
        MeshNames.push_back(Name);
    }

    void AddMeshByData(std::vector<VtxData> VertexData, std::vector<unsigned int> Indices, std::string Name)
    {   
        Meshes.insert( {Name, Mesh(VertexData, Indices)} );
        MeshNames.push_back(Name);
    }

    void Resize(int width, int height)
    {
        OrthoProjMat4 = glm::ortho(0.0f, (float)width, 0.0f, (float)height);
        ProjMat4 = glm::perspective(glm::radians(AssetManager::EditorCam.Fov), (float)width / height, 0.1f, 1000.0f);
    }
}