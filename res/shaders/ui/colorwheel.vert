#version 330 core

layout (location = 0) in vec2 pos;

uniform mat4 projection;

out vec2 uvs;

void main()
{
    uvs = vec2(
        (gl_VertexID == 0 || gl_VertexID == 3) ? 1.0 : 0.0,
        (gl_VertexID == 0 || gl_VertexID == 1) ? 1.0 : 0.0
    );
    
    gl_Position = projection * vec4(pos.x, pos.y, 0.0, 1.0);
};