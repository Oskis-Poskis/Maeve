#version 440
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNorm;

layout(std430, binding = 0) buffer Matrices {
    mat4 modelMatrices[];
};

uniform mat4 lightSpaceMatrix;
// uniform mat4 model;

void main()
{
    mat4 model = modelMatrices[gl_InstanceID];
    gl_Position = lightSpaceMatrix * model * vec4(aPos, 1.0);
}