#pragma once

namespace Deferred
{   
    enum GBuffer
    {
        GAlbedo,
        GNormal,
        GDepthStencil,
    };

    void CreateFBO(int width, int height);
    unsigned int &GetFBO();
    unsigned int GetTexture(GBuffer which);

    void DrawFullscreenQuad(unsigned int texture);
    void DrawTexturedQuad(glm::vec2 bottomLeft, glm::vec2 topRight, unsigned int texture);
    void CalculatePBR(glm::vec3 camPos);
    void VisualizeGBuffers();
}