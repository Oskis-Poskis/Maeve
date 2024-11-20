#include <iostream>
#include <memory>

#include <glm/glm.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "deffered_manager.h"
#include "../render_engine.h"
#include "../asset_manager.h"
#include "../../common/shader.h"
#include "../../common/qk.h"
#include "../../ui/text_renderer.h"

namespace Deferred
{
    void Resize(int width, int height);
    void DrawTexturedQuad(glm::vec2 bottomLeft, glm::vec2 topRight, unsigned int texture);

    unsigned int deferredFBO;
    unsigned int defferedQuadVAO;
    std::unique_ptr<Shader> fullscreenShader;

    unsigned int screenQuadVAO, screenQuadVBO;
    std::unique_ptr<Shader> screenQuadShader;

    unsigned int GBuffers[4];
    std::unique_ptr<Shader> pbr;

    void CreateFBO(int width, int height)
    {
        Engine::RegisterResizeCallback(Resize);

        glGenFramebuffers(1, &deferredFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, deferredFBO);

        // Albedo
        glGenTextures(1, &GBuffers[GAlbedo]);
        glBindTexture(GL_TEXTURE_2D, GBuffers[GAlbedo]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, GBuffers[GAlbedo], 0);

        // Normal
        glGenTextures(1, &GBuffers[GNormal]);
        glBindTexture(GL_TEXTURE_2D, GBuffers[GNormal]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, GBuffers[GNormal], 0);

        // Depth Stencil
        glGenTextures(1, &GBuffers[GDepthStencil]);
        glBindTexture(GL_TEXTURE_2D, GBuffers[GDepthStencil]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, GBuffers[GDepthStencil], 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE) std::cout << "Deffered Frambuffer successfully initialized\n\n";
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        pbr              = std::make_unique<Shader>("/../res/shaders/deferred/shading");
        fullscreenShader = std::make_unique<Shader>("/../res/shaders/deferred/fullscreen");
        screenQuadShader = std::make_unique<Shader>("/../res/shaders/deferred/screenquad");

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
        glBindTexture(GL_TEXTURE_2D, GBuffers[GAlbedo]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0,  GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glBindTexture(GL_TEXTURE_2D, GBuffers[GNormal]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0,  GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glBindTexture(GL_TEXTURE_2D, GBuffers[GDepthStencil]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
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
        fullscreenShader->Use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);

        glBindVertexArray(defferedQuadVAO);

        glDisable(GL_DEPTH_TEST);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        glEnable(GL_DEPTH_TEST);
    }

    void DrawTexturedQuad(glm::vec2 bottomLeft, glm::vec2 topRight, unsigned int texture)
    {
        screenQuadShader->Use();
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

    void CalculatePBR(glm::vec3 cPos)
    {
        pbr->Use();
        pbr->SetVector3("cPos", cPos);
        pbr->SetMatrix4("iProjMatrix", glm::inverse(AssetManager::ProjMat4));
        pbr->SetMatrix4("viewMatrix", AssetManager::ViewMat4);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, GBuffers[GAlbedo]);
        pbr->SetInt("GAlbedo", GAlbedo);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, GBuffers[GNormal]);
        pbr->SetInt("GNormal", GNormal);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, GBuffers[GDepthStencil]);
        pbr->SetInt("GDepth", GDepthStencil);
        
        glBindVertexArray(defferedQuadVAO);

        glDisable(GL_DEPTH_TEST);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        glEnable(GL_DEPTH_TEST);
    }

    float length = 0.25;
    void VisualizeGBuffers()
    {
        glDisable(GL_DEPTH_TEST);

        DrawTexturedQuad(glm::vec2(1.0 - length * 2, 1.0f - length), glm::vec2(1.0f - length, 1.0f), Deferred::GetTexture(Deferred::GAlbedo));
        glm::ivec2 pos = qk::NDCToPixel(1.0f - length * 1.5f, 1.0f);
        Text::RenderCentered("GAlbedo", pos.x, pos.y - Text::CalculateMaxTextHeight("A", 0.4f), 0.4f, glm::vec3(0.9f), glm::vec3(0.05f));

        DrawTexturedQuad(glm::vec2(1.0 - length, 1.0f - length), glm::vec2(1.0f, 1.0f), Deferred::GetTexture(Deferred::GNormal));
        pos = qk::NDCToPixel(1.0f - length * 0.5f, 1.0f);
        Text::RenderCentered("GNormal", pos.x, pos.y - Text::CalculateMaxTextHeight("A", 0.4f), 0.4f, glm::vec3(0.9f), glm::vec3(0.05f));

        DrawTexturedQuad(glm::vec2(1.0 - length, 1.0f - length * 2), glm::vec2(1.0f, 1.0f - length), Deferred::GetTexture(Deferred::GDepthStencil));
        pos = qk::NDCToPixel(1.0f - length * 0.5f, 1.0f - length);
        Text::RenderCentered("GDepthStencil", pos.x, pos.y - Text::CalculateMaxTextHeight("A", 0.4f), 0.4f, glm::vec3(0.9f), glm::vec3(0.05f));

        glEnable(GL_DEPTH_TEST);
    }
}