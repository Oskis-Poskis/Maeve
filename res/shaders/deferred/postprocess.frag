#version 330 core

in vec2 uvs;
out vec4 FragColor;

uniform sampler2D framebuffer;
uniform sampler2D mask;

uniform int numSteps = 32;
uniform vec2 size;

const float TAU = 6.28318530;

void main()
{
    float stencil = texture(mask, uvs).r;
    vec3  shaded  = texture(framebuffer, uvs).rgb;

    float aspectRatio = size.x / size.y;
    float outlineThickness = 2.5 / size.x;

    float outlinemask = 0.0;
    for (float i = 0.0; i < TAU; i += TAU / numSteps) {
        vec2 offset = vec2(sin(i), cos(i)) * outlineThickness;
        offset.x *= 1.0;
        offset.y *= aspectRatio;

        outlinemask = mix(outlinemask, 1.0, texture(mask, uvs + offset).r);
    }

    float pixelSize = fwidth(outlinemask);
    outlinemask = smoothstep(0.5 - pixelSize * 0.5, 0.5 + pixelSize * 0.5, outlinemask);

    vec3 outline_applied = mix(shaded, vec3(255, 159, 44) / 255.0, outlinemask - stencil);

    FragColor = vec4(outline_applied, 1.0);
}