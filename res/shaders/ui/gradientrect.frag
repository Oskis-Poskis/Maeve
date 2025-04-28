#version 330 core

out vec4 fragColor;
in vec2 uvs;

uniform vec3 color1;
uniform vec3 color2;

void main()
{
    float mask = uvs.y;
    fragColor = vec4(mix(color1, color2, mask), 1.0);
}