#version 330 core
layout (location = 1) out vec4 gAlbedo;
layout (location = 2) out vec3 gNormal;

in vec3 normal;
in vec3 fragPos;

void main()
{
    gAlbedo = vec4(vec3(1.0), 1.0);
    gNormal = normalize(normal);
}