#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNorm;

layout (std430, binding = 0) buffer BVHVisBuffer {
    mat4 instanceMatrices[];
};

// uniform mat4 model;
uniform mat4 parent;
uniform mat4 view;
uniform mat4 projection;

flat out int instanceID;

void main()
{
    instanceID = gl_InstanceID;
    mat4 model = instanceMatrices[gl_InstanceID];
    gl_Position = projection * view * parent * model * vec4(aPos, 1.0);
}