#version 330 core

in vec2 uvs;
out vec4 FragColor;

uniform sampler2D framebuffer;

void main()
{
    FragColor = vec4(texture(framebuffer, uvs).rgb, 1.0);
}