#version 330 core

in vec2 uvs;
out vec4 FragColor;

uniform sampler2D framebuffer;
uniform sampler2D mask;

uniform int numSteps = 8;

const float TAU = 6.28318530;

void main()
{
    float stencil = texture(mask, uvs).r;
    vec3  shaded  = texture(framebuffer, uvs).rgb;

    float outlinemask = 0.0;
    for (float i = 0.0; i < TAU; i += TAU / numSteps)
    {
        vec2 offset = vec2(sin(i), cos(i)) * 0.0015;
        outlinemask = mix(outlinemask, 1.0, texture(mask, uvs + offset).r);
    }

    vec3 outline_applied = mix(shaded, vec3(255, 159, 44) / 255.0, outlinemask - stencil);

    FragColor = vec4(outline_applied, 1.0);
}