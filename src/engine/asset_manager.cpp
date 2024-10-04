#include <iostream>
#include <iterator>
#include <fstream>

#include "../../lib/glad/glad.h"
#include "asset_manager.h"
#include "render_engine.h"
#include "../ui/text_renderer.h"
#include "../ui/ui.h"
#include "../common/stat_counter.h"

namespace AssetManager
{
    void Resize(int width, int height);

    void Initialize()
    {
        Engine::RegisterResizeCallback(Resize);
        Resize(Engine::GetWindowSize().x, Engine::GetWindowSize().y);

        AddMeshByData(AssetManager::Presets::PlaneVtxData, AssetManager::Presets::PlaneIndices, "plane");
        AddMeshByData(AssetManager::Presets::CubeVtxData, AssetManager::Presets::CubeIndices, "cube");
        
        S_GBuffers = std::make_unique<Shader>("/../res/shaders/deferred/gbuffers");
        S_Lambert = std::make_unique<Shader>("/../res/shaders/lambert");
    }

    Mesh::Mesh(std::vector<VtxData> VertexData, std::vector<uint> Indices)
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
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,  sizeof(uint) * Indices.size(), &Indices[0], GL_STATIC_DRAW);

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

    void AddMeshByData(std::vector<VtxData> VertexData, std::vector<uint> Indices, std::string Name)
    {   
        Meshes.insert( {Name, Mesh(VertexData, Indices)} );
    }

    void Resize(int width, int height)
    {
        OrthoProjMat4 = glm::ortho(0.0f, (float)width, 0.0f, (float)height);
    }

    namespace ObjLoader
    {
        void Load(std::string Path, std::string MeshName)
        {
            std::cout << "Loading Obj file: " + Statistics::ProjectPath + Path << std::endl;

            std::vector<uint> positionIndices, uvIndices, normalIndices;
            std::vector<glm::vec3> positions, normals;
            std::vector<glm::vec2> uvs;

            glm::vec3 v;
            glm::vec2 t;
            int numIndices = 0;
            int matches = 0;

            std::vector<std::string> lines;
            std::ifstream file;
            char text[256];

            file.open(Statistics::ProjectPath + Path);
            while (!file.eof())
            {
                file.getline(text, 256);
                lines.push_back(text);
            }
            file.close();

            for (int i = 0; i < lines.size(); i++)
            {
                if (lines[i][0] == 'f' && lines[i][1] == ' ')
                {
                    uint vertexIndex[3], uvIndex[3], normalIndex[3];
                    matches = sscanf(lines[i].c_str(), "f %d/%d/%d %d/%d/%d %d/%d/%d\n",
                                     &vertexIndex[0], &uvIndex[0], &normalIndex[0],
                                     &vertexIndex[1], &uvIndex[1], &normalIndex[1],
                                     &vertexIndex[2], &uvIndex[2], &normalIndex[2]);

                    if (matches != 9)
                    {
                        matches = sscanf(lines[i].c_str(), "f %d//%d %d//%d %d//%d\n",
                                         &vertexIndex[0], &uvIndex[0], &normalIndex[0],
                                         &vertexIndex[1], &uvIndex[1], &normalIndex[1],
                                         &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
                    }

                    // std::cout << "Face indices count: "<< matches << std::endl;

                    break;
                }
            }

            for (int i = 0; i < lines.size(); i++)
            {
                if (lines[i][0] == 'v' && lines[i][1] == ' ')
                {
                    sscanf(lines[i].c_str(), "v %f %f %f", &v.x, &v.y, &v.z);
                    positions.push_back(v);
                }
                else if (lines[i][1] == 'n' && lines[i][2] == ' ')
                {
                    sscanf(lines[i].c_str(), "vn %f %f %f", &v.x, &v.y, &v.z);
                    normals.push_back(v);
                }
                else if (lines[i][1] == 't' && lines[i][2] == ' ')
                {
                    sscanf(lines[i].c_str(), "vt %f %f", &t.x, &t.y);
                    uvs.push_back(t);
                }
                else if (lines[i][0] == 'f' && lines[i][1] == ' ')
                {
                    numIndices++;

                    uint vertexIndex[3], uvIndex[3], normalIndex[3];
                    if (matches == 9)
                    {
                        sscanf(lines[i].c_str(), "f %d/%d/%d %d/%d/%d %d/%d/%d\n",
                               &vertexIndex[0], &uvIndex[0], &normalIndex[0],
                               &vertexIndex[1], &uvIndex[1], &normalIndex[1],
                               &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
                    }
                    else if (matches == 6)
                    {
                        sscanf(lines[i].c_str(), "f %d//%d %d//%d %d//%d\n",
                               &vertexIndex[0], &normalIndex[0],
                               &vertexIndex[1], &normalIndex[1],
                               &vertexIndex[2], &normalIndex[2]);
                    }

                    positionIndices.push_back(vertexIndex[0] - 1);
                    positionIndices.push_back(vertexIndex[1] - 1);
                    positionIndices.push_back(vertexIndex[2] - 1);
                    uvIndices.push_back(uvIndex[0] - 1);
                    uvIndices.push_back(uvIndex[1] - 1);
                    uvIndices.push_back(uvIndex[2] - 1);
                    normalIndices.push_back(normalIndex[0] - 1);
                    normalIndices.push_back(normalIndex[1] - 1);
                    normalIndices.push_back(normalIndex[2] - 1);
                }
            }

            int numpositions = positionIndices.size();
            int numnormals = normalIndices.size();
            int numuvs = uvIndices.size();

            std::vector<VtxData> vertices;
            std::vector<uint> indices;

            VtxData vertex{};
            for (uint i = 0; i < numIndices * 3; i++)
            {
                vertex =
                {
                    positions[positionIndices[i]],
                    normals[normalIndices[i]]
                };
                vertices.push_back(vertex);
            }

            std::cout << "Obj triangle Count: " << vertices.size() / 3 << "\n" << std::endl;

            Meshes.insert( {MeshName, Mesh(vertices)} );
        }
    }
}