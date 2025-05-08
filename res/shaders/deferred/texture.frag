#version 330 core

in vec2 uvs;
out vec4 color;

uniform sampler2D image;
uniform bool isDepth       = false;
uniform bool sampleStencil = false;
uniform bool linearize     = false;

float LinearizeDepth(float depth, float near, float far) {
    return (2.0 * near) / (far + near - depth * (far - near));
}

void main()
{
    if (!isDepth) color = vec4(texture(image, uvs).rgb, 1.0);
    else
    {
        if (sampleStencil) color = vec4(vec3(texture(image, uvs).a), 1.0);
        else
        {
            if (linearize) color = vec4(vec3(LinearizeDepth(texture(image, uvs).r, 0.1, 1000.0)), 1.0);
            else color = vec4(vec3(texture(image, uvs).r), 1.0);
        }
    }
}