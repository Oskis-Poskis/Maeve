#version 330 core

in vec2 uvs;
out vec4 color;

uniform sampler2DArray image;
uniform bool isDepth       = false;
uniform bool sampleStencil = false;
uniform bool linearize     = false;
uniform int layer;

float LinearizeDepth(float depth, float near, float far) {
    return (2.0 * near) / (far + near - depth * (far - near));
}

void main()
{
    vec4 tex = texture(image, vec3(uvs, layer));
    if (!isDepth) color = vec4(tex.rgb, 1.0);
    else
    {
        if (sampleStencil) color = vec4(vec3(tex.a), 1.0);
        else
        {
            if (linearize) color = vec4(vec3(LinearizeDepth(tex.r, 0.1, 1000.0)), 1.0);
            else color = vec4(vec3(tex.r), 1.0);
        }
    }
}