#include <iostream>
#include <iterator>
#include <fstream>
#include <chrono>
using namespace std::chrono;

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "asset_manager.h"
#include "scene_manager.h"
#include "render_engine.h"
#include "../ui/text_renderer.h"
#include "../ui/ui.h"
#include "../common/stat_counter.h"
#include "../common/qk.h"
#include "../common/input.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace AM
{
    void Resize(int width, int height);
    void VisualizeMeshBVH();
    void ReloadShaders();

    void Initialize()
    {
        Engine::RegisterResizeCallback(Resize);
        Engine::RegisterEditorFunction([&]() { EditorCam.Update(); });
        Engine::RegisterEditorDraw3DFunction(VisualizeMeshBVH);
        Engine::RegisterEditorReloadShadersFunction(ReloadShaders);

        Resize(Engine::GetWindowSize().x, Engine::GetWindowSize().y);

        AddMeshByData(AM::Presets::CubeOutlineVtxData, AM::Presets::CubeOutlineIndices, "MV::CUBEOUTLINE");
        AddMeshByData(AM::Presets::PlaneVtxData, AM::Presets::PlaneIndices, "plane");
        AddMeshByData(AM::Presets::CubeVtxData, AM::Presets::CubeIndices, "cube");
        AddMeshByData(std::vector<VtxData> {}, "empty");
        
        S_GBuffers        = std::make_unique<Shader>("/res/shaders/deferred/draw_gbuffers");
        S_SingleColor     = std::make_unique<Shader>("/res/shaders/editor/singlecolor");
        S_BVHVis          = std::make_unique<Shader>("/res/shaders/editor/bvh_vis");
        S_BVHVisInstanced = std::make_unique<Shader>("/res/shaders/editor/bvh_vis_instanced");

        EditorCam         = Camera(glm::vec3(0.0f, -3.0f, 0.0f), 70, -90.0f, 0.0f);
        EditorCam.WorldUp = glm::vec3(0.0f, 0.0f, 1.0f);
        EditorCam.Front   = glm::vec3(0.0f, 1.0f, 0.0f);
    }
    
    Mesh::Mesh(const std::vector<VtxData>& VertexData, std::vector<unsigned int> Indices)
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
        vertexData = VertexData;
        indices    = Indices;

        qk::StartTimer();
        bvh.Build(VertexData);
        std::cout << "[:] Built bvh for " << qk::FmtK(int(VertexData.size()) / 3) << " triangles in " << qk::StopTimer() << " seconds\n";

        qk::StartTimer();
        /* EDITOR ONLY */ qk::PrepareBVHVis(bvh.bvhNodes);
        std::cout << "[:] Built bvh debug in " << qk::StopTimer() << " seconds\n";
    }

    Mesh::Mesh(const std::vector<VtxData> &VertexData)
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
        vertexData = VertexData;
        
        qk::StartTimer();
        bvh.Build(VertexData);
        std::cout << "[:] Built bvh for " << qk::FmtK(int(VertexData.size()) / 3) << " triangles in " << qk::StopTimer() << " seconds\n";

        qk::StartTimer();
        /* EDITOR ONLY */ qk::PrepareBVHVis(bvh.bvhNodes);
        std::cout << "[:] Built bvh debug in " << qk::StopTimer() << " seconds\n";
    }

    void AddMeshByData(const std::vector<VtxData>& VertexData, std::string Name)
    {
        Meshes.insert( {Name, Mesh(VertexData)} );
        MeshNames.push_back(Name);
    }

    void AddMeshByData(const std::vector<VtxData>& VertexData, std::vector<unsigned int> Indices, std::string Name)
    {   
        Meshes.insert( {Name, Mesh(VertexData, Indices)} );
        MeshNames.push_back(Name);
    }

    void Resize(int width, int height)
    {
        OrthoProjMat4 = glm::ortho(0.0f, (float)width, 0.0f, (float)height);
        ProjMat4 = glm::perspective(glm::radians(AM::EditorCam.Fov), (float)width / height, 0.1f, 1000.0f);
    }

    int minDepth = 0;
    int maxDepth = 6;
    void VisualizeMeshBVH()
    {
        if (Engine::debugMode == 2 && !SM::SceneNodeNames.empty()) {
            SM::SceneNode* node = SM::SceneNodes[SM::GetSelectedIndex()];
            if (node->GetType() == SM::NodeType::Object_)
            {
                SM::Object* obj  = SM::GetObjectFromNode(node);
                AM::Mesh&   mesh = AM::Meshes.at(obj->GetMeshID());
                AM::BVH&    bvh  = mesh.bvh;

                bool inGame = Input::GetInputContext() == Input::InputContext::Game;
                if (inGame && Input::KeyPressed(GLFW_KEY_DOWN))  maxDepth--;
                if (inGame && Input::KeyPressed(GLFW_KEY_UP))    maxDepth++;
                if (inGame && Input::KeyPressed(GLFW_KEY_LEFT))  minDepth = std::max(minDepth - 1, 0);
                if (inGame && Input::KeyPressed(GLFW_KEY_RIGHT)) minDepth++;

                if (!Input::MouseButtonDown(GLFW_MOUSE_BUTTON_1)) bvh.DrawBVHRecursive(bvh.rootIdx, 0, minDepth, maxDepth, obj->GetModelMatrix());
                else {
                    glm::vec2 mousePos  = glm::vec2(Input::GetMouseX(), Input::GetMouseY());
                    glm::vec3 nearPoint = qk::ScreenToWorld(mousePos, 0.0f);
                    glm::vec3 farPoint  = qk::ScreenToWorld(mousePos, 1.0f);

                    glm::vec3 worldDir    = glm::normalize(farPoint - nearPoint);
                    glm::vec3 worldOrigin = nearPoint;

                    glm::mat4 modelInv  = glm::inverse(obj->GetModelMatrix());
                    glm::vec3 objOrigin = glm::vec3(modelInv * glm::vec4(worldOrigin, 1.0f));
                    glm::vec3 objDir    = glm::vec3(modelInv * glm::vec4(worldDir, 0.0f));
                    AM::Ray ray(objOrigin, objDir);
                    
                    AM::ClosestHit closestTri;
                    AM::ClosestHit closestAABB;
                    bvh.TraverseBVH_Ray(bvh.rootIdx, ray, mesh.vertexData, closestTri, closestAABB, obj->GetModelMatrix(), true);
                    
                    if (closestTri.hit) {
                        glm::vec3 v0 = glm::vec3(obj->GetModelMatrix() * glm::vec4(closestTri.v0, 1.0f));
                        glm::vec3 v1 = glm::vec3(obj->GetModelMatrix() * glm::vec4(closestTri.v1, 1.0f));
                        glm::vec3 v2 = glm::vec3(obj->GetModelMatrix() * glm::vec4(closestTri.v2, 1.0f));

                        glDisable(GL_DEPTH_TEST);
                        qk::DrawLine(v0, v1, {0.0f, 1.0f, 0.0f}, 4);
                        qk::DrawLine(v1, v2, {0.0f, 1.0f, 0.0f}, 4);
                        qk::DrawLine(v2, v0, {0.0f, 1.0f, 0.0f}, 4);
                        glEnable(GL_DEPTH_TEST);

                        qk::DrawTri(v0, v1, v2, {0.0f, 0.0f, 1.0f});
                    }
                }
                // qk::DrawBVHCubesInstanced(obj->GetModelMatrix(), 2);
            }
        }        
    }

    glm::vec3 Tri::GetCenter(const std::vector<VtxData>& vertices) const
    {
        glm::vec3 v0 = vertices[id0].Position;
        glm::vec3 v1 = vertices[id1].Position;
        glm::vec3 v2 = vertices[id2].Position;

        return (v0 + v1 + v2) / 3.0f;
    }

    glm::vec3 AABB::GetCenter()
    {
        return glm::vec3(
            (min.x + max.x) * 0.5f,
            (min.y + max.y) * 0.5f,
            (min.z + max.z) * 0.5f
        );
    }

    glm::vec3 AABB::GetScale()
    {
        return glm::vec3(
            max.x - min.x,
            max.y - min.y,
            max.z - min.z
        ) * 0.5f;
    }

    glm::mat4 AABB::GetMatrixRelativeToParent(const glm::mat4 &ParentMatrix)
    {
        glm::mat4 finalMatrix = ParentMatrix;
        finalMatrix = glm::translate(finalMatrix, AABB::GetCenter());
        finalMatrix = glm::scale(finalMatrix, AABB::GetScale());
        return finalMatrix;
    }

    AABB AABB::Compute(const std::vector<VtxData>& vertices, const std::vector<Tri>& triIndices, unsigned int start, unsigned int count)
    {
        AABB aabb;
        aabb.min = glm::vec3( FLT_MAX);
        aabb.max = glm::vec3(-FLT_MAX);

        for (unsigned int i = start; i < start + count; i++) {
            // glm::vec3 c = triIndices[i].GetCenter(vertices);
            // aabb.min = glm::min(aabb.min, c);
            // aabb.max = glm::max(aabb.max, c);

            glm::vec3 c;
            Tri tri = triIndices[i];
            for (size_t j = 0; j < 3; j++)
            {
                if (j == 0) c = vertices[tri.id0].Position;
                if (j == 1) c = vertices[tri.id1].Position;
                if (j == 2) c = vertices[tri.id2].Position;

                aabb.min = glm::min(aabb.min, c);
                aabb.max = glm::max(aabb.max, c);
            }
        }

        return aabb;
    }

    AABB AABB::Combine(const AABB &a, const AABB &b)
    {
        glm::vec3 min = glm::min(a.min, b.min);
        glm::vec3 max = glm::max(a.max, b.max);
        return AABB(min, max);
    }

    void AABB::Merge(const AABB& other)
    {
        min = glm::min(min, other.min);
        max = glm::max(max, other.max);   
    }

    int AABB::getLongestAxis(const AABB &aabb)
    {
        glm::vec3 extent = aabb.max - aabb.min;
        if (extent.x > extent.y && extent.x > extent.z) return 0; // X axis
        if (extent.y > extent.z) return 1;                        // Y axis
        return 2;                                                 // Z axis
    }

    bool Ray::IntersectAABB(const AABB& aabb, float& tMin, float& tMax) const
    {
        for (int i = 0; i < 3; ++i)
        {
            float invD = 1.0f / direction[i];
            float t0 = (aabb.min[i] - origin[i]) * invD;
            float t1 = (aabb.max[i] - origin[i]) * invD;

            if (invD < 0.0f)
                std::swap(t0, t1);

            tMin = std::max(tMin, t0);
            tMax = std::min(tMax, t1);

            if (tMax < tMin) return false; // No intersection
        }

        return true; // Intersection occurred
    }

    bool Ray::IntersectTri(const Tri& tri, const std::vector<VtxData>& vertices, float& outT) const
    {
        glm::vec3 v0 = vertices[tri.id0].Position;
        glm::vec3 v1 = vertices[tri.id1].Position;
        glm::vec3 v2 = vertices[tri.id2].Position;

        glm::vec3 edge1 = v1 - v0;
        glm::vec3 edge2 = v2 - v0;
        glm::vec3 h = glm::cross(direction, edge2);
        float a = glm::dot(edge1, h);

        const float epsilon = 1e-6f;
        if (fabs(a) < epsilon) return false;

        float f = 1.0f / a;
        glm::vec3 s = origin - v0;
        float u = f * glm::dot(s, h);
        if (u < 0.0f || u > 1.0f) return false;

        glm::vec3 q = glm::cross(s, edge1);
        float v = f * glm::dot(direction, q);
        if (v < 0.0f || (u + v) > 1.0f) return false;

        float t = f * glm::dot(edge2, q);
        if (t > epsilon) {
            outT = t;
            return true;
        }

        return false;
    }


    void BVH::Build(const std::vector<VtxData>& vertices)
    {
        bvhNodes.clear();
        triIndices.clear();

        triIndices.resize(vertices.size() / 3);
        for (unsigned int i = 0; i < triIndices.size(); i++) {
            triIndices[i].id0 = i * 3 + 0;
            triIndices[i].id1 = i * 3 + 1;
            triIndices[i].id2 = i * 3 + 2;
        }

        rootIdx = build_recursive(vertices, 0, triIndices.size());
    }

    size_t LEAF_NODE_THRESHOLD = 1;
    unsigned int BVH::build_recursive(const std::vector<VtxData>& vertices, unsigned int start, unsigned int count)
    {
        BVH_Node node;
        node.aabb = AABB::Compute(vertices, triIndices, start, count);

        if (count <= LEAF_NODE_THRESHOLD) {
            node.isLeaf = true;
            node.start  = start;
            node.count  = count;

            unsigned int nodeIdx = bvhNodes.size();
            bvhNodes.push_back(node);

            return nodeIdx;
        }

        int splitAxis = AABB::getLongestAxis(node.aabb);
        unsigned int mid = start + count / 2;

        // 0.209s for 62.976 tris (debug)
        std::nth_element(
            triIndices.begin() + start,
            triIndices.begin() + mid,
            triIndices.begin() + start + count,
            [&](const Tri& a, const Tri& b) {
                return a.GetCenter(vertices)[splitAxis] < b.GetCenter(vertices)[splitAxis];
            });

        // 0.588s for 62.976 tris (debug)
        // std::sort(triIndices.begin() + start, triIndices.begin() + start + count,
        //     [&](const Tri& a, const Tri& b) {
        //         return a.GetCenter(vertices)[splitAxis] < b.GetCenter(vertices)[splitAxis];
        //     });

        unsigned int left  = build_recursive(vertices, start, mid - start);
        unsigned int right = build_recursive(vertices, mid,   count - (mid - start));
        
        node.leftChild  = left;
        node.rightChild = right;

        unsigned int nodeIdx = bvhNodes.size();
        bvhNodes.push_back(node);

        return nodeIdx;
    }

    void BVH::DrawBVHRecursive(unsigned int nodeIdx, unsigned int curDepth, unsigned int minDepth, unsigned int maxDepth, const glm::mat4& parentMatrix)
    {
        const BVH_Node& node = bvhNodes[nodeIdx];
        if (node.isLeaf) return;
        
        if (curDepth <= maxDepth && curDepth >= minDepth) {
            float hue = 90.0f - (float)(curDepth - minDepth) / (maxDepth - minDepth) * 90.0f;
            glm::vec3 color = qk::HSVToRGB({ hue, 1.0f, 1.0f });
            qk::DrawBVHCube(node.aabb.min, node.aabb.max, parentMatrix, color);
        }
        
        if (curDepth < maxDepth) {
            DrawBVHRecursive(node.leftChild, curDepth + 1, minDepth, maxDepth, parentMatrix);
            DrawBVHRecursive(node.rightChild, curDepth + 1, minDepth, maxDepth, parentMatrix);
        }
    }

    void BVH::TraverseBVH_Ray(unsigned int nodeIdx, Ray& ray, const std::vector<VtxData>& vertices, ClosestHit& closestHit, ClosestHit& closestHitAABB, const glm::mat4& parentMatrix, bool drawDebug)
    {
        const BVH_Node& node = bvhNodes[nodeIdx];

        float tMin = ray.minDistance;
        float tMax = ray.maxDistance;
        float aabbTMin = tMin;
        float aabbTMax = tMax;

        if (!ray.IntersectAABB(node.aabb, aabbTMin, aabbTMax)) return;

        // Track closest AABB hit
        if (aabbTMin < closestHitAABB.t) {
            closestHitAABB.t = aabbTMin;
            closestHitAABB.hit = true;
            closestHitAABB.v0 = node.aabb.min;
            closestHitAABB.v1 = node.aabb.max;
            closestHitAABB.v2 = glm::vec3(0.0f); // optional, unused for AABB
        }

        if (node.isLeaf) {
            for (unsigned int i = 0; i < node.count; i++) {
                Tri tri(triIndices[node.start + i].id0,
                        triIndices[node.start + i].id1,
                        triIndices[node.start + i].id2);

                float t = 0.0f;
                if (ray.IntersectTri(tri, vertices, t) && t < closestHit.t) {
                    closestHit.t = t;
                    closestHit.v0 = vertices[tri.id0].Position;
                    closestHit.v1 = vertices[tri.id1].Position;
                    closestHit.v2 = vertices[tri.id2].Position;
                    closestHit.n0 = vertices[tri.id0].Normal;
                    closestHit.n1 = vertices[tri.id1].Normal;
                    closestHit.n2 = vertices[tri.id2].Normal;
                    closestHit.hit = true;
                }
            }
            return;
        }

        if (drawDebug) {
            glm::vec3 color = glm::vec3(1.0f, 0.0f, 0.0f);
            qk::DrawBVHCube(node.aabb.min, node.aabb.max, parentMatrix, color);
        }

        // Recurse left and right only if valid
        if (node.leftChild != UINT32_MAX)
            TraverseBVH_Ray(node.leftChild,  ray, vertices, closestHit, closestHitAABB, parentMatrix, drawDebug);
        if (node.rightChild != UINT32_MAX)
            TraverseBVH_Ray(node.rightChild, ray, vertices, closestHit, closestHitAABB, parentMatrix, drawDebug);
    }

    void ReloadShaders()
    {
        S_GBuffers->Reload();
        S_SingleColor->Reload();
        S_BVHVis->Reload();
        S_BVHVisInstanced->Reload();
    }
}