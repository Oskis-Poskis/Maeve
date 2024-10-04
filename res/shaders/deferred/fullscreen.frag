#version 330 core

in vec2 uvs;
out vec4 color;

uniform sampler2D framebuffer;

void main()
{
    color = vec4(texture(framebuffer, uvs).rgb, 1.0);
}