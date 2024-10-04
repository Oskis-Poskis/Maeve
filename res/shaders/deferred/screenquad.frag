#version 330 core

in vec2 uvs;
out vec4 color;

uniform sampler2D image;
uniform bool isDepth = false;

const float near = 0.1;
const float far  = 1000;

float Linearize(float depth) 
{
    float z = depth * 2.0 - 1.0;
    return (2.0 * near * far) / (far + near - z * (far - near));	
}

void main()
{
    if (!isDepth) color = vec4(texture(image, uvs).rgb, 1.0);
    else color = vec4(vec3(Linearize(texture(image, uvs).r)), 1.0);
}