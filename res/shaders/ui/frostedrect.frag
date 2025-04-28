#version 330 core

out vec4 fragColor;
in vec2 uvs;

uniform sampler2D sceneTexture;
uniform vec3 tint = vec3(1.0);

vec3 sampleTexture(vec2 offset)
{
    return texture(sceneTexture, uvs + offset).rgb;
}

void main()
{
    vec3 col = vec3(0.0);
    float radius = 0.005; // Adjust radius for blur strength

    // Define the size of the blur grid (larger grid = stronger blur)
    int sampleCount = 10; // Increase for more samples, for a smoother blur
    float offset = radius / float(sampleCount / 5); // Adjust spacing between samples

    // Sample uniformly from a square grid pattern
    for(int x = -sampleCount / 2; x <= sampleCount / 2; ++x)
    {
        for(int y = -sampleCount / 2; y <= sampleCount / 2; ++y)
        {
            col += sampleTexture(vec2(x, y) * offset); // Sample each pixel
        }
    }

    col /= float(sampleCount * sampleCount); // Normalize by the total number of samples

    fragColor = vec4(col * tint, 1.0);
}