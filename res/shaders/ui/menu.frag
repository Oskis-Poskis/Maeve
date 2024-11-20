#version 330 core

out vec4 fragColor;

uniform vec3 color;
uniform float blend;

void main()
{
    fragColor = vec4(color, blend);
}