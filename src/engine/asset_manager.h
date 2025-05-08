#pragma once

#include <vector>
#include <memory>
#include <unordered_map>

#include <glm/glm.hpp>

#include "../common/shader.h"
#include "../common/camera.h"

namespace AM
{
    struct VtxData
    {
        glm::vec3 Position;
        glm::vec3 Normal;
        
        VtxData(glm::vec3 pos, glm::vec3 norm) : Position(pos), Normal(norm) {}
    };

    struct MeshData
    {
        std::vector<VtxData> VertexData;
        std::vector<unsigned int> Indices;
    };

    struct Tri
    {
        Tri() : id0(0), id1(0), id2(0) {}
        Tri(unsigned int Id0, unsigned int Id1, unsigned int Id2) : id0(Id0), id1(Id1), id2(Id2) {}

        unsigned int id0, id1, id2;

        glm::vec3 GetCenter(const std::vector<VtxData>& vertices) const;
    };

    struct AABB
    {
        AABB() : min(glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX)), max(glm::vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX)) {}
        AABB(glm::vec3 Min, glm::vec3 Max) : min(Min), max(Max) {}

        glm::vec3 min;
        glm::vec3 max;

        glm::vec3 GetCenter();
        glm::vec3 GetScale();
        glm::mat4 GetMatrixRelativeToParent(const glm::mat4& ParentMatrix);

        static AABB Compute(const std::vector<VtxData>& vertices, const std::vector<Tri>& triIndices, unsigned int start, unsigned int count);
        static AABB Combine(const AABB& a, const AABB& b);
        static int getLongestAxis(const AABB& aabb);
        void Merge(const AABB& other);
    };

    struct Ray
    {
        Ray(glm::vec3 Origin, glm::vec3 Direction) : origin(Origin), direction(Direction) {}
        glm::vec3 origin;
        glm::vec3 direction;
        float minDistance = 0.0f;
        float maxDistance = FLT_MAX;

        bool IntersectAABB(const AABB& aabb, float& tMin, float& tMax) const;
        bool IntersectTri(const Tri& tri, const std::vector<VtxData>& vertices, float& out) const;
    };

    struct ClosestHit {
        bool hit = false;
        float t = FLT_MAX;
        glm::vec3 v0, v1, v2;
        glm::vec3 n0, n1, n2;
    };

    struct BVH_Node
    {
        AABB aabb;
        bool isLeaf = false;
        unsigned int leftChild  = UINT32_MAX;
        unsigned int rightChild = UINT32_MAX;
        unsigned int start = 0;
        unsigned int count = 0;
    };

    struct BVH
    {
        public:
            unsigned int rootIdx;
            std::vector<BVH_Node> bvhNodes;
            std::vector<Tri> triIndices;

            void Build(const std::vector<VtxData>& vertices);
            void DrawBVHRecursive(unsigned int nodeIdx, unsigned int curDepth, unsigned int minDepth, unsigned int maxDepth, const glm::mat4& parentMatrix);
            void TraverseBVH_Ray(unsigned int nodeIdx, Ray& ray,
                                 const std::vector<VtxData>& vertices,
                                 ClosestHit& closestHit, ClosestHit& closestHitAABB,
                                 const glm::mat4& parentMatrix = glm::mat4(1.0f),
                                 bool drawDebug = false);

        private:
            unsigned int build_recursive(const std::vector<VtxData>& vertices, unsigned int start, unsigned int count);
    };

    struct Mesh
    {
        unsigned int VAO;
        unsigned int VBO;
        unsigned int EBO;
        int  TriangleCount;
        bool UseElements;

        std::vector<VtxData> vertexData;
        std::vector<unsigned int> indices;

        AABB aabb;
        unsigned int rootNodeId = 0;
        unsigned int nodesUsed  = 1;
        BVH bvh;
        
        Mesh(const std::vector<VtxData>& VertexData);
        Mesh(const std::vector<VtxData>& VertexData, std::vector<unsigned int> Faces);
    };

    void Initialize();
    void AddMeshByData(const std::vector<VtxData>& VertexData, std::string Name);
    void AddMeshByData(const std::vector<VtxData>& VertexData, std::vector<unsigned int> Faces, std::string Name);
    // std::vector<glm::vec3> ExtractPositionsFromVtxData(const std::vector<VtxData>& vertexData);
    
    inline std::unordered_map<std::string, Mesh> Meshes;
    inline std::vector<std::string> MeshNames;
    inline std::vector<std::string> LightNames = { "Point Light" };

    inline std::unique_ptr<Shader> S_SingleColor;
    // inline std::unique_ptr<Shader> S_Lambert;
    inline std::unique_ptr<Shader> S_BVHVis;
    inline std::unique_ptr<Shader> S_BVHVisInstanced;

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

        inline std::vector<VtxData> CubeOutlineVtxData = {
            VtxData(glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(0.0f)), // 0
            VtxData(glm::vec3( 1.0f, -1.0f, -1.0f), glm::vec3(0.0f)), // 1
            VtxData(glm::vec3( 1.0f,  1.0f, -1.0f), glm::vec3(0.0f)), // 2
            VtxData(glm::vec3(-1.0f,  1.0f, -1.0f), glm::vec3(0.0f)), // 3
            VtxData(glm::vec3(-1.0f, -1.0f,  1.0f), glm::vec3(0.0f)), // 4
            VtxData(glm::vec3( 1.0f, -1.0f,  1.0f), glm::vec3(0.0f)), // 5
            VtxData(glm::vec3( 1.0f,  1.0f,  1.0f), glm::vec3(0.0f)), // 6
            VtxData(glm::vec3(-1.0f,  1.0f,  1.0f), glm::vec3(0.0f))  // 7
        };
        inline std::vector<unsigned int> CubeOutlineIndices = {
            0, 1, 1, 2, 2, 3, 3, 0, // bottom
            4, 5, 5, 6, 6, 7, 7, 4, // top
            0, 4, 1, 5, 2, 6, 3, 7  // sides
        };

    }
}