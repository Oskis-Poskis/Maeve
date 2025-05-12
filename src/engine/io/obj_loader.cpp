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
#include <thread>
#include <queue>
#include <filesystem>

namespace AM::IO
{
    bool LineStartsWith(std::string line, std::string compare);
    
    static inline float fastParseFloat(const char* begin, const char* end) {
        float value = 0.0f;
        std::from_chars_result res = std::from_chars(begin, end, value);
        return value;
    }

    static inline int fastParseInt(const char* begin, const char* end) {
        int value = 0;
        std::from_chars_result res = std::from_chars(begin, end, value);
        return value;
    }

    std::vector<VtxData> LoadObjFile(const std::string& relativePath)
    {
        auto start = high_resolution_clock::now();

        namespace fs = std::filesystem;
        std::string path = relativePath;
        std::ifstream file(path, std::ios::in);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open OBJ: " + path);
        }

        std::vector<glm::vec3> positions;
        std::vector<glm::vec2> uvs;
        std::vector<glm::vec3> normals;
        std::vector<VtxData>   vertices;

        // positions.reserve(1024);
        // uvs.reserve(1024);
        // normals.reserve(1024);
        // vertices.reserve(4096);

        std::string line;
        while (std::getline(file, line))
        {
            if (line.size() < 2) continue;
            char type = line[0];
            if (type == 'v' && line[1] == ' ') {
                // v x y z
                const char* ptr = line.c_str() + 2;
                glm::vec3 v;
                // parse three floats
                const char* start = ptr;
                while (*ptr && *ptr == ' ') ++ptr;
                start = ptr;
                while (*ptr && *ptr != ' ') ++ptr;
                v.x = fastParseFloat(start, ptr);
                while (*ptr == ' ') ++ptr;
                start = ptr;
                while (*ptr && *ptr != ' ') ++ptr;
                v.y = fastParseFloat(start, ptr);
                while (*ptr == ' ') ++ptr;
                start = ptr;
                while (*ptr && *ptr != ' ' && *ptr != '\r') ++ptr;
                v.z = fastParseFloat(start, ptr);
                positions.push_back(v);
            }
            else if (type == 'v' && line[1] == 't') {
                // vt u v
                const char* ptr = line.c_str() + 3;
                glm::vec2 t;
                const char* start = ptr;
                while (*ptr && *ptr != ' ') ++ptr;
                t.x = fastParseFloat(start, ptr);
                while (*ptr == ' ') ++ptr;
                start = ptr;
                while (*ptr && *ptr != ' ' && *ptr != '\r') ++ptr;
                t.y = fastParseFloat(start, ptr);
                uvs.push_back(t);
            }
            else if (type == 'v' && line[1] == 'n') {
                // vn x y z
                const char* ptr = line.c_str() + 3;
                glm::vec3 n;
                const char* start = ptr;
                while (*ptr && *ptr != ' ') ++ptr;
                n.x = fastParseFloat(start, ptr);
                while (*ptr == ' ') ++ptr;
                start = ptr;
                while (*ptr && *ptr != ' ') ++ptr;
                n.y = fastParseFloat(start, ptr);
                while (*ptr == ' ') ++ptr;
                start = ptr;
                while (*ptr && *ptr != ' ' && *ptr != '\r') ++ptr;
                n.z = fastParseFloat(start, ptr);
                normals.push_back(n);
            }
            else if (type == 'f' && line[1] == ' ') {
                // f v/t/n v/t/n v/t/n ...
                // triangulate n-gons via fan
                std::vector<int> vi, ti, ni;
                const char* ptr = line.c_str() + 2;
                while (*ptr) {
                    // skip spaces
                    while (*ptr == ' ') ++ptr;
                    if (!*ptr || *ptr=='\r') break;
                    // parse indices
                    const char* start = ptr;
                    // vertex index
                    while (*ptr && *ptr != '/' && *ptr != ' ') ++ptr;
                    int vIdx = fastParseInt(start, ptr) - 1;
                    int tIdx = -1, nIdx = -1;
                    if (*ptr == '/') {
                        ++ptr;
                        // texture?
                        if (*ptr != '/') {
                            start = ptr;
                            while (*ptr && *ptr != '/') ++ptr;
                            tIdx = fastParseInt(start, ptr) - 1;
                        }
                        if (*ptr == '/') {
                            ++ptr;
                            start = ptr;
                            while (*ptr && *ptr != ' ' && *ptr!='\r') ++ptr;
                            nIdx = fastParseInt(start, ptr) - 1;
                        }
                    }
                    vi.push_back(vIdx);
                    ti.push_back(tIdx);
                    ni.push_back(nIdx);
                    // skip to next
                    while (*ptr && *ptr != ' ') ++ptr;
                }
                // build triangles
                for (size_t k = 1; k + 1 < vi.size(); ++k) {
                    for (int idx : {0, (int)k, (int)(k+1)}) {
                        VtxData vert{};
                        vert.Position = positions[vi[idx]];
                        // if (ti[idx] >= 0) vert.uv = uvs[ti[idx]];
                        if (ni[idx] >= 0) vert.Normal = normals[ni[idx]];
                        vertices.push_back(vert);
                    }
                }
            }
        }
        file.close();

        auto stop = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(stop - start).count();
        std::cout << "Loaded " << vertices.size() / 3 << " triangles from " << path << " in " << (float)duration / 1000000<< "\n";

        return vertices;
    }

    void LoadObjAsync(const std::string& path, std::string meshName)
    {
        std::thread([path, meshName]
            {
            auto vertices = LoadObjFile(path);
            qk::PostFunctionToMainThread([v = std::move(vertices), meshName] {
                AM::AddMeshByData(v, meshName);
            });
        }).detach();
    }

    void LoadObjFolderAsync(const std::string& folderPath, const std::string& meshNamePrefix)
    {
        std::thread([folderPath, meshNamePrefix]() {
            namespace fs = std::filesystem;
            for (auto& entry : fs::directory_iterator(folderPath)) {
                if (!entry.is_regular_file()) continue;
                if (entry.path().extension() != ".obj") continue;

                // Compute relative path for loader
                std::string relPath = fs::relative(entry.path(), Stats::ProjectPath).string();
                auto vertices = LoadObjFile(relPath);

                // Compose mesh name using prefix and file stem
                std::string meshName = meshNamePrefix + "_" + entry.path().stem().string();
                qk::PostFunctionToMainThread([v = std::move(vertices), meshName]() mutable {
                    AM::AddMeshByData(v, meshName);
                });
            }
        }).detach();
    }

    bool LineStartsWith(std::string line, std::string comp)
    {
        return line.substr(0, comp.length()) == comp;
    }
}