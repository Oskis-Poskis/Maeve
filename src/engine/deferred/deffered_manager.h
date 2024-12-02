#pragma once

namespace Deferred
{   
    enum GBuffer
    {
        GCombined,
        GAlbedo,
        GNormal,
        GDepthStencil,
    };

    void CreateFBO();
    unsigned int &GetFBO();
    unsigned int GetTexture(GBuffer which);

    void DrawFullscreenQuad(unsigned int texture);
    void DrawTexturedQuad(glm::vec2 bottomLeft, glm::vec2 topRight, unsigned int texture);
    void CalculatePBR();
    void VisualizeGBuffers();
}