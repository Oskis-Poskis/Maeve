#pragma once

namespace Deferred
{   
    enum GBuffer
    {
        GCombined,
        GAlbedo, GRoughness,
        GNormal,
        GPosition,
        GDepthStencil,
    };

    void CreateFBO(int width, int height);
    unsigned int &GetFBO();
    unsigned int GetTexture(GBuffer which);

    void DrawFullscreenQuad(unsigned int texture);
    void DrawTexturedQuad(glm::vec2 bottomLeft, glm::vec2 topRight, unsigned int texture);
    void VisualizeGBuffers();
}