#include <iostream>
#include <memory>
#include <format>

#include <glm/glm.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "deffered_manager.h"
#include "../render_engine.h"
#include "../asset_manager.h"
#include "../scene_manager.h"
#include "../../common/shader.h"
#include "../../common/qk.h"
#include "../../ui/text_renderer.h"

namespace Deferred
{
    void Resize(int width, int height);
    void ReloadShaders();
    void CheckFBOStatus(std::string FBOName);

    unsigned int gbufferFBO;
    unsigned int defferedQuadVAO;
    std::unique_ptr<Shader> S_fullscreenQuad;
    
    unsigned int dirShadowMapFBO;
    const int NUM_CASCADES   = 3;
    const int SHADOW_MAP_RES = 2048;
    float cascadeSplits[NUM_CASCADES] = { 10.0f, 25.0f, 55.0f };
    glm::mat4 lightSpaceMatrices[NUM_CASCADES];
    unsigned int DirCascades;

    unsigned int screenQuadVAO, screenQuadVBO;
    std::unique_ptr<Shader> S_texture, S_texturea;

    void Initialize()
    {
        Engine::RegisterResizeCallback(Resize);
        Engine::RegisterEditorReloadShadersFunction(ReloadShaders);

        glGenFramebuffers(1, &gbufferFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, gbufferFBO);

        // Combined
        glGenTextures(1, &GBuffers[GShaded]);
        glBindTexture(GL_TEXTURE_2D, GBuffers[GShaded]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, GBuffers[GShaded], 0);

        // Albedo
        glGenTextures(1, &GBuffers[GAlbedo]);
        glBindTexture(GL_TEXTURE_2D, GBuffers[GAlbedo]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, GBuffers[GAlbedo], 0);

        // Normal
        glGenTextures(1, &GBuffers[GNormal]);
        glBindTexture(GL_TEXTURE_2D, GBuffers[GNormal]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, GBuffers[GNormal], 0);

        // Mask
        glGenTextures(1, &GBuffers[GMask]);
        glBindTexture(GL_TEXTURE_2D, GBuffers[GMask]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, GBuffers[GMask], 0);

        // Depth
        glGenTextures(1, &GBuffers[GDepth]);
        glBindTexture(GL_TEXTURE_2D, GBuffers[GDepth]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, GBuffers[GDepth], 0);

        // Dir Light Shadow Factor
        glGenTextures(1, &GBuffers[GDirShadowFactor]);
        glBindTexture(GL_TEXTURE_2D, GBuffers[GDirShadowFactor]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, GBuffers[GDirShadowFactor], 0);

        Resize(Engine::GetWindowSize().x, Engine::GetWindowSize().y);

        CheckFBOStatus("Deferred");

        glGenFramebuffers(1, &dirShadowMapFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, dirShadowMapFBO);

        // Directional Shadow Map Depth

        glGenFramebuffers(1, &dirShadowMapFBO);
    
        glGenTextures(1, &DirCascades);
        glBindTexture(GL_TEXTURE_2D_ARRAY, DirCascades);
        glTexImage3D( GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT32F,
            SHADOW_MAP_RES,
            SHADOW_MAP_RES,
            NUM_CASCADES,
            0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
            
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
            
        constexpr float bordercolor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, bordercolor);
            
        glBindFramebuffer(GL_FRAMEBUFFER, dirShadowMapFBO);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, DirCascades, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);

        CheckFBOStatus("Shadow");

        S_GBuffers   = std::make_unique<Shader>("/res/shaders/deferred/draw_gbuffers");
        S_shading    = std::make_unique<Shader>("/res/shaders/deferred/shading");
        S_shadow     = std::make_unique<Shader>("/res/shaders/deferred/shadow");
        S_shadowCalc = std::make_unique<Shader>("/res/shaders/deferred/shadowcalc");
        S_mask       = std::make_unique<Shader>("/res/shaders/deferred/mask");
        S_texture    = std::make_unique<Shader>("/res/shaders/deferred/texture");
        S_texturea   = std::make_unique<Shader>("/res/shaders/deferred/texturea");
        S_fullscreenQuad  = std::make_unique<Shader>("/res/shaders/deferred/texture_fullscreen");
        S_postprocessQuad = std::make_unique<Shader>("/res/shaders/deferred/postprocess");

        glGenVertexArrays(1, &defferedQuadVAO);
        glGenVertexArrays(1, &screenQuadVAO);
        glGenBuffers(1, &screenQuadVBO);

        glBindVertexArray(screenQuadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, screenQuadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 2 * 4, NULL, GL_DYNAMIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, false, sizeof(float) * 2, 0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    // Compute camera frustum corners in world space for a given split
    static std::array<glm::vec4, 8> frustumCorners;

    inline void GetFrustumCornersWS(const glm::mat4& invProjView) {
        // NDC corners
        static constexpr glm::vec4 ndc[8] = {
            { -1, -1, -1, 1 },{ +1, -1, -1, 1 },
            { -1, +1, -1, 1 },{ +1, +1, -1, 1 },
            { -1, -1, +1, 1 },{ +1, -1, +1, 1 },
            { -1, +1, +1, 1 },{ +1, +1, +1, 1 }
        };

        for (int i = 0; i < 8; ++i) {
            auto pt = invProjView * ndc[i];
            frustumCorners[i] = pt / pt.w;
        }
    }
    
    void DrawShadows()
    {
        // glCullFace(GL_FRONT);
        glBindFramebuffer(GL_FRAMEBUFFER, dirShadowMapFBO);
        glEnable(GL_POLYGON_OFFSET_FILL);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        glPolygonOffset(2.0f, 4.0f);

        glViewport(0, 0, 2048, 2048);

        glm::vec3 lightTarget(0.0f);
        glm::vec3 lightDir(1.0f, -1.0f, 1.0f);
        // glm::vec3 lightPos(10.0f, -10.0f, 10.0f);

        S_shadow->Use();
        for (int i = 0; i < NUM_CASCADES; i++)
        {
            glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, DirCascades, 0, i);
            glClear(GL_DEPTH_BUFFER_BIT);

            float near = (i == 0) ? 0.1f : cascadeSplits[i - 1];
            float far  = cascadeSplits[i];

            // Get corners of this cascade’s frustum
            const glm::mat4& camProj = glm::perspective(glm::radians(AM::EditorCam.Fov), Engine::GetWindowAspect(), near, far);
            const glm::mat4& camView = AM::ViewMat4;
            
            glm::mat4 invPV = glm::inverse(camProj * camView);

            GetFrustumCornersWS(invPV);

            glm::vec3 center = glm::vec3(0, 0, 0);
            for (const auto& v : frustumCorners)
            {
                center += glm::vec3(v);
            }
            center /= 8;
                
            const auto lightView = glm::lookAt(
                center + lightDir,
                center,
                glm::vec3(0.0f, 0.0f, 1.0f)
            );
            
            float minX = std::numeric_limits<float>::max();
            float maxX = std::numeric_limits<float>::lowest();
            float minY = std::numeric_limits<float>::max();
            float maxY = std::numeric_limits<float>::lowest();
            float minZ = std::numeric_limits<float>::max();
            float maxZ = std::numeric_limits<float>::lowest();
            for (const auto& v : frustumCorners)
            {
                const auto trf = lightView * v;
                minX = std::min(minX, trf.x);
                maxX = std::max(maxX, trf.x);
                minY = std::min(minY, trf.y);
                maxY = std::max(maxY, trf.y);
                minZ = std::min(minZ, trf.z);
                maxZ = std::max(maxZ, trf.z);
            }

            constexpr float zMult = 10.0f;
            if   (minZ < 0) minZ *= zMult;
            else minZ /= zMult;

            if   (maxZ < 0) maxZ /= zMult;
            else maxZ *= zMult;
            
            const glm::mat4 lightProj = glm::ortho(minX, maxX, minY, maxY, minZ, maxZ);
                
            // 5. Combine
            lightSpaceMatrices[i] = lightProj * lightView;

            S_shadow->SetMatrix4("lightSpaceMatrix", lightSpaceMatrices[i]);

            for (auto const& mesh : AM::Meshes)
            {
                if (mesh.second.TriangleCount == 0 || mesh.second.VAO == 0) continue;

                int numElements = mesh.second.TriangleCount * 3;
                glBindVertexArray(mesh.second.VAO);

                int index = 0;
                for (auto &node : SM::SceneNodes)
                {
                    if (node->GetType() == SM::NodeType::Object_)
                    {
                        SM::Object* object = dynamic_cast<SM::Object*>(node);
                        if (object->GetMeshID() == mesh.first)
                        {
                            S_shadow->SetMatrix4("model", object->GetModelMatrix());
                            if (mesh.second.UseElements) glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_INT, 0);
                            else glDrawArrays(GL_TRIANGLES, 0, mesh.second.TriangleCount * 3);
                        }
                        index++;
                    }
                }
            }
        }

        glViewport(0, 0, Engine::GetWindowSize().x, Engine::GetWindowSize().y);

        glDisable(GL_POLYGON_OFFSET_FILL);

        // glCullFace(GL_BACK);
    }

    void CalcShadows()
    {
        glDepthMask(false);
        S_shadowCalc->Use();
        S_shadowCalc->SetVector3("cDir", AM::EditorCam.Front);
        S_shadowCalc->SetVector3("cPos", AM::EditorCam.Position);
        S_shadowCalc->SetMatrix4("iProjMatrix", glm::inverse(AM::ProjMat4));
        S_shadowCalc->SetMatrix4("viewMatrix",  AM::ViewMat4);
        S_shadowCalc->SetMatrix4("lightSpaceMatrix", lightSpaceMatrices[0]);

        S_shadowCalc->SetFloat("cascadeSplits[0]", cascadeSplits[0]);
        S_shadowCalc->SetFloat("cascadeSplits[1]", cascadeSplits[1]);
        S_shadowCalc->SetFloat("cascadeSplits[2]", cascadeSplits[2]);
        S_shadowCalc->SetMatrix4(std::format("lightSpaceMatrices[{}]", 0), lightSpaceMatrices[0]);
        S_shadowCalc->SetMatrix4(std::format("lightSpaceMatrices[{}]", 1), lightSpaceMatrices[1]);
        S_shadowCalc->SetMatrix4(std::format("lightSpaceMatrices[{}]", 2), lightSpaceMatrices[2]);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, GBuffers[GNormal]);
        S_shadowCalc->SetInt("GNormal", GNormal);

        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, GBuffers[GDepth]);
        S_shadowCalc->SetInt("GDepth",  GDepth);

        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_2D_ARRAY, DirCascades);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_MODE, GL_NONE);
        S_shadowCalc->SetInt("DirShadowMapRaw", ShadowCascades::First);

        // glActiveTexture(GL_TEXTURE7);
        // glBindTexture(GL_TEXTURE_2D_ARRAY, DirCascades);
        // glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
        // glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
        // glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        // glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // S_shadowCalc->SetInt("DirShadowMap", ShadowCascades::First + 1);
        
        glBindVertexArray(defferedQuadVAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        glDepthMask(true);
    }

    void DrawMask()
    {
        if (SM::SceneNodes.empty()) return;
        
        SM::Object* object = dynamic_cast<SM::Object*>(SM::SceneNodes[SM::GetSelectedIndex()]);
        
        if (object)
        {
            glDisable(GL_DEPTH_TEST);

            Deferred::S_mask->Use();
            Deferred::S_mask->SetMatrix4("projection", AM::ProjMat4);
            Deferred::S_mask->SetMatrix4("view", AM::ViewMat4);

            auto meshIter = AM::Meshes.find(object->GetMeshID());
            if (meshIter != AM::Meshes.end())
            {
                auto &mesh = meshIter->second;
                int numElements = mesh.TriangleCount * 3;
                glBindVertexArray(mesh.VAO);

                Deferred::S_mask->SetMatrix4("model", object->GetModelMatrix());
                if (mesh.UseElements) glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_INT, 0);
                else glDrawArrays(GL_TRIANGLES, 0, mesh.TriangleCount * 3);

                glEnable(GL_DEPTH_TEST);
            }
        }
    }

    const GLenum buffersGBuffers[]{ GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4 };
    void DrawGBuffers()
    {
        glDrawBuffers(5, buffersGBuffers);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        S_GBuffers->Use();
        S_GBuffers->SetMatrix4("projection", AM::ProjMat4);
        S_GBuffers->SetMatrix4("view", AM::ViewMat4);

        for (auto const& [meshID, batch] : SM::DrawList)
        {
            const auto& mesh = AM::Meshes.at(meshID);
            glBindVertexArray(mesh.VAO);

            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, batch.SSBOIdx); // Binding point 0

            // Send instance count
            int instanceCount = static_cast<int>(batch.Objects.size());
            if (mesh.UseElements)
                glDrawElementsInstanced(GL_TRIANGLES, mesh.TriangleCount * 3, GL_UNSIGNED_INT, 0, instanceCount);
            else
                glDrawArraysInstanced(GL_TRIANGLES, 0, mesh.TriangleCount * 3, instanceCount);
        }

        // for (auto const& pair : SM::DrawList)
        // {
        //     auto& mesh = AM::Meshes.at(pair.first);
        //     glBindVertexArray(mesh.VAO);
            
        //     for (auto* object : pair.second.Objects)
        //     {
        //         S_GBuffers->SetMatrix4("model", object->GetModelMatrix());
        //         if (mesh.UseElements)
        //             glDrawElements(GL_TRIANGLES, mesh.TriangleCount * 3, GL_UNSIGNED_INT, 0);
        //         else
        //             glDrawArrays(GL_TRIANGLES, 0, mesh.TriangleCount * 3);
        //     }
        // }

        // for (auto const& mesh : AM::Meshes)
        // {
        //     int numElements = mesh.second.TriangleCount * 3;
        //     glBindVertexArray(mesh.second.VAO);

        //     for (auto const& obj : SM::DrawList[mesh.first])
        //     {
        //         S_GBuffers->SetMatrix4("model", obj->GetModelMatrix());
        //         if (mesh.second.UseElements) glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_INT, 0);
        //         else glDrawArrays(GL_TRIANGLES, 0, mesh.second.TriangleCount * 3);
        //     }

        //     // int index = 0;
        //     // for (auto &node : SM::SceneNodes)
        //     // {
        //     //     if (node->GetType() == SM::NodeType::Object_)
        //     //     {
        //     //         SM::Object* object = dynamic_cast<SM::Object*>(node);
        //     //         if (object->GetMeshID() == mesh.first)
        //     //         {
        //     //             S_GBuffers->SetMatrix4("model", object->GetModelMatrix());
        //     //             if (mesh.second.UseElements) glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_INT, 0);
        //     //             else glDrawArrays(GL_TRIANGLES, 0, mesh.second.TriangleCount * 3);
        //     //         }
        //     //         index++;
        //     //     }
        //     // }
        // }
    }

    void Resize(int width, int height)
    {
        glBindTexture(GL_TEXTURE_2D, GBuffers[GShaded]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0,  GL_RGB, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glBindTexture(GL_TEXTURE_2D, GBuffers[GAlbedo]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0,  GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glBindTexture(GL_TEXTURE_2D, GBuffers[GNormal]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0,  GL_RGB, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glBindTexture(GL_TEXTURE_2D, GBuffers[GMask]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width, height, 0,  GL_RED, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

        glBindTexture(GL_TEXTURE_2D, GBuffers[GDepth]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glBindTexture(GL_TEXTURE_2D, GBuffers[GDirShadowFactor]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0,  GL_RED, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }

    unsigned int &GetGBufferFBO()
    {
        return gbufferFBO;
    }
    
    unsigned int &GetShadowFBO()
    {
        return dirShadowMapFBO;
    }
    
    void DrawFullscreenQuad(unsigned int texture)
    {
        S_fullscreenQuad->Use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);

        glBindVertexArray(defferedQuadVAO);

        glDisable(GL_DEPTH_TEST);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        glEnable(GL_DEPTH_TEST);
    }

    void DoPostProcessAndDisplay()
    {
        S_postprocessQuad->Use();
        S_postprocessQuad->SetInt("framebuffer", GShaded);
        S_postprocessQuad->SetInt("mask", GMask);
        S_postprocessQuad->SetVector2("size", Engine::GetWindowSize());

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, GBuffers[GShaded]);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, GBuffers[GMask]);

        glBindVertexArray(defferedQuadVAO);

        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }

    void DrawTexturedQuad(glm::vec2 bottomLeft, glm::vec2 topRight, unsigned int texture, bool singleChannel, bool sampleStencil, bool linearize)
    {
        S_texture->Use();
        S_texture->SetInt("isDepth", singleChannel);
        S_texture->SetInt("sampleStencil", sampleStencil);
        S_texture->SetInt("linearize", linearize);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);

        float vertices[8] =
        {
            topRight.x,   topRight.y,
            bottomLeft.x, topRight.y,
            bottomLeft.x, bottomLeft.y,
            topRight.x,   bottomLeft.y
        };

        glBindBuffer(GL_ARRAY_BUFFER, screenQuadVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

        glBindVertexArray(screenQuadVAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }

    void DrawTexturedAQuad(glm::vec2 bottomLeft, glm::vec2 topRight, unsigned int textureArray, int layer, bool singleChannel, bool sampleStencil, bool linearize)
    {
        S_texturea->Use();
        S_texturea->SetInt("isDepth", singleChannel);
        S_texturea->SetInt("sampleStencil", sampleStencil);
        S_texturea->SetInt("linearize", linearize);
        S_texturea->SetInt("layer", layer);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D_ARRAY, textureArray);

        float vertices[8] =
        {
            topRight.x,   topRight.y,
            bottomLeft.x, topRight.y,
            bottomLeft.x, bottomLeft.y,
            topRight.x,   bottomLeft.y
        };

        glBindBuffer(GL_ARRAY_BUFFER, screenQuadVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

        glBindVertexArray(screenQuadVAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }

    void DoShading()
    {
        glDepthMask(false);
        S_shading->Use();
        S_shading->SetVector3("cDir", AM::EditorCam.Front);
        S_shading->SetVector3("cPos", AM::EditorCam.Position);
        S_shading->SetMatrix4("iProjMatrix", glm::inverse(AM::ProjMat4));
        S_shading->SetMatrix4("viewMatrix",  AM::ViewMat4);
        // S_shading->SetMatrix4(std::format("lightSpaceMatrices[{}]", 0), lightSpaceMatrices[0]);

        S_shading->SetInt("NumPointLights", SM::NumLights);
        int li = 0;
        for (size_t i = 0; i < SM::SceneNodes.size(); i++)
        {
            SM::Light* light = dynamic_cast<SM::Light*>(SM::SceneNodes[i]);
            if (light)
            {
                S_shading->SetVector3(std::format("PointLights[{}].", li) + "position",  light->GetPosition());
                S_shading->SetVector3(std::format("PointLights[{}].", li) + "color",     light->GetColor());
                S_shading->SetFloat(std::format("PointLights[{}].",   li) + "intensity", light->GetIntensity());
                li++;
            }
        }
        
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, GBuffers[GAlbedo]);
        S_shading->SetInt("GAlbedo", GAlbedo);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, GBuffers[GNormal]);
        S_shading->SetInt("GNormal", GNormal);

        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, GBuffers[GDepth]);
        S_shading->SetInt("GDepth",  GDepth);

        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, GBuffers[GDirShadowFactor]);
        S_shading->SetInt("GDirShadowFactor",  GDirShadowFactor);
        
        glBindVertexArray(defferedQuadVAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        glDepthMask(true);
    }

    float length = 0.275;
    float textScale = 0.5f;
    void VisualizeGBuffers()
    {
        struct QuadInfo {
            glm::vec2 BottomLeft, TopRight;
            unsigned int texture;
            std::string title;
            bool singleChannel = false;
        };
 
        std::vector<QuadInfo> quads =
        {
            { {1.0f - length * 2, 1.0f - length},     {1.0f - length, 1.0f},          GBuffers[GAlbedo], "GAlbedo" },
            { {1.0f - length, 1.0f - length},         {1.0f, 1.0f},                   GBuffers[GNormal], "GNormal" },
            { {1.0f - length * 2, 1.0f - length * 2}, {1.0f - length, 1.0f - length}, GBuffers[GDepth],  "Gdepth", true },
            { {1.0f - length, 1.0f - length * 2},     {1.0f, 1.0f - length},          GBuffers[GMask],   "GMask",  true}
        };

        for (auto& quad : quads) {
            DrawTexturedQuad(quad.BottomLeft, quad.TopRight, quad.texture, quad.singleChannel);

            glm::ivec2 pos = qk::NDCToPixel(quad.TopRight.x - length * 0.5f, quad.TopRight.y);
            Text::RenderCenteredBG(quad.title, pos.x, pos.y - Text::CalculateMaxTextAscent("G", textScale), textScale, glm::vec3(0.9f), glm::vec3(0.05f));
        }
    }

    void VisualizeShadowMap()
    {
        glm::vec2 topright(1.0f, 1.0f);

        float length = 0.25f;
        float aspect = (float)Engine::GetWindowSize().x / Engine::GetWindowSize().y;
        float height = length * aspect;

        // Cascade 0
        glm::vec2 bl0 = glm::vec2(1.0f - length, 1.0f - height);
        DrawTexturedAQuad(bl0, topright, DirCascades, 0, true);

        // Cascade 1
        glm::vec2 bl1 = bl0 - glm::vec2(0.0f, height + 0.02f);
        glm::vec2 tr1 = bl1 + glm::vec2(length, height);
        DrawTexturedAQuad(bl1, tr1, DirCascades, 1, true);

        // Cascade 2
        glm::vec2 bl2 = bl1 - glm::vec2(0.0f, height + 0.02f);
        glm::vec2 tr2 = bl2 + glm::vec2(length, height);
        DrawTexturedAQuad(bl2, tr2, DirCascades, 2, true);

        // SHadow Factor
        glm::vec2 bottomLeft(1.0f - length * 2.5f, 1.0f - length * 1.5f);
        DrawTexturedQuad(bottomLeft, topright - glm::vec2(length, 0.0f), GBuffers[GDirShadowFactor], true);
    }

    void ReloadShaders()
    {
        S_GBuffers->Reload();
        S_shading->Reload();
        S_shadow->Reload();
        S_shadowCalc->Reload();
        S_mask->Reload();
        S_texture->Reload();
        S_texturea->Reload();
        S_fullscreenQuad->Reload();
        S_postprocessQuad->Reload();
    }
    
    void CheckFBOStatus(std::string FBOName)
    {
        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status == GL_FRAMEBUFFER_COMPLETE) {
            std::cout << FBOName << " Framebuffer successfully initialized\n";
        } else {
            std::cerr << FBOName <<" Framebuffer failed to initialize: ";
            switch (status) {
                case GL_FRAMEBUFFER_UNDEFINED: std::cerr << "GL_FRAMEBUFFER_UNDEFINED\n"; break;
                case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: std::cerr << "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT\n"; break;
                case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: std::cerr << "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT\n"; break;
                case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER: std::cerr << "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER\n"; break;
                case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER: std::cerr << "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER\n"; break;
                case GL_FRAMEBUFFER_UNSUPPORTED: std::cerr << "GL_FRAMEBUFFER_UNSUPPORTED\n"; break;
                case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE: std::cerr << "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE\n"; break;
                case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS: std::cerr << "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS\n"; break;
                default: std::cerr << "Unknown error (0x" << std::hex << status << ")\n"; break;
            }
        }
    }
}