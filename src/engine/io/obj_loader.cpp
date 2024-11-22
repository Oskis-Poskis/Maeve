#include <iostream>
#include <iterator>
#include <fstream>
#include <chrono>
using namespace std::chrono;

#include <glad/glad.h>
#include "../asset_manager.h"
#include "../render_engine.h"
#include "../../ui/text_renderer.h"
#include "../../ui/ui.h"
#include "../../common/stat_counter.h"
#include "../../common/qk.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace AssetManager::IO
{
    bool LineStartsWith(std::string line, std::string compare);

    void LoadObj(std::string Path, std::string MeshName)
    {   
        auto start = high_resolution_clock::now();

        std::string filePath = qk::CombinedPath(Statistics::ProjectPath, Path);

        std::cout << "[>] Opening obj file: " << filePath << std::endl;

        std::vector<unsigned int> positionIndices, uvIndices, normalIndices;
        std::vector<glm::vec3> positions, normals;
        std::vector<glm::vec2> uvs;

        glm::vec3 v;
        glm::vec2 t;
        
        int numIndices = 0;
        int matches = 0;
        bool checkedTexCoords = false;

        std::vector<std::string> lines;
        std::ifstream file;
        char text[256];

        file.open(filePath);
        if (!file.is_open())
        {
            std::cout << "[!] Failed to open obj file :(\n";
            return;
        }

        while (!file.eof())
        {
            file.getline(text, 256);
            lines.push_back(text);
        }
        file.close();
        
        for (std::string line : lines)
        {
            if (LineStartsWith(line, "v "))
            {
                sscanf(line.c_str(), "v %f %f %f", &v.x, &v.y, &v.z);
                positions.push_back(v);
            }
            else if (LineStartsWith(line, "vn "))
            {
                sscanf(line.c_str(), "vn %f %f %f", &v.x, &v.y, &v.z);
                normals.push_back(v);
            }
            else if (LineStartsWith(line, "vt "))
            {
                sscanf(line.c_str(), "vt %f %f", &t.x, &t.y);
                uvs.push_back(t);
            }
            else if (LineStartsWith(line, "f "))
            {
                numIndices++;
                // Check number of face members
                if (LineStartsWith(line, "f ") && !checkedTexCoords)
                {
                    unsigned int temp[3];
                    // Has position, normal and texcoord
                    matches = sscanf(line.c_str(), "f %d/%d/%d %d/%d/%d %d/%d/%d\n",
                                     &temp[0], &temp[0], &temp[0],
                                     &temp[1], &temp[1], &temp[1],
                                     &temp[2], &temp[2], &temp[2]);

                    // Only position and normal
                    if (matches != 9)
                    {
                        matches = sscanf(line.c_str(), "f %d//%d %d//%d %d//%d\n",
                                         &temp[0], &temp[0],
                                         &temp[1], &temp[1],
                                         &temp[2], &temp[2]);
                    }

                    checkedTexCoords = true;
                }

                unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
                if (matches == 9)
                {
                    sscanf(line.c_str(), "f %d/%d/%d %d/%d/%d %d/%d/%d\n",
                           &vertexIndex[0], &uvIndex[0], &normalIndex[0],
                           &vertexIndex[1], &uvIndex[1], &normalIndex[1],
                           &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
                }
                else if (matches == 6)
                {
                    sscanf(line.c_str(), "f %d//%d %d//%d %d//%d\n",
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

        VtxData vertex{};
        for (unsigned int i = 0; i < numIndices * 3; i++)
        {
            vertex =
            {
                positions[positionIndices[i]],
                normals[normalIndices[i]]
            };
            vertices.push_back(vertex);
        }

        AssetManager::AddMeshByData(vertices, MeshName);

        auto stop = high_resolution_clock::now();

        auto duration = duration_cast<microseconds>(stop - start).count();
        std::cout << "[:] Loaded obj with " << qk::FmtK(int(vertices.size()) / 3) << " triangles in " << (float)duration / 1000000 << " seconds\n\n";
    }

    bool LineStartsWith(std::string line, std::string comp)
    {
        return line.substr(0, comp.length()) == comp;
    }
}