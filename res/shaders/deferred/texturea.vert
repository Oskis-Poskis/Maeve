#version 330 core
layout (location = 0) in vec2 vertex;

out vec2 uvs;

const vec2 texcoords[4] = vec2[]
(
    vec2( 1.0,  1.0),
    vec2( 0.0,  1.0),
    vec2( 0.0,  0.0),
    vec2( 1.0,  0.0)
);

void main()
{
    uvs = texcoords[gl_VertexID];
    gl_Position = vec4(vertex.xy, 0.0, 1.0);
}