#pragma once

#include "../../common/shader.h"

namespace Deferred
{   
    enum GBuffer
    {
        GShaded,
        GAlbedo,
        GNormal,
        GDepthStencil,
    };

    inline std::unique_ptr<Shader> s_shading;

    void CreateFBO();
    unsigned int &GetFBO();
    unsigned int GetTexture(GBuffer which);

    void DrawFullscreenQuad(unsigned int texture);
    void DrawTexturedQuad(glm::vec2 bottomLeft, glm::vec2 topRight, unsigned int texture, bool singleChannel = false);
    void DoShading();
    void VisualizeGBuffers();
}