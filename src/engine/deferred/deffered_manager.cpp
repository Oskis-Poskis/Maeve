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

    unsigned int deferredFBO;
    unsigned int defferedQuadVAO;
    std::unique_ptr<Shader> S_fullscreenQuad;

    unsigned int screenQuadVAO, screenQuadVBO;
    std::unique_ptr<Shader> S_quad;

    unsigned int GBuffers[5];

    void Initialize()
    {
        Engine::RegisterResizeCallback(Resize);
        Engine::RegisterEditorReloadShadersFunction(ReloadShaders);

        glGenFramebuffers(1, &deferredFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, deferredFBO);

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

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE) std::cout << "Deffered Frambuffer successfully initialized\n\n";
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        S_shading = std::make_unique<Shader>("/res/shaders/deferred/shading");
        S_mask = std::make_unique<Shader>("/res/shaders/deferred/mask");
        S_quad = std::make_unique<Shader>("/res/shaders/deferred/texture");
        S_fullscreenQuad = std::make_unique<Shader>("/res/shaders/deferred/texture_fullscreen");
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
    }

    unsigned int &GetFBO()
    {
        return deferredFBO;
    }

    unsigned int GetTexture(GBuffer which)
    {
        return GBuffers[which];
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

    void DrawTexturedQuad(glm::vec2 bottomLeft, glm::vec2 topRight, unsigned int texture, bool singleChannel, bool sampleStencil)
    {
        S_quad->Use();
        if (sampleStencil)
        {
            S_quad->SetInt("isDepth", singleChannel);
            S_quad->SetInt("sampleStencil", true);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture);
        }
        else
        {
            S_quad->SetInt("isDepth", singleChannel);
            S_quad->SetInt("sampleStencil", false);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture);
        }

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

            auto &mesh = AM::Meshes.at(object->GetMeshID());
            int numElements = mesh.TriangleCount * 3;
            glBindVertexArray(mesh.VAO);

            Deferred::S_mask->SetMatrix4("model", object->GetModelMatrix());
            if (mesh.UseElements) glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_INT, 0);
            else glDrawArrays(GL_TRIANGLES, 0, mesh.TriangleCount * 3);

            glEnable(GL_DEPTH_TEST);
        }
    }

    void DoShading()
    {
        glDepthMask(false);
        S_shading->Use();
        S_shading->SetVector3("cDir", AM::EditorCam.Front);
        S_shading->SetVector3("cPos", AM::EditorCam.Position);
        S_shading->SetMatrix4("iProjMatrix", glm::inverse(AM::ProjMat4));
        S_shading->SetMatrix4("viewMatrix", AM::ViewMat4);

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
        S_shading->SetInt("GDepth", GDepth);
        
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
            Deferred::GBuffer texture;
            std::string title;
            bool singleChannel = false;
        };

        std::vector<QuadInfo> quads =
        {
            { {1.0f - length * 2, 1.0f - length},     {1.0f - length, 1.0f},          GAlbedo, "GAlbedo" },
            { {1.0f - length, 1.0f - length},         {1.0f, 1.0f},                   Deferred::GNormal, "GNormal" },
            { {1.0f - length * 2, 1.0f - length * 2}, {1.0f - length, 1.0f - length}, Deferred::GDepth,  "Gdepth", true },
            { {1.0f - length, 1.0f - length * 2},     {1.0f, 1.0f - length},          Deferred::GMask,   "GMask",  true}
        };

        for (auto& quad : quads) {
            DrawTexturedQuad(quad.BottomLeft, quad.TopRight, Deferred::GetTexture(quad.texture), quad.singleChannel);

            glm::ivec2 pos = qk::NDCToPixel(quad.TopRight.x - length * 0.5f, quad.TopRight.y);
            Text::RenderCenteredBG(quad.title, pos.x, pos.y - Text::CalculateMaxTextAscent("G", textScale), textScale, glm::vec3(0.9f), glm::vec3(0.05f));
        }
    }

    void ReloadShaders()
    {
        S_shading->Reload();
        S_mask->Reload();
        S_quad->Reload();
        S_fullscreenQuad->Reload();
        S_postprocessQuad->Reload();
    }
}