#version 330 core
layout (location = 0) out vec4 gAlbedo;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec3 gPosition;

in vec3 normal;
in vec3 fragPos;

void main()
{
    gAlbedo = vec4(vec3(1), 1.0);
    gNormal = normalize(normal);
    gPosition = fragPos;
}