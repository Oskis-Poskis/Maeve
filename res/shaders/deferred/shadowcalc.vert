#version 330 core

out vec2 uvs;

const vec2 positions[4] = vec2[]
(
    vec2( 1.0,  1.0),
    vec2(-1.0,  1.0),
    vec2(-1.0, -1.0),
    vec2( 1.0, -1.0)
);

void main() 
{
    uvs = positions[gl_VertexID] / 2.0 + 0.5;
    gl_Position = vec4(positions[gl_VertexID], 0.0, 1.0);
}