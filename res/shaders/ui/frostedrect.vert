#version 330 core

layout (location = 0) in vec2 pos;
uniform mat4 projection;
uniform vec2 textureSize;

out vec2 uvs;

void main()
{
    uvs = pos / textureSize;
    gl_Position = projection * vec4(pos.x, pos.y, 0.0, 1.0);
}