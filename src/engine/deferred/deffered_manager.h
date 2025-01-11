#pragma once

#include "../../common/shader.h"

namespace Deferred
{   
    enum GBuffer
    {
        GShaded,
        GAlbedo,
        GNormal,
        GMask,
        GDepth,
    };

    inline std::unique_ptr<Shader> S_shading;
    inline std::unique_ptr<Shader> S_mask;
    inline std::unique_ptr<Shader> S_postprocessQuad;

    void Initialize();
    unsigned int &GetFBO();
    unsigned int GetTexture(GBuffer which);

    void DrawFullscreenQuad(unsigned int texture);
    void DrawPostProcessQuad();
    void DrawTexturedQuad(glm::vec2 bottomLeft, glm::vec2 topRight, unsigned int texture, bool singleChannel = false, bool sampleStencil = false);
    void DrawMask();
    void DoShading();
    void VisualizeGBuffers();
}