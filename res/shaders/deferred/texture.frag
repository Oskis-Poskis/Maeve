#version 330 core

in vec2 uvs;
out vec4 color;

uniform sampler2D image;
uniform bool isDepth = false;

void main()
{
    if (!isDepth) color = vec4(texture(image, uvs).rgb, 1.0);
    else color = vec4(vec3(texture(image, uvs).r), 1.0);
}