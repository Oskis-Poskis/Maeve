#pragma once

#include "../../common/shader.h"

namespace Deferred
{   
    enum GBuffer
    {
        GShaded          = 0,
        GAlbedo          = 1,
        GNormal          = 2,
        GMask            = 3,
        GDepth           = 4,
        GDirShadowFactor = 5,
    };

    enum ShadowCascades
    {
        First  = 6,
        Second = 7,
        Third  = 8,
    };

    inline std::unique_ptr<Shader> S_GBuffers;
    inline std::unique_ptr<Shader> S_shading;
    inline std::unique_ptr<Shader> S_shadow;
    inline std::unique_ptr<Shader> S_shadowCalc;
    inline std::unique_ptr<Shader> S_mask;
    inline std::unique_ptr<Shader> S_postprocessQuad;

    inline unsigned int GBuffers[6];

    void Initialize();
    void DrawMask();
    void DrawGBuffers();
    void DrawShadows();
    void CalcShadows();
    void DoShading();

    unsigned int &GetGBufferFBO();
    unsigned int &GetShadowFBO();
    
    void DrawFullscreenQuad(unsigned int texture);
    void DoPostProcessAndDisplay();
    void DrawTexturedQuad(glm::vec2 bottomLeft, glm::vec2 topRight, unsigned int texture, bool singleChannel = false, bool sampleStencil = false, bool linearize = false);
    void DrawTexturedAQuad(glm::vec2 bottomLeft, glm::vec2 topRight, unsigned int textureArray, int layer, bool singleChannel = false, bool sampleStencil = false, bool linearize = false);
    void VisualizeGBuffers();
    void VisualizeShadowMap();
}