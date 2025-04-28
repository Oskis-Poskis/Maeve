#version 330 core

out vec4 fragColor;
in vec2 uvs;

uniform vec3 color;
uniform float blend;

void main()
{
    fragColor = vec4(color, blend);
}